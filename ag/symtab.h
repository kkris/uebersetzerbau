struct symbol {
    char *name;
    struct symbol *next;
};

struct sym_table {
    struct symbol *sym;
    struct sym_table *parent;
};
