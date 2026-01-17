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
    //printf("found a slot %x\n", p);
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
            puts("===");
            if (bh->bh_rec[i].size == -1)
            {
                printf("Record at (%d, %d) deleted\n", b, i);
                continue;
            }
            p = (struct Record *) (block + bh->bh_rec[i].br_off);
            printf("Record at (%d, %d)\n", b, i);
            puts("---");
            RecordOutput(p);
        }
    }
}

void delete_record(int bno, int rno)
{
    struct BlockHeader *h;
    struct BlockRecord *r;
    int off, sz;
    int i;

    BlockRead(file_fd, bno, block);

    h = (struct BlockHeader*) block;
    off = h->bh_rec[rno].br_off;
    sz = h->bh_rec[rno].size;

    memmove(block + h->bh_free_end + sz, 
            block + h->bh_free_end,
            off - h->bh_free_end);

    for (i = 0; i < h->bh_nrec; i++)
    {
       r = &h->bh_rec[i]; 
       if (r->size == -1)
           continue;
       if (r->br_off < off)
           r->br_off += sz;
    }
    h->bh_free_end += sz;
    h->bh_rec[rno].size = -1;
    
    BlockWrite(file_fd, bno, block);
}

void update_record(int bno, int rno)
{
    struct BlockHeader *h;
    struct BlockRecord *r;
    int off, sz, newoff, newsz;
    int i;

    struct instructor ins;
    struct Record *p;

    RecordInput(&ins);

    //output_record(&ins);

    newsz = RecordSize(&ins);

    printf("record size = %d\n", sz);

    BlockRead(file_fd, bno, block);

    h = (struct BlockHeader*) block;
    off = h->bh_rec[rno].br_off;
    sz = h->bh_rec[rno].size;
    newoff = off + sz - newsz;

    memmove(block + h->bh_free_end + sz - newsz, 
            block + h->bh_free_end,
            off - h->bh_free_end);
    p = (struct Record *) (block + newoff);
    RecordFill(p, &ins);

    for (i = 0; i < h->bh_nrec; i++)
    {
       r = &h->bh_rec[i]; 
       if (r->size == -1)
           continue;
       if (r->br_off < off)
           r->br_off += (sz - newsz);
    }
    h->bh_free_end += (sz - newsz);
    h->bh_rec[rno].br_off = newoff;
    h->bh_rec[rno].size = newsz;
    
    BlockWrite(file_fd, bno, block);
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

    while ((ch = getopt(argc, argv, "fidul")) != -1)
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
            case '?':
            default:
                usage();
        }
    }
    argc -= optind;
    argv += optind;

    printf("sizeof(double) = %lu\n", sizeof(double));
    printf("sizeof(record) = %lu\n", sizeof(struct instructor));
    printf("BLOCK SIZE = %d\n", BLOCK_SZ);

    if (op == OP_CREATEFILE)
    {
        FileCreate(DB_FILE_NAME);
        exit(0);
    }

    FileOpen(DB_FILE_NAME);

    switch(op)
    {
        case OP_LIST:
            list_records();
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

    FileClose();

    exit(0);
}


