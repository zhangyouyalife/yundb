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

struct instructor
{
    char id[5 + 1];
    char name[20 + 1];
    char dept_name[20 + 1];
    double salary;
};

void output_record(struct instructor *ins);

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

void output_record(struct instructor *ins)
{
    printf("Id: ");
    output_str(ins->id, strlen(ins->id));
    printf("\nName: ");
    output_str(ins->name, strlen(ins->name));
    printf("\nDepart Name: ");
    output_str(ins->dept_name, strlen(ins->dept_name));
    printf("\nSalary %lf\n", ins->salary);
}


void RecordFill(struct Record *r, struct instructor *ins)
{
    r->id.vf_off = sizeof(struct Record);
    r->id.vf_len = strlen(ins->id);
    memcpy(((char *) r) + r->id.vf_off, ins->id, r->id.vf_len);

    r->name.vf_off = r->id.vf_off + r->id.vf_len;
    r->name.vf_len = strlen(ins->name);
    memcpy(((char *) r) + r->name.vf_off, ins->name, r->name.vf_len);


    r->dept_name.vf_off = r->name.vf_off + r->name.vf_len;
    r->dept_name.vf_len = strlen(ins->dept_name);
    memcpy(((char *) r) + r->dept_name.vf_off, ins->dept_name, r->dept_name.vf_len);

    r->salary = ins->salary;
}

void RecordOutput(struct Record *r)
{
    printf("Id: ");
    output_str(((char *) r) + r->id.vf_off, r->id.vf_len);
    printf("\nName: ");
    output_str(((char *) r) + r->name.vf_off, r->name.vf_len);
    printf("\nDepart Name: ");
    output_str(((char *) r) + r->dept_name.vf_off, r->dept_name.vf_len);
    printf("\nSalary %lf\n", r->salary);
}

int16_t RecordSize(struct instructor *ins)
{
    int sz;

    sz = sizeof(struct Record);
    sz += strlen(ins->id);
    sz += strlen(ins->name);
    sz += strlen(ins->dept_name);

    return sz;
}

void insert_record()
{
    int b, sz;
    struct instructor ins;
    struct Record *p;

    RecordInput(&ins);

    //output_record(&ins);

    puts("Inserting record ...");

    sz = RecordSize(&ins);

    printf("record size = %d\n", sz);
    for (b = 1; b < relation.hdr->blks; b++)
    {
        f_rb(&relation, b, block);
        p = (struct Record *) f_nr(block, sz);
        if (p != NULL)
        {
            RecordFill(p, &ins);
            f_wb(&relation, b, block);
            return;
        }
    }

    // no available block
    printf("new block\n");
    f_binit(block);
    printf("new record slot\n");
    p = (struct Record *) f_nr(block, sz);
    //printf("found a slot %x\n", p);
    RecordFill(p, &ins);
    f_wb(&relation, b, block);
    relation.hdr->blks = b+1;
}

void list_records(char *rname)
{   
    int b, sz, i;

    struct instructor ins;
    struct dd_rel *p;
    struct dbf_blkhdr *bh;
    char s[256];

    printf("To find relation [%s]\n", rname);

    for (b = 1; b < relation.hdr->blks; b++)
    {
        /* find relation */
        f_rb(&relation, b, block);
        bh = (struct dbf_blkhdr*) block ;

        for (i = 0; i < bh->nrec; i++)
        {
            printf(".");
            if (bh->rec[i].sz == -1)
            {
                continue;
            }
            p = (struct dd_rel *) (block + bh->rec[i].off);
            memset(s, 0, 256);
            /*printf("p name off %d\n", p->name.off);*/
            strncpy(s, (char *)p + p->name.off, p->name.len);
            /*printf("found a relation: %s\n", s);*/
            if (strcmp(s, rname) == 0)
            {
                printf("\nRelation found\n");
                return;
            }
        }
    }

    puts("Relation not found");
}

void delete_record(int bno, int rno)
{
    struct dbf_blkhdr *h;
    struct dbf_rec *r;
    int off, sz;
    int i;

    f_rb(&relation, bno, block);

    h = (struct dbf_blkhdr*) block;
    off = h->rec[rno].off;
    sz = h->rec[rno].sz;

    memmove(block + h->free + sz, 
            block + h->free,
            off - h->free);

    for (i = 0; i < h->nrec; i++)
    {
       r = &h->rec[i]; 
       if (r->sz == -1)
           continue;
       if (r->off < off)
           r->off += sz;
    }
    h->free += sz;
    h->rec[rno].sz = -1;
    
    f_wb(&relation, bno, block);
}

void update_record(int bno, int rno)
{
    struct dbf_blkhdr *h;
    struct dbf_rec *r;
    int off, sz, newoff, newsz;
    int i;

    struct instructor ins;
    struct Record *p;

    RecordInput(&ins);

    //output_record(&ins);

    newsz = RecordSize(&ins);

    printf("record size = %d\n", sz);

    f_rb(&relation, bno, block);

    h = (struct dbf_blkhdr*) block;
    off = h->rec[rno].off;
    sz = h->rec[rno].sz;
    newoff = off + sz - newsz;

    memmove(block + h->free + sz - newsz, 
            block + h->free,
            off - h->free);
    p = (struct Record *) (block + newoff);
    RecordFill(p, &ins);

    for (i = 0; i < h->nrec; i++)
    {
       r = &h->rec[i]; 
       if (r->sz == -1)
           continue;
       if (r->off < off)
           r->off += (sz - newsz);
    }
    h->free += (sz - newsz);
    h->rec[rno].off = newoff;
    h->rec[rno].sz = newsz;
    
    f_wb(&relation, bno, block);
}
#define OP_UPDATE       1
#define OP_LIST         2
#define OP_CREATEFILE   3
#define OP_INSERT       4
#define OP_DELETE       5

int main(int argc, char** argv)
{
    int ch;
    int op;
    int bno, rno;
    char relname[256];

    while ((ch = getopt(argc, argv, "fidulr:")) != -1)
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
            case '?':
            default:
                usage();
        }
    }
    argc -= optind;
    argv += optind;

    printf("sizeof(double) = %lu\n", sizeof(double));
    printf("sizeof(record) = %lu\n", sizeof(struct instructor));
    printf("BLOCK SIZE = %d\n", BLK_SZ);

    if (op == OP_CREATEFILE)
    {
        //f_crt(&relation, DB_FILE_NAME);
        db_init(DB_PATH);
        exit(0);
    }

    f_open(&relation, DB_REL_PATH);

    switch(op)
    {
        case OP_LIST:
            list_records(relname);
            break;
        case OP_INSERT:
            insert_record();
            break;
        case OP_DELETE:
            bno = atoi(argv[0]);
            rno = atoi(argv[1]);
            printf("Deleteing record at %d %d\n", bno, rno);
            delete_record(bno, rno);
            break;
        case OP_UPDATE:
            bno = atoi(argv[0]);
            rno = atoi(argv[1]);
            printf("Updating record at %d %d\n", bno, rno);
            update_record(bno, rno);
            break;
        default:
            puts("Invalid operation\n");
            exit(2);
    }

    f_close(&relation);

    exit(0);
}


