// cmdtable.h
typedef struct { const char *name; int (*fn)(int argc, char **argv); } cmd_entry_t;

#define REGISTER_CMD(cname, cfn) \
    static int cfn(int argc, char **argv); \
    __attribute__((section("cmdtable"), used)) \
    static cmd_entry_t _cmd_##cfn = { cname, cfn };