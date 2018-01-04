/*

  Run as
  numactl -C cpuid ./uncore_imc MB
  e.g.,
  numactl -C 0 ./uncore_imc 1000

  Care must be taken to ensure that the cpuid, which is the "processor" field
  in /proc/cpuinfo, has a "physical id" field matching that of the memory
  controller one is measuring. For example, on a node with 2 sockets of
  Broadwell (E5-2698 v4) and two threads per core, cpuids 0-19, 40-59
  have physical id 0.

 */

#define _GNU_SOURCE
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


void setup();
uint64_t readctr(int flag, int ctr);

#define NMC 4
#define NEDC 0
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

// borrowed from stream
double sec()
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
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
    printf("%d Iterations, %lu bytes (%3.2f GiB)\n",
	   ITER, ITER * data_bytes, ITER * data_bytes/1024.0/1024.0/1024.0);
    size_t emo = ITER * data_lines;
    printf("%15s: %8lu\n\n", "Expected memops (reads)", emo);

    void *data = 0;
    int ret = posix_memalign(&data, 1024*1024, data_bytes);
    assert(ret == 0);
    memset(data, 0, data_bytes);


    struct counters s1, s2;

    double start = sec();
    readcounters(&s1);
    for(int j=0;j<ITER;j++)
    {
        __asm
        {
           mov rax, data        /* put data ptr into ax */
           mov r10, rax         /* put data ptr into r10 */
           mov r11, rax         /* put data ptr into r11 */
           mov rbx, data_lines  /* put number of cache lines into rbx */
 
           xor rcx, rcx         /* zero register rcx, loop counter */
           LN1:
	     cmp rcx, rbx       /* compare loop ctr to num cache lines */
             jge LN2            /* if loop ctr > n cache lines, exit */
             inc rcx            /* increment loop ctr */
             mov rax, QWORD PTR [r11]  /* move qword at addr in r11 to rax */
             add r11, 64        /* increment addr in r11 by a cache line */
             jmp LN1            /* return to top of loop */
           LN2:
        }
    }
    readcounters(&s2);
    double stop = sec();

    uint64_t sum;
    printf("%25s","DDR Reads:");
    sum = 0;
    for (int i = 0; i < NMC; ++i)
    {
        uint64_t diff = s2.mc_rd[i] - s1.mc_rd[i];
        printf(" %8lu", diff);
        sum += diff;
    }
    uint64_t rds_tot = sum;
    printf("\n%25s %8lu\n", "DDR Reads Total:", rds_tot);
    printf("%25s", "DDR Writes:");
    sum = 0;
    for (int i = 0; i < NMC; ++i)
    {
        uint64_t diff = s2.mc_wr[i] - s1.mc_wr[i];
        printf(" %8lu", diff);
        sum += diff;
    }
    uint64_t wrt_tot = sum;
    printf("\n%25s %8lu\n", "DDR Writes Total:", wrt_tot);
    uint64_t rd_wr_tot = rds_tot+wrt_tot;
    printf("\n%25s %8lu\n", "Reads plus Writes Total:", rd_wr_tot);

    printf("\n%25s %8lu\n", "Expected memops (reads):", emo);
    printf("%25s %6.2f%%\n", "Difference:",
	   100.0*fabs((double)emo - (double)rds_tot)/((double) emo));

    double time = stop-start;
    printf("\n%25s: %3.3f\n", "Time elapsed",time);
    double mln = 1024.0*1024.0;
    printf("%25s: %3.3f MiB/s\n", "Bandwidth (expected)",
	   (double)(emo*64)/time/mln);
    printf("%25s: %3.3f MiB/s\n", "Bandwidth (counted)", 
	   (double)(rds_tot*64)/time/mln);

#if 0
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
#endif

}
