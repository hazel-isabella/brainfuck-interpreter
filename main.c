#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define USAGE "USAGE: brainfuck program_file"

#define INC_POINTER '>'
#define DEC_POINTER '<'

#define INC_BYTE '+'
#define DEC_BYTE '-'

#define OUT_BYTE '.'
#define IN_BYTE ','
#define INSTRUCTION_START '['
#define INSTRUCTION_END ']'

#define MEMGROWTH 2

int
main(int argc, char** argv)
{
    FILE *file = NULL;
    uint8_t *memory = NULL;
    long *pos, cnt = 0;
    int memlen = 2048;
    int pid = -1, cpid = 0;
    int instructionrefcnt = 0; 
    int options = 0, wstatus;
    char c;

    errno = 0; // Redundant but whatever

    if (argc != 2) {
        fprintf(stderr, "%s\n", USAGE);
        errno = EINVAL;
        goto exit;
    }
    
    file = fopen(argv[1], "r");
    if (file == NULL || errno)
        goto exit;
    
    memory = (uint8_t*)mmap(NULL, memlen, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pos = (long*)mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (errno)
        goto exit;

    *pos = 0;
    for (int i = 0; i < memlen; i++) {
        (memory)[i] = 0;
    }
    while ((c = fgetc(file)) != EOF)
    {
        if (c == EOF)
            goto exit;

        switch (c) {
            case INSTRUCTION_START:
                instructionrefcnt++;
                if (memory[*pos] == 0 || instructionrefcnt != 1) // Skipping to end of loop
                    goto next;

                if ((cpid = fork()) == 0) {
                    instructionrefcnt = 0;
                    pid = cpid;
                    cpid = 0;
                } else {
                    waitpid(cpid, &wstatus, options);
                    fseek(file, cnt, SEEK_SET);
                    instructionrefcnt--;
                    continue;
                }

                break;
            case INSTRUCTION_END:
                if (pid == 0 && instructionrefcnt == 0) // We have found the end of loop
                    goto exit;
                instructionrefcnt--;
                goto next;
            default:
                break;
        }

        if (instructionrefcnt > 0)
            goto next;

        switch (c) {
            case INC_POINTER:
                (*pos)++;
                if (*pos >= memlen) {
                    errno = ENOMEM;
                    goto exit;
                }
                break;
            case DEC_POINTER:
                if (*pos > 0)
                    (*pos)--;
                break;
            case INC_BYTE:
                (memory)[*pos]++;
                break;
            case DEC_BYTE:
                (memory)[*pos]--;
                break;
            case OUT_BYTE:
                printf("%c", memory[*pos]);
                fflush(stdout);
                break;
            case IN_BYTE:
                read(0, &(memory[*pos]), 1);
                break;
        }
        goto next;
next:
        cnt++;
        continue;
    }

exit:
    if (pid == 0)
        return errno;
    if (memory != NULL) {
        munmap(memory, sizeof(uint8_t) * memlen);
    }
    if (pos != NULL)
        munmap(pos, sizeof(int));
    if (file != NULL)
        fclose(file);

    return errno;
}

