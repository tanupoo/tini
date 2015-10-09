
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
};

void tini_free(struct tini_base *);
struct tini_sect *tini_get_sect(struct tini_base *, const char *);
void tini_print_kv(struct tini_kv *);
void tini_print_sect(struct tini_sect *);
void tini_print(struct tini_base *);
int tini_parse(char *, struct tini_base **);
int tini_parse_cb(char *, int (*)(int, int, char *, char *, char *));
