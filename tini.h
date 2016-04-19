/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */
#define TINI_LINE_SIZE 512

#define TINI_FLAG_IGNORE   0
#define TINI_FLAG_SECTION  1
#define TINI_FLAG_KEYVALUE 2

struct tini_kv {
	char *key;
	char *value;
};

struct tini_sect {
	char *name;
	int n_kv;	/* the number of kv */
	struct tini_kv **kv;
};

struct tini_base {
	int n_sect;
	struct tini_sect **sect;
	char *tini_file;	/* config file name */
};

struct tini_sect *tini_get_sect(struct tini_base *, const char *);
char *tini_get_v(struct tini_base *, const char *, const char *);
void tini_free(struct tini_base *);
void tini_print_kv(struct tini_kv *);
void tini_print_sect(struct tini_sect *);
void tini_print(struct tini_base *);
int tini_parse(const char *, struct tini_base **);
int tini_parse_cb(const char *, int (*)(int, int, const char *, const char *, const char *));
