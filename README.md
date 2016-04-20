tini
====

Tiny INI-like file parer and utility.
You can see a sample code in test_ini.c.

## basic format

    ~~~~
    [section_1]
    key1 = value1
    key2 = value2
    ~~~~

## The file format and rules

- any spaces ([\n\t\s]+) in the head of the line are removed.
- any spaces in the tail of the value are removed.
- if the line has only spaces, it is removed.

under the above rules,

- after removing any spaces in the head of the line, if the first charactor in the line is '#', the line is ignored.
- any charactors except '[', ']' and '=' are allowed for sector, key, and value.
- at least one section must be defined. (*)
- a section must begin '[' and close with ']'.
- a key must not begin '['.
- a key must not include " =".  it is dealt with a separator.
- " ="(space-equal) must be placed between the key and value.
- the value of null is allowed.
- this parser doesn't check the duplication of key.  just added to the result.

## TODO

- allow no space next to '=' as null value.

## improvement idea

- allow a pair of key-value with no section ?

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

## sample of an INI file

you can see the following example in test.ini.

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
    
    [any spaces in the key]
    key1 = hoge
    
    [any spaces in section]
    key3 = allowed.
    
    [ space at the head and tail. ]
    how should it be = any comment ?
    
    [comment: tail of the value]
    k = value # this is not a comment.
    
    [null value]
        this defines a null value =   
        note that there are spaces in the tail, next to the equal charactor = 	
    ~~~~
