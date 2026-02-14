/* File Organization: Fixed-Length Records */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "block.h"
#include "file.h"
#include "tuple.h"
#include "db.h"
#include "datadict.h"
#include "data.h"
#include "buf.h"

struct instructor
{
    char id[5 + 1];
    char name[20 + 1];
    char dept_name[20 + 1];
    double salary;
};

void create_instructor_rel()
{
    struct ddl_create c;

    ddl_create_new(&c, "instructor", 4, FO_HEAP);
    ddl_create_attr(&c, "id", 0, DOMAIN_VARCHAR, 5);
    ddl_create_attr(&c, "name", 1, DOMAIN_VARCHAR, 20);
    ddl_create_attr(&c, "dept_name", 2, DOMAIN_VARCHAR, 20);
    ddl_create_attr(&c, "salary", 3, DOMAIN_FLOAT, 8);
    ddl_create_go(&c);
    ddl_create_free(&c);
}

void usage()
{
    puts("\nfo_fixed: File Organization: Fixed-Length Records\n");
    puts("Options\n");
    puts("\t-i insert record\n");
    puts("\t-l list records\n");
    exit(0);
}

static char input_buff[100];

void input_str(char *s, int n)
{
    int i;
    size_t len;

    if (fgets(input_buff, 100, stdin) != NULL)
    {
        len = strlen(input_buff);
        if (len > 0 && input_buff[len-1] == '\n') {
            input_buff[len-1] = 0;
            len -= 1;
        }
        
        strncpy(s, input_buff, n);

        s[n] = 0;
    }
}

void output_str(char *db_str, int n)
{
    int i;
    
    for (i = 0; i < n; i++)
    {
        input_buff[i] = db_str[i];
    }
    input_buff[i] = 0;
    printf("%s", input_buff);
}

void input_double(double *d)
{
    scanf("%lf", d);
}
void RecordInput(struct instructor *r)
{
    printf("Id: ");
    input_str(r->id, 5);

    printf("Name: ");
    input_str(r->name, 20);

    printf("Dept Name: ");
    input_str(r->dept_name, 20);

    printf("Salary: ");
    input_double(&r->salary);
}

void clear_input()
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF) ;
}

void insert_record()
{
    struct instructor ins;
    union dml_value v[4];

    RecordInput(&ins);

    puts("Inserting record ...");

    v[0].v_val = ins.id;
    v[1].v_val = ins.name;
    v[2].v_val = ins.dept_name;
    v[3].f_val = ins.salary;
    dml_insert("instructor", v);
    puts("OK");

    clear_input();
}

void list_records(char *rname)
{   
    int i;
    char val[256];
    struct d_datum_h *v;
    struct dql_cursor cur;
    struct dql_tuple t;

    puts("===");
    puts(rname);
    puts("===");
    dql_cursor_create(&cur, rname);
    if (0 != dql_cursor_open(&cur) )
    {
        perror("list_records dql_cursor_open");
        return;
    }

    while ( dql_cursor_fetch(&t, &cur)  == 0)
    {
        
        for (i = 0; i < t.desc->nattr; i++)
        {
            v = db_attr_val(i, t.t, t.desc);
            printf("%s: %s\n", t.desc->attrs[i].name, d_text(val, v));
            d_hfree(v);
        }

        puts("---");
    }
    dql_cursor_close(&cur);
}

void delete_record(char *id)
{
    struct dml_where w;
    
    w.attr = "id";
    w.v.v_val = id;

    dml_delete("instructor", &w);

    puts("OK");
}

void update_record(char *id)
{
    union dml_value v[4];
    struct dml_where w;
    
    struct instructor ins;

    RecordInput(&ins);

    v[0].v_val = ins.id;
    v[1].v_val = ins.name;
    v[2].v_val = ins.dept_name;
    v[3].f_val = ins.salary;

    w.attr = "id";
    w.v.v_val = id;

    dml_update("instructor", v, &w);

    puts("OK");

    clear_input();
}

#define OP_UPDATE       1
#define OP_LIST         2
#define OP_INSERT       4
#define OP_DELETE       5
#define OP_CREATEINS    6
#define OP_EXIT         7
#define OP_BUFINFO      8
#define OP_DATADICT     9

