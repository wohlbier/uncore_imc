#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>


void setup();
uint64_t readctr(int flag, int ctr);

#define NMC 6
#define NEDC 8
#define MC_FLAG_READ 0
#define MC_FLAG_WRITE 1
#define EDC_FLAG_READ 2
#define EDC_FLAG_WRITE 3

struct counters
{
    uint64_t mc_rd[NMC];
    uint64_t mc_wr[NMC];
    uint64_t edc_rd[NEDC];
    uint64_t edc_wr[NEDC];
};


static void readcounters(struct counters *s)
{
    for (int i = 0; i < NMC; ++i)
    {
        s->mc_rd[i] = readctr(MC_FLAG_READ, i);
        s->mc_wr[i] = readctr(MC_FLAG_WRITE, i);
    }
    for (int i = 0; i < NEDC; ++i)
    {
        s->edc_rd[i] = readctr(EDC_FLAG_READ, i);
        s->edc_wr[i] = readctr(EDC_FLAG_WRITE, i);
    }
}

#define ITER 10


int
main(int argc, char **argv)
{
    assert(argc == 2);
    setup();
    int data_size=atoi(&argv[1][0]);    // size in MB
    size_t data_bytes = data_size*1024*1024;
    size_t data_lines = data_bytes / 64;
    printf("%d Iterations, %lu bytes\n", ITER, ITER * data_bytes);
    printf("%15s: %8lu\n", "Expected memops", ITER * data_lines);
 
    void *data = 0;
    int ret = posix_memalign(&data, 1024*1024, data_bytes);
    assert(ret == 0);
    memset(data, 0, data_bytes);
 
    
    struct counters s1, s2;

    readcounters(&s1);
    for(int j=0;j<ITER;j++)
    {
        __asm
        {
                mov rax, data
                mov r10, rax
                mov r11, rax
                mov rbx, data_lines
 
                xor rcx, rcx
                LN1:
                        cmp rcx, rbx
                        jge LN2
                        inc rcx
                        mov rax, QWORD PTR [r11]
                        add r11, 64
                        jmp LN1
                LN2:
        }
    }
    readcounters(&s2);
    uint64_t sum;
    printf("%15s:", " DDR Reads");
    sum = 0;
    for (int i = 0; i < NMC; ++i)
    {
        uint64_t diff = s2.mc_rd[i] - s1.mc_rd[i];
        printf(" %8lu", diff);
        sum += diff;
    }
    printf("\n%15s: %8lu\n", " DDR Reads Total", sum);
    printf("%15s:", " DDR Writes");
    sum = 0;
    for (int i = 0; i < NMC; ++i)
    {
        uint64_t diff = s2.mc_wr[i] - s1.mc_wr[i];
        printf(" %8lu", diff);
        sum += diff;
    }
    printf("\n%15s: %8lu\n", " DDR Writes Total", sum);
    printf("%15s:", " MCDRAM Reads");
    sum = 0;
    for (int i = 0; i < NEDC; ++i)
    {
        uint64_t diff = s2.edc_rd[i] - s1.edc_rd[i];
        printf(" %8lu", diff);
        sum += diff;
    }
    printf("\n%15s: %8lu\n", " MCDRAM Reads Total", sum);
    printf("%15s:", " MCDRAM Writes");
    sum = 0;
    for (int i = 0; i < NEDC; ++i)
    {
        uint64_t diff = s2.edc_wr[i] - s1.edc_wr[i];
        printf(" %8lu", diff);
        sum += diff;
    }
    printf("\n%15s: %8lu\n", " MCDRAM Writes Total", sum);

}