#include <dpu.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <dpu_profiler.h>
#include <stdlib.h>
#include <string.h>

#define DPU_BINARY "fm_index_dpu"
#define STEP 1  // step size of L column
#define L_LENGTH 16  // length of L column (rows)
#define SAMPLE_RATE 5  // sample rate of occ
#define OCC_INDEX_NUM 5  // number of occs per occ entry (depends on step)
#define QUERY_LENGTH 3  // length of queries
#define QUERY_NUM 2  // number of queries
#define DPU_NUM 64  // number of DPUs

// ivan says hello
// tienshuo says hello
 
int main() {
  

  uint16_t L[L_LENGTH * DPU_NUM];
  uint32_t sampled_OCC[(((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM) * DPU_NUM];
  uint32_t offsets[OCC_INDEX_NUM * DPU_NUM];
  uint32_t query[QUERY_LENGTH * QUERY_NUM];
  uint32_t num_query_found[DPU_NUM * QUERY_NUM];
  uint32_t dpu_index = 0;
  char QUERY[STEP * QUERY_LENGTH] = "TAC";
  uint32_t scale;

  
  struct dpu_set_t set, dpu;

  DPU_ASSERT(dpu_alloc(DPU_NUM, NULL, &set));
  DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));

  // read occ_table, L_column, F_offsets
  FILE *input = fopen("table.txt", "r");
  for(uint32_t i = 0; i < DPU_NUM; i++){
    for(uint32_t j = 0; j < OCC_INDEX_NUM; j++){
      fscanf(input, "%d", &offsets[i * OCC_INDEX_NUM + j]);
    }
    for(uint32_t j = 0; j < L_LENGTH; j++){
      fscanf(input, "%d", &L[i * L_LENGTH + j]);
    }
    for(uint32_t j = 0; j < ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM; j++){
      fscanf(input, "%d", &sampled_OCC[i * ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM + j]);
    }
  }
  fclose(input);


  // prepare host buffer
  DPU_FOREACH(set, dpu) {
    DPU_ASSERT(dpu_prepare_xfer(dpu, &L[dpu_index * L_LENGTH]));
    dpu_index ++;
  }
  // push contents of host buffer to DPUs
  DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "L", 0, sizeof(uint16_t) * L_LENGTH, DPU_XFER_ASYNC));
  dpu_index = 0;

  DPU_FOREACH(set, dpu) {
    DPU_ASSERT(dpu_prepare_xfer(dpu, &sampled_OCC[dpu_index * ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM]));
    dpu_index ++;
  }
  DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "sampled_OCC", 0, sizeof(uint32_t) * ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM, DPU_XFER_ASYNC));
  dpu_index = 0;

  DPU_FOREACH(set, dpu) {
    DPU_ASSERT(dpu_prepare_xfer(dpu, &offsets[dpu_index * OCC_INDEX_NUM]));
    dpu_index ++;
  }
  DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "offsets", 0, sizeof(uint32_t) * OCC_INDEX_NUM, DPU_XFER_ASYNC));
  dpu_index = 0;

  DPU_ASSERT(dpu_sync(set));

  // DPU_FOREACH(set, dpu) {
  //   DPU_ASSERT(dpu_copy_to(dpu, "L", 0, L, sizeof(uint16_t) * L_LENGTH));
  //   DPU_ASSERT(dpu_copy_to(dpu, "sampled_OCC", 0, sampled_OCC, sizeof(uint32_t) * ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM));
  //   DPU_ASSERT(dpu_copy_to(dpu, "offsets", 0, offsets, sizeof(uint32_t) * OCC_INDEX_NUM));
  // }

  for(uint32_t query_num = 0; query_num < QUERY_NUM; query_num++){
    if(query_num == 1) {
      strncpy(QUERY, "CGC", STEP * QUERY_LENGTH);
    }
    for(uint32_t i = 0; i < QUERY_LENGTH; i++){
      scale = 1;
      for(uint32_t j = 0; j < STEP; j++){
        if(QUERY[STEP * i + j] == 'A') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 1;
        if(QUERY[STEP * i + j] == 'C') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 2;
        if(QUERY[STEP * i + j] == 'G') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 3;
        if(QUERY[STEP * i + j] == 'T') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 4;
        scale *= 5;
      }
    }
  }

  for(uint32_t i = 0; i < QUERY_NUM; i++){

    // DPU_FOREACH(set, dpu) {
	  //   DPU_ASSERT(dpu_copy_to(dpu, "query", 0, query, sizeof(query)));
    // }

    DPU_ASSERT(dpu_broadcast_to(set, "query", 0, &query[QUERY_LENGTH * i], sizeof(uint32_t) * QUERY_LENGTH, DPU_XFER_ASYNC));

    DPU_ASSERT(dpu_launch(set, DPU_ASYNCHRONOUS));
  
    DPU_FOREACH(set, dpu)
    {  
        //DPU_ASSERT(dpu_log_read(dpu, stdout));
        //DPU_ASSERT(dpu_copy_from(dpu, "num_query_found", 0, &num_query_found[dpu_index][i], sizeof(int)));
        DPU_ASSERT(dpu_prepare_xfer(dpu, &num_query_found[dpu_index * QUERY_NUM + i]));
        dpu_index ++;
    }
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, "num_query_found", 0, sizeof(uint32_t), DPU_XFER_ASYNC));
    dpu_index = 0;
    //DPU_ASSERT(dpu_sync(set));
  }
  DPU_ASSERT(dpu_sync(set));

  for(uint32_t i = 0; i < DPU_NUM; i++){
    for(uint32_t j = 0; j < QUERY_NUM; j++){
      printf("QUERY %d found in DPU %d: %d   ", j, i, num_query_found[i * QUERY_NUM + j]);
    }
    printf("\n");
  }

  // DPU_ASSERT(dpu_free(set));

  return 0;
}