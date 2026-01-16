/* File Organization: Fixed-Length Records */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

struct __attribute__((packed)) instructor
{
    char id[5];
    char name[20];
    char dept_name[20];
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

    rsize = sizeof(ins);
    //printf("%d", rsize);
    puts("Writing record...");
    output_record(&ins);
    lseek(dbf, 0, SEEK_END);
    if (rsize != write(dbf, &ins, sizeof(ins)))
    {
        perror("Write error");
        exit(1);
    }
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


void list_records(int dbf)
{
    int r, off;
    struct instructor ins;

    lseek(dbf, 0, SEEK_SET);

    off = 0;
    while ((r = read(dbf, ((char *)&ins) + off, sizeof(ins) - off)) > 0)
    {
        off += r;
        if (off == sizeof(ins))
        {
            output_record(&ins);
            puts("---");
            off = 0;
        }
    }
}

#define OP_APPEND   1
#define OP_LIST     2

int main(int argc, char** argv)
{
    int dbf;
    int ch;
    int op;

    while ((ch = getopt(argc, argv, "al")) != -1)
    {
        switch (ch) {
            case 'a':
                op = OP_APPEND;
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

    if ((dbf = open("fo_fixed.db", O_CREAT|O_RDWR, 0644)) < 0)
    {
        perror("Open fo_fixed.db failed");
        exit(1);
    }

    switch(op)
    {
        case OP_APPEND:
            append_record(dbf);
            break;
        case OP_LIST:
            list_records(dbf);
            break;
        default:
            puts("Invalid operation\n");
            exit(2);
    }

    if ( -1 == close(dbf))
    {
        perror("Close fo_fixed.db failed");
        exit(1);
    }

    exit(0);
}


