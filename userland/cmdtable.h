#ifndef CMDTABLE_H
#define CMDTABLE_H

typedef struct {
    const char *name;
    int (*fn)(int argc, char **argv);
} cmd_entry_t;

#define REGISTER_CMD(cname, cfn) \
    __attribute__((section("cmdtable"), used)) \
    static cmd_entry_t _cmd_##cfn = { cname, cfn };

extern cmd_entry_t __start_cmdtable[];
extern cmd_entry_t __stop_cmdtable[];

#endif