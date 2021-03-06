/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "tini.h"

#define SEP	" ="

static char *sep = SEP;

#ifdef DEBUG
static int f_tini_debug = 0;
#endif

struct tini_sect *
tini_get_sect(struct tini_base *base, const char *section)
{
	int i;

	if (base->n_sect == 0)
		return NULL;

	for (i = 0; i < base->n_sect; i++) {
		if (strcmp(base->sect[i]->name, section) == 0)
			return base->sect[i];
	}

	return NULL;
}

char *
tini_get_v(struct tini_base *base, const char *s_sect, const char *s_key)
{
	struct tini_sect *sect;
	int i;

	if ((sect = tini_get_sect(base, s_sect)) == NULL)
		return NULL;

	for (i = 0; i < sect->n_kv; i++) {
		if (strcmp(sect->kv[i]->key, s_key) == 0)
			return sect->kv[i]->value;
	}

	return NULL;
}

/*
 * for reuse the database (e.g. reloading), it just clear it.
 * it doesn't free the memory pointed by "base".
 */
void
tini_free(struct tini_base *base)
{
	int i, j;

	if (base == NULL)
		return;

	for (i = 0; i < base->n_sect; i++) {
		free(base->sect[i]->name);
		for (j = 0; j < base->sect[i]->n_kv; j++) {
			free(base->sect[i]->kv[j]->key);
			free(base->sect[i]->kv[j]->value);
		}
		free(base->sect[i]);
	}
	free(base->tini_file);
	free(base);
}

static struct tini_base *
tini_new()
{
	struct tini_base *new;

	new = calloc(1, sizeof(struct tini_base));
	if (new == NULL)
		err(1, "calloc(tini_base)");

	return new;
}

static struct tini_kv *
tini_new_kv(struct tini_sect *sect, const char *key, const char *value)
{
	struct tini_kv **new;
	int n;
       
	n = sect->n_kv + 1;

	new = realloc(sect->kv, n * sizeof(struct tini_kv *));
	if (new == NULL)
		err(1, "realloc(tini_kv *)");

	sect->kv = new;

	if ((new[n - 1] = malloc(sizeof(struct tini_kv))) == NULL)
		err(1, "malloc(tini_kv)");

	memset(new[n - 1], 0, sizeof(struct tini_kv));

	if ((new[n - 1]->key = strdup(key)) == NULL)
		err(1, "strdup(key)");
	if ((new[n - 1]->value = strdup(value)) == NULL)
		err(1, "strdup(value)");

	sect->n_kv = n;	/* update */

	return new[n - 1];
}

static struct tini_sect *
tini_new_sect(struct tini_base *base, const char *name)
{
	struct tini_sect **new;
	int n;
       
	n = base->n_sect + 1;

	new = realloc(base->sect, n * sizeof(struct tini_sect *));
	if (new == NULL)
		err(1, "realloc(tini_sect *)");

	base->sect = new;

	if ((new[n - 1] = malloc(sizeof(struct tini_sect))) == NULL)
		err(1, "malloc(tini_sect)");

	memset(new[n - 1], 0, sizeof(struct tini_sect));

	if ((new[n - 1]->name = strdup(name)) == NULL)
		err(1, "strdup(name)");

	base->n_sect = n;

	return new[n - 1];
}

static int
tini_parse_one(char *linebuf, int lineno, char **s, char **k, char **v)
{
	char *bp = linebuf;

	/* removing any spaces in the head */
	while (*bp == ' ' || *bp == '\t')
		bp++;
	if (*bp == '\n') {
		/* null line */
		return TINI_FLAG_IGNORE;
	}
	if (*bp == '\0')
		err(1, "there is no EOL. it should not happen.");

	/* comment ? */
	if (*bp == '#') {
		/* found a comment line */
		return TINI_FLAG_IGNORE;
	}

	/* section ? */
	if (*bp == '[') {
		*s = ++bp;
		while (*bp != '\0') {
			if (*bp == ']') {
				/* found a section */
				*bp = '\0';
				return TINI_FLAG_SECTION;
			}
			bp++;
		}
	}

	/* key and value ? */
	*k = bp;
	*v = strstr(bp, sep);
	if (*v == *k)
		err(1, "FATAL: sep is NULL. it should not happen.");
	if (*v == NULL)
		err(1, "ERROR: there is no separator in line %d", lineno);

	/* found key and value */
	**v = '\0';
	*v = *v + strlen(sep);

	/* removing any spaces in the head of the key */
	bp = *v;
	while (*bp == ' ' || *bp == '\t')
		bp++;
	if (*bp == '\n') {
		/* null value */
		**v = '\0';
		return TINI_FLAG_KEYVALUE;
	}
	if (*bp == '\0')
		err(1, "FATAL: there is no EOL. it should not happen.");
	*v = bp;

	/* removing any spaces in the tail */
	bp = *v + strlen(*v);
	while (*bp == '\0' || *bp == ' ' || *bp == '\t' || *bp == '\n') {
		bp--;
	}
	if (bp + 2 == *v) {
		/* found a value of null */
		/*
		 * because if there are only spaces in the value,
		 * bp should point to '=' by the removing spaces operation,
		 * the separator is ' = ' and the *v should point
		 * to the address next to the space immediately following '='.
		 */
		**v = '\0';
	}
	*(bp + 1) = '\0';

	return TINI_FLAG_KEYVALUE;
}

/*
 * if base0 is not NULL, this function allocates new area for the tini_base.
 * so, before it is called, *base0 has to be cleared properly.
 */
static int
tini_parse_core(const char *tini_file,
    struct tini_base **base0,
    int (*handler)(int, int, const char *, const char *, const char *))
{
	struct tini_base *base = NULL;
	FILE *fp;
	char *linebuf;
	int max_size = TINI_LINE_SIZE;
	int len;
	int lineno = 0;
	struct tini_sect *section;
	char *section_name;
	char *s, *k, *v;
	int ret;

	if (base0 != NULL) {
		if ((base = tini_new()) == NULL)
			err(1, "tini_new()");
		if ((base->tini_file = strdup(tini_file)) == NULL)
			err(1, "strdup()");
		*base0 = base;
	}

	if ((linebuf = malloc(max_size)) == NULL)
		err(1, "malloc(TINI_LINE_SIZE)");

	if ((fp = fopen(tini_file, "r")) == NULL)
		err(1, "fopen(%s)", tini_file);

	s = k = v = NULL;
	section = NULL;
	section_name = NULL;
	while (fgets(linebuf, max_size, fp) != NULL) {
		lineno++;
		len = strlen(linebuf);
		if (linebuf[len - 1] != '\n')
			errx(1, "ERROR: line %d is too long.", lineno);
		ret = tini_parse_one(linebuf, lineno, &s, &k, &v);
#ifdef DEBUG
		if (f_tini_debug) {
			if (ret == TINI_FLAG_SECTION)
				printf("  s:[%s]\n", s);
			else if (ret == TINI_FLAG_KEYVALUE) {
				printf("    k:[%s]\n", k);
				printf("    v:[%s]\n", v);
			}
		}
#endif
		if (ret == TINI_FLAG_IGNORE)
			;
		else if (ret == TINI_FLAG_SECTION) {
			if (base0 != NULL)
				section = tini_new_sect(base, s);
			if (handler != NULL) {
				section_name = s;
				handler(lineno, ret, s, NULL, NULL);
			}
		} else if (ret == TINI_FLAG_KEYVALUE) {
			if (base0 != NULL) {
				if (section == NULL)
					err(1, "ERROR: section is not defined.");
				tini_new_kv(section, k, v);
			}
			if (handler != NULL) {
				if (section_name == NULL)
					err(1, "ERROR: section is not defined.");
				handler(lineno, ret, section_name, k, v);
			}
		} else
			err(1, "ERROR: it should not come here.");
	}
	if (!feof(fp))
		err(1, "fgets()");
	fclose(fp);

	return 0;
}

void
tini_print_kv(struct tini_kv *kv)
{
	printf("    key: [%s]\n", kv->key);
	printf("  value: [%s]\n", kv->value);
}

void
tini_print_sect(struct tini_sect *sect)
{
	int j;

	printf("section: [%s]\n", sect->name);
	for (j = 0; j < sect->n_kv; j++)
		tini_print_kv(sect->kv[j]);
}

void
tini_print(struct tini_base *base)
{
	int i;

	printf("tini_file: [%s]\n", base->tini_file);
	for (i = 0; i < base->n_sect; i++)
		tini_print_sect(base->sect[i]);
}

int
tini_parse(const char *tini_file, struct tini_base **base0)
{
	*base0 = NULL;
	return tini_parse_core(tini_file, base0, NULL);
}

int
tini_parse_cb(const char *tini_file,
    int (*handler)(int, int, const char *, const char *, const char *))
{
	return tini_parse_core(tini_file, NULL, handler);
}
