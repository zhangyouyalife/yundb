/* File Organization: Fixed-Length Records */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "block.h"
#include "file.h"
#include "record.h"
#include "db.h"
#include "datadict.h"

#define DB_PATH         "./database/"
#define DB_REL_PATH     "./database/relation.rel"
#define DB_ATTR_PATH     "./database/attribute.rel"

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

void insert_record()
{
    int b, sz;
    struct instructor ins;
    union db_value v[4];

    RecordInput(&ins);

    puts("Inserting record ...");

    v[0].v_val = ins.id;
    v[1].v_val = ins.name;
    v[2].v_val = ins.dept_name;
    v[3].f_val = ins.salary;
    dml_insert("instructor", v);
    puts("OK");
}

void list_records(char *rname)
{   
    int b, sz, i;

    struct instructor ins;
    struct dd_rel *p;
    struct dbf_blkhdr *bh;
    char s[256];
    struct dd_reldesc rd;
    struct dbf f;
    struct dbf_it it;
    char *r;
    char val[256];

    /* printf("To find relation [%s]\n", rname); */
    if (dd_reldesc_get(&rd, rname))
    {
        /* metadata found */
        /* printf("Found [%s]\n", rd.name); */
        /*printf("It has %d atrributes\n", rd.nattr);
        for (i = 0; i < rd.nattr; i++)
        {
            printf("%s\n", rd.attrs[i].name);
        } */
        /* show data */
        puts("===");
        puts(rname);
        puts("===");
        sprintf(s, "%s%s.rel", DB_PATH, rname);
        f_open(&f, s);
        f_it(&f, &it);
        while ( (r = f_itnext(&it)) != 0)
        {
            for (i = 0; i < rd.nattr; i++)
            {
                db_val(val, i, r, &rd);
                printf("%s: %s\n", rd.attrs[i].name, val);
            }
            puts("---");
        }
        f_itfree(&it);
        f_close(&f);


        dd_reldesc_free(&rd);
    }
    else
    {
        puts("Relation not found");
    }
}

void delete_record(char *id)
{
    struct dbf_blkhdr *h;
    struct dbf_rec *r;
    int off, sz;
    int i;
    struct db_where w;
    
    w.attr = "id";
    w.v.v_val = id;

    dml_delete("instructor", &w);

    puts("OK");
}

void update_record(char *id)
{
    struct dbf_blkhdr *h;
    struct dbf_rec *r;
    int off, sz, newoff, newsz;
    int i;
    union db_value v[4];
    struct db_where w;
    
    struct instructor ins;
    struct Record *p;

    RecordInput(&ins);

    v[0].v_val = ins.id;
    v[1].v_val = ins.name;
    v[2].v_val = ins.dept_name;
    v[3].f_val = ins.salary;

    w.attr = "id";
    w.v.v_val = id;

    dml_update("instructor", v, &w);

    puts("OK");
}

#define OP_UPDATE       1
#define OP_LIST         2
#define OP_CREATEFILE   3
#define OP_INSERT       4
#define OP_DELETE       5
#define OP_CREATEINS    6

int main(int argc, char** argv)
{
    int ch;
    int op;
    int bno, rno;
    char relname[256];

    strcpy(db_path, DB_PATH);

    while ((ch = getopt(argc, argv, "fidulr:c")) != -1)
    {
        switch (ch) {
            case 'f':
                op = OP_CREATEFILE;
                break;
            case 'u':
                op = OP_UPDATE;
                break;
            case 'i':
                op = OP_INSERT;
                break;
            case 'd':
                op = OP_DELETE;
                break;
            case 'l':
                op = OP_LIST;
                break;
            case 'r':
                memset(relname, 0, 256);
                strncpy(relname, optarg, 255);
                break;
            case 'c':
                op = OP_CREATEINS;
                break;
            case '?':
            default:
                usage();
        }
    }
    argc -= optind;
    argv += optind;
/*
    printf("sizeof(double) = %lu\n", sizeof(double));
    printf("sizeof(record) = %lu\n", sizeof(struct instructor));
    printf("BLOCK SIZE = %d\n", BLK_SZ);
*/
    if (op == OP_CREATEFILE)
    {
        //f_crt(&relation, DB_FILE_NAME);
        db_init(DB_PATH);
        exit(0);
    }

    f_open(&relation, DB_REL_PATH);
    f_open(&attribute, DB_ATTR_PATH);

    switch(op)
    {
        case OP_LIST:
            list_records(relname);
            break;
        case OP_INSERT:
            insert_record();
            break;
        case OP_DELETE:
            delete_record(argv[0]);
            break;
        case OP_UPDATE:
            update_record(argv[0]);
            break;
        case OP_CREATEINS:
            create_instructor_rel();
            break;
        default:
            puts("Invalid operation\n");
            exit(2);
    }

    f_close(&attribute);
    f_close(&relation);

    exit(0);
}
