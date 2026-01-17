/* File Organization: Fixed-Length Records */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "exitcode.h"
#include "block.h"
#include "file.h"

#define DB_FILE_NAME    "fo_var.db"

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

void append_record(int dbf)
{
    struct instructor ins;
    int rsize;

    printf("Id: ");
    input_str(ins.id, 5);

    printf("Name: ");
    input_str(ins.name, 20);

    printf("Dept Name: ");
    input_str(ins.dept_name, 20);

    printf("Salary: ");
    input_double(&ins.salary);

    //rsize = sizeof(ins);
    //printf("%d", rsize);
    puts("Appending record...");
    //RecordAppend(&ins);
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
    for (b = 1; b < file_header->fh_blocks; b++)
    {
        BlockRead(file_fd, b, block);
        p = BlockNewRecord(block, sz);
        if (p != NULL)
        {
            RecordFill(p, &ins);
            BlockWrite(file_fd, b, block);
            return;
        }
    }

    // no available block
    printf("new block\n");
    BlockNew(block);
    printf("new record slot\n");
    p = BlockNewRecord(block, sz);
    printf("found a slot %x\n", p);
    RecordFill(p, &ins);
    BlockWrite(file_fd, b, block);
    file_header->fh_blocks = b+1;
}

void list_records()
{   
    int b, sz, i;

    struct instructor ins;
    struct Record *p;
    struct BlockHeader *bh;

    puts("Listing records ...");

    for (b = 1; b < file_header->fh_blocks; b++)
    {
        BlockRead(file_fd, b, block);
        bh = (struct BlockHeader*) block ;

        for (i = 0; i < bh->bh_nrec; i++)
        {
            p = block + bh->bh_rec[i].br_off;
            RecordOutput(p);
        }
    }
}

#define OP_APPEND       1
#define OP_LIST         2
#define OP_CREATEFILE   3
#define OP_INSERT       4
#define OP_DELETE       5

int main(int argc, char** argv)
{
    int dbf;
    int ch;
    int op;

    while ((ch = getopt(argc, argv, "faidl")) != -1)
    {
        switch (ch) {
            case 'f':
                op = OP_CREATEFILE;
                break;
            case 'a':
                op = OP_APPEND;
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
            case '?':
            default:
                usage();
        }
    }
    argc -= optind;
    argv += optind;

    printf("sizeof(double) = %lu\n", sizeof(double));
    printf("sizeof(record) = %lu\n", sizeof(struct instructor));
    printf("BLOCK SIZE = %lu\n", BLOCK_SZ);

    if (op == OP_CREATEFILE)
    {
        FileCreate(DB_FILE_NAME);
        exit(0);
    }

    FileOpen(DB_FILE_NAME);

    switch(op)
    {
        case OP_APPEND:
            append_record(dbf);
            break;
        case OP_LIST:
            list_records();
            break;
        case OP_INSERT:
            insert_record();
            break;
        case OP_DELETE:
//            rp.bno = atoi(argv[0]);
 //           rp.off = atoi(argv[1]);
 //           printf("Deleteing record at %u %u\n", rp.bno, rp.off);
            break;
        default:
            puts("Invalid operation\n");
            exit(2);
    }

    FileClose();

    exit(0);
}