struct cmd
{
    uint8_t op;
};

struct cmd_list
{
    uint8_t op;
    char rel[256];
};

struct cmd1
{
    uint8_t op;
    char a1[256];
};

char cmdbuf[256];

void cmd_free(struct cmd *c)
{
    free(c);
}

struct cmd *cmd_read()
{
    struct cmd *c;

    printf("> ");
    if (NULL == fgets(cmdbuf, 256, stdin))
    {
        c = malloc(sizeof(struct cmd));
        c->op = OP_EXIT;
        return c;
    }

    if (strncmp(cmdbuf, "\\b", 2) == 0)
    {
        c = malloc(sizeof(struct cmd));
        c->op = OP_BUFINFO;
        return c;
    }
    if (strncmp(cmdbuf, "\\dd", 3) == 0)
    {
        c = malloc(sizeof(struct cmd));
        c->op = OP_DATADICT;
        return c;
    }

    if (strncmp(cmdbuf, "create", 6) == 0)
    {
        c = malloc(sizeof(struct cmd));
        c->op = OP_CREATEINS;
        return c;
    }

    if (strncmp(cmdbuf, "list", 4) == 0)
    {
        c = malloc(sizeof(struct cmd_list));
        c->op = OP_LIST;
        sscanf(cmdbuf, "list %s\n", ((struct cmd_list *)c)->rel);
        return c;
    }

    if (strncmp(cmdbuf, "i", 1) == 0)
    {
        c = malloc(sizeof(struct cmd));
        c->op = OP_INSERT;
        return c;
    }

    if (strncmp(cmdbuf, "d", 1) == 0)
    {
        c = malloc(sizeof(struct cmd1));
        c->op = OP_DELETE;
        sscanf(cmdbuf, "d %s\n", ((struct cmd1 *)c)->a1);
        return c;
    }

    if (strncmp(cmdbuf, "u", 1) == 0)
    {
        c = malloc(sizeof(struct cmd1));
        c->op = OP_UPDATE;
        sscanf(cmdbuf, "u %s\n", ((struct cmd1 *)c)->a1);
        return c;
    }

    return 0;
}

void print_dd(void *d)
{
    struct dd_rel_m *m;
    struct dd_attrdesc *a;
    int i;

    m = (struct dd_rel_m *)d;
    printf("(%s, nattr=%d, org=%d, fd=%d)\n",
            m->desc.name, 
            m->desc.nattr,
            m->desc.forg,
            m->f.fd);
    for (i = 0; i < m->desc.nattr; i++)
    {
        a = &m->desc.attrs[i];
        printf("\t%s\n", a->name);
    }

}

int cmd_exec(struct cmd *c)
{
    switch (c->op)
    {
        case OP_BUFINFO:
            b_info();
            break;

        case OP_CREATEINS:
            create_instructor_rel();
            break;

        case OP_LIST:
            list_records(((struct cmd_list *)c)->rel);
            break;

        case OP_DATADICT:
            ll_travel(&datadict, print_dd);
            break;

        case OP_INSERT:
            insert_record();
            break;

        case OP_DELETE:
            delete_record(((struct cmd1*)c)->a1);
            break;
        case OP_UPDATE:
            update_record(((struct cmd1*)c)->a1);
            break;

        default:
            return 1;
    }

    return 0;
}

int main(int argc, char** argv)
{
    int ch;
    int op;
    char relname[256];
    struct cmd *cmd;

    if (argc < 2)
    {
        puts("Usage:");
        puts("\topendb <db_path>");
        exit(1);
    }

    strcpy(db_path, argv[1]);

    b_init();

    dd_init();

    while (1)
    {
        cmd = cmd_read();
        if (0 == cmd)
        {
            puts("invalid command");
            continue;
        }

        if (cmd->op == OP_EXIT)
        {
            cmd_free(cmd);
            break;
        }
           
        cmd_exec(cmd);
        cmd_free(cmd);
    }

    b_sync();
    dd_free();

    exit(0);
}
