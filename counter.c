#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <err.h>
#include "perf_utils.h"

struct evinfo
{
  uint64_t count;
  void *buf;
  int32_t fd;
};

#define NMC 4
#define NEDC 0
struct gbl_
{

  struct evinfo mc_rd[NMC];
  struct evinfo mc_wr[NMC];
  struct evinfo edc_rd[NEDC];
  struct evinfo edc_wr[NEDC];
};

static struct gbl_ gbl;

static void
evsetup(const char *ename, struct evinfo *ep, int event, int umask);

  void
setup()
{
  int ret;

  for (int mc = 0; mc < NMC; ++mc)
  {
    int cpu=(mc < NMC/2) ? mc : mc%2 + 4;
    char fname[1024];
    snprintf(fname, sizeof(fname), "/sys/devices/uncore_imc_%d", cpu);
    //evsetup(fname, &gbl.mc_rd[mc], 0x3/*event*/, 0x1/*umask*/);
    //evsetup(fname, &gbl.mc_wr[mc], 0x3, 0x2);
    evsetup(fname, &gbl.mc_rd[mc], 0x04/*event*/, 0x03/*umask*/);
    evsetup(fname, &gbl.mc_wr[mc], 0x04, 0x0c);
  }
  for (int mc = 0; mc < NEDC; ++mc)
  {
    char fname[1024];
    snprintf(fname, sizeof(fname), "/sys/devices/uncore_edc_eclk_%d", mc);
    evsetup(fname, &gbl.edc_rd[mc], 0x1/*event*/, 0x1/*umask*/);
    evsetup(fname, &gbl.edc_wr[mc], 0x2, 0x1);
  }
}

static void
evsetup(const char *ename, struct evinfo *ep, int event, int umask)
{
  char fname[1024];
  snprintf(fname, sizeof(fname), "%s/type", ename);
  FILE *fp = fopen(fname, "r");
  assert(fp != 0);
  int type;
  int ret = fscanf(fp, "%d", &type);
  assert(ret == 1);
  fclose(fp);
  printf("Using PMU type %d from %s\n", type, ename);

  struct perf_event_attr hw = {};
  hw.size = sizeof(hw);
  hw.type = type;
  hw.config = event | (umask << 8);	// see /sys/devices/uncore_*/format/*
  hw.config1 = umask;
  int cpu = 0;
  ep->fd = perf_event_open(&hw, -1, cpu, -1, 0);
  if (ep->fd == -1)
    err(1, "CPU %d, event 0x%lx", cpu, hw.config);
}

#define MC_FLAG_READ 0
#define MC_FLAG_WRITE 1
#define EDC_FLAG_READ 2
#define EDC_FLAG_WRITE 3
uint64_t
readctr(int flag, int ctr)
{
  struct evinfo *ep;
  switch(flag)
  {
    case MC_FLAG_READ:
      ep = &gbl.mc_rd[ctr];
      break;
    case MC_FLAG_WRITE:
      ep = &gbl.mc_wr[ctr];
      break;
    case EDC_FLAG_READ:
      ep = &gbl.edc_rd[ctr];
      break;
    case EDC_FLAG_WRITE:
      ep = &gbl.edc_wr[ctr];
      break;
    default:
      ep = &gbl.mc_rd[ctr];
      break;
  }

  size_t s = read(ep->fd, &ep->count, sizeof(uint64_t));
  if (s != sizeof(uint64_t))
  {
    printf("read returns %lu\n", s);
    err(1, "read counter");
  }
  return ep->count;
}

#if 0
  int
main(int argc, char **argv)
{
  setup();
  uint64_t val1, val2;
  val1 = readctr(0, 0);
#define SZ (1024*1024*32)
  static int buff[SZ];
  int s;
  for (int i = 0; i < SZ; ++i)
    s += ((volatile int *)buff)[i];
  printf("s = %d\n", s);
  val2 = readctr(0, 0);
  printf("ctrs: %lu %lu\n", val1, val2);

  return(0);
}
#endif
