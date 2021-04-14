#include <assert.h>
#include <dpu.h>
#include <dpu_log.h>
#include <stdio.h>

#ifndef DPU_BINARY
// #define DPU_BINARY "./helloworld"
#endif

int main(void) {
  struct dpu_set_t set, dpu;
  uint32_t nr_ranks;
  uint32_t nr_dpus;

  DPU_ASSERT(dpu_alloc(DPU_ALLOCATE_ALL, "backend=hw", &set));
  DPU_ASSERT(dpu_get_nr_ranks(set, &nr_ranks));
  DPU_ASSERT(dpu_get_nr_dpus(set, &nr_dpus));
  DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));

  DPU_FOREACH(set, dpu) {
    // /*DPU_ASSERT(dpu_log_read(dpu, stdout));*/
  }
  printf("number of ranks: %d\n", nr_ranks);
  printf("number of dpus on rank: %d\n", nr_dpus);

  DPU_ASSERT(dpu_free(set));

  return 0;
}
