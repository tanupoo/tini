tini
====

Tiny INI-like file parer and utility.

## The file format and rules

- at least one section must be defined.
- any spaces ([\n\t\s]+) in the head of the line are removed.
- any spaces in the tail of the value are removed.

under the above rules,

- if the first charactor in the line is '#', the line is ignored.
- the line has only spaces, the line is ignored.
- a section must begin '[' and close with ']'.
- a key must not begin '['.
- " = "(space-equal-space) must be placed between the key and value.
- the value of null is allowed. only spaces are placed after the separator.
- omitting the space following '=' is not allowed.
- this parser doesn't check the duplication of key.

## utility

See tini.h.

    ~~~~
    struct tini_sect *tini_get_sect(struct tini_base *, const char *);
    char *tini_get_v(struct tini_base *, const char *, const char *);
    void tini_free(struct tini_base *);
    void tini_print_kv(struct tini_kv *);
    void tini_print_sect(struct tini_sect *);
    void tini_print(struct tini_base *);
    int tini_parse(const char *, struct tini_base **);
    int tini_parse_cb(const char *, int (*)(int, int, const char *, const char *, const char *));
    ~~~~

## sample

    ~~~~
    #
    # this is test example for ini-like config tiny parser.
    #
    # see the comment in the parse.c for detail.
    #
    
    # output files. the path may be included.
    [output]
    	14745668 = file001.CSV
    	14811145 = file002.CSV
    
    # key and map definition.
    [keymap]
    key1 = ke5b5d40d24f20663db986c80242456342c505fc9
    key2 = k9b34feefd03215bb6e4628d9ce07edd730fb321c
    
    [null]
        this defines a null value =   
        note that there are spaces in the tail, next to the equal charactor = 	
    
    #[error]
    #    but, this happens error =
    ~~~~
