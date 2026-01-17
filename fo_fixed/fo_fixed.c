/* File Organization: Fixed-Length Records */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define EC_IO       1

#define RECORD_SZ   sizeof(struct instructor)

#define BLOCK_SZ    256

char block[256];

void BlockWrite(int dbf, int bno, char *bd);
void BlockRead(int dbf, int bno, char *bd);

struct __attribute__((packed)) RecordPointer
{
    uint32_t  bno;
    uint16_t  off;
};

struct __attribute__((packed)) FileHeader {
    struct RecordPointer last;
    struct RecordPointer free;
};

struct __attribute__((packed)) FreeNode {
    struct RecordPointer next;
};

#define DB_FILE_NAME    "fo_fixed.db"
int file_fd;
struct FileHeader file_header;

void RecordPointerSet(struct RecordPointer *p, uint32_t bno, uint16_t off) {
    p->bno = bno;
    p->off = off;
}

void FileCreate()
{
    int dbf;
    struct FileHeader *h;

    if ((dbf = open(DB_FILE_NAME, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0)
    {
        perror("Open fo_fixed.db failed");
        exit(EC_IO);
    }

    h = (struct FileHeader *) block;
    RecordPointerSet(&(h->last), 0, sizeof(struct FileHeader));
    RecordPointerSet(&(h->free), 0, 0);
    
    BlockWrite(dbf, 0, block);

    if ( -1 == close(dbf))
    {
        perror("Close fo_fixed.db failed");
        exit(1);
    }
}

void FileOpen()
{
    struct FileHeader *p;

    if ((file_fd = open(DB_FILE_NAME, O_RDWR)) < 0)
    {
        perror("Open fo_fixed.db failed");
        exit(EC_IO);
    }

    BlockRead(file_fd, 0, block);

    p = (struct FileHeader *) block;
    RecordPointerSet(&file_header.last, p->last.bno, p->last.off);
    RecordPointerSet(&file_header.free, p->free.bno, p->free.off);
}

void FileClose()
{
    struct FileHeader *p;

    BlockRead(file_fd, 0, block);

    p = (struct FileHeader *) block;
    RecordPointerSet(&p->last, file_header.last.bno, file_header.last.off);
    RecordPointerSet(&p->free, file_header.free.bno, file_header.free.off);

    BlockWrite(file_fd, 0, block);

    if ( -1 == close(file_fd))
    {
        perror("Close fo_fixed.db failed");
        exit(1);
    }
}

void BlockSeek(int dbf, int bno)
{
    if (-1 == lseek(dbf, bno * BLOCK_SZ, SEEK_SET))
    {
        perror("block write error");
        exit(EC_IO);
    }
}

void BlockWrite(int dbf, int bno, char *bd)
{
    BlockSeek(dbf, bno);

    if (BLOCK_SZ != write(dbf, bd, BLOCK_SZ))
    {
        perror("block write error");
        exit(EC_IO);
    }
}

void BlockRead(int fd, int bno, char *bd)
{
    int r;
    char *p;

    BlockSeek(fd, bno);

    p = bd;
    while ((r = read(fd, p, bd + BLOCK_SZ - p)) > 0)
        p += r;

    if (p - bd != BLOCK_SZ)
    {
        perror("block read error");
        exit(EC_IO);
    }
}


struct __attribute__((packed)) instructor
{
    char id[5];
    char name[20];
    char dept_name[20];
    double salary;
};

void RecordAppend(struct instructor *ins);

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

        for (i = 0; i < n && i < len; i++)
        {
            s[i] = input_buff[i];
        }
        for ( ; i < n; i++)
        {
            s[i] = ' ';
        }
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

void RecordDelete(struct RecordPointer *p)
{
    struct FreeNode *n;

    BlockRead(file_fd, p->bno, block);

    n = (struct FreeNode *) (block + p->off);
    RecordPointerSet(&n->next, 
            file_header.free.bno,
            file_header.free.off);
    RecordPointerSet(&(file_header.free),
            p->bno, p->off);

    BlockWrite(file_fd, p->bno, block);
}

void RecordInsert(struct instructor *ins)
{
    struct FreeNode *fp;
    int bno;

    if (file_header.free.bno == 0 && file_header.free.off == 0)
    {
        RecordAppend(ins);
    }
    else
    {
        // free node
        bno = file_header.free.bno;

        BlockRead(file_fd, bno, block);
       
        fp = (struct FreeNode *) (block + file_header.free.off);

        RecordPointerSet(&file_header.free, fp->next.bno, fp->next.off); 

        memcpy(fp, ins, RECORD_SZ);

        BlockWrite(file_fd, bno, block);
    }
}

void RecordAppend(struct instructor *ins)
{
    int off, bno;

    off = file_header.last.off;
    bno = file_header.last.bno;

    if (off + RECORD_SZ > BLOCK_SZ)
    {
        // store in a new block
        off = 0;
        bno++;
        bzero(block, BLOCK_SZ);
    }
    else
    {
        BlockRead(file_fd, bno, block);
    }

    memcpy(block + off, ins, RECORD_SZ);

    BlockWrite(file_fd, bno, block);

    file_header.last.bno = bno;
    file_header.last.off = off + RECORD_SZ;
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
    RecordAppend(&ins);
}

void output_record(struct instructor *ins)
{
    printf("Id: ");
    output_str(ins->id, sizeof(ins->id));
    printf("\nName: ");
    output_str(ins->name, sizeof(ins->name));
    printf("\nDepart Name: ");
    output_str(ins->dept_name, sizeof(ins->dept_name));
    printf("\nSalary %lf\n", ins->salary);
}

void insert_record()
{
    struct instructor ins;

    RecordInput(&ins);

    puts("Inserting record ...");

    RecordInsert(&ins);
}

void list_records(int dbf)
{
    struct instructor *ins;
    int bno, off;

    bno = 0;    
    off = sizeof(file_header);

    while (bno <= file_header.last.bno)
    {
        BlockRead(file_fd, bno, block);
        while (bno < file_header.last.bno && off + RECORD_SZ <= BLOCK_SZ
                || off + RECORD_SZ <= file_header.last.off)
        {
            ins = (struct instructor *) (block + off);
            puts("===");
            printf("Record (%u, %u)\n", bno, off);
            puts("---");
            output_record(ins);
            off += RECORD_SZ;
        }

        bno++;
        off = 0;
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
    struct RecordPointer rp;

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
        FileCreate();
        exit(0);
    }

    FileOpen();

    switch(op)
    {
        case OP_APPEND:
            append_record(dbf);
            break;
        case OP_LIST:
            list_records(dbf);
            break;
        case OP_INSERT:
            insert_record();
            break;
        case OP_DELETE:
            rp.bno = atoi(argv[0]);
            rp.off = atoi(argv[1]);
            printf("Deleteing record at %u %u\n", rp.bno, rp.off);
            RecordDelete(&rp);
            break;
        default:
            puts("Invalid operation\n");
            exit(2);
    }

    FileClose();

    exit(0);
}


