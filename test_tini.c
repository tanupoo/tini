#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "tini.h"

int f_debug = 0;

char *prog_name = NULL;

void
usage()
{
	printf(
"Usage: %s [-dh]\n"
	, prog_name);

	exit(0);
}

int
handler(int lineno, int flag, char *section, char *key, char *value)
{
	if (flag == TINI_FLAG_SECTION)
		printf("    %4d:s:[%s]\n", lineno, section);
	else if (flag == TINI_FLAG_KEYVALUE) {
		printf("    %4d:k:[%s]\n", lineno, key);
		printf("    %4d:v:[%s]\n", lineno, value);
	}

	return 0;
}

void
config_load(char *config_file, struct tini_base **base)
{
	if (tini_parse(config_file, base) < 0)
		errx(1, "can't load %s", config_file);
}

void
config_reload(char *config_file, struct tini_base **base)
{
	tini_free(*base);
	return config_load(config_file, base);
}

int
run(char *config_file)
{
	struct tini_base *base;

	printf("=== parsing %s\n", config_file);

	printf("--- tini_parse()\n");
	config_load(config_file, &base);
	tini_print(base);

	printf("--- reloading.\n");
	config_reload(config_file, &base);
	tini_print(base);
	tini_free(base);

	printf("--- tini_parse_cb().\n");
	tini_parse_cb(config_file, handler);

	return 0;
}

int
main(int argc, char *argv[])
{
	int ch;
	char *config_file = "test.ini";;

	prog_name = 1 + rindex(argv[0], '/');

	while ((ch = getopt(argc, argv, "c:dh")) != -1) {
		switch (ch) {
		case 'c':
			config_file = optarg;
			break;
		case 'd':
			f_debug++;
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	run(config_file);

	return 0;
}

