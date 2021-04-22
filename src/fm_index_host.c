#include <dpu.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <dpu_profiler.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DPU_BINARY "fm_index_dpu"
#define STEP 4  // step size of L column
#define L_LENGTH 102  // length of L column (rows)
#define SAMPLE_RATE 64  // sample rate of occ
#define OCC_INDEX_NUM 625  // number of occs per occ entry (depends on step)
#define CHAR_QUERY_LENGTH 48  // length of searching genome
#define QUERY_NUM 640  // number of queries
#define DPU_NUM 640  // number of DPUs
#define QUERY_LENGTH (CHAR_QUERY_LENGTH / STEP)  // length of encoded queries

 
int main() {
  

  //uint16_t L[L_LENGTH * DPU_NUM];
  uint32_t *L = malloc(sizeof(uint32_t) * L_LENGTH * DPU_NUM);
  //uint32_t sampled_OCC[(((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM) * DPU_NUM];
  uint32_t *sampled_OCC = malloc(sizeof(uint32_t) * (((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM) * DPU_NUM);
  //uint32_t offsets[OCC_INDEX_NUM * DPU_NUM];
  uint32_t *offsets = malloc(sizeof(uint32_t) * OCC_INDEX_NUM * DPU_NUM);
  //uint32_t query[QUERY_LENGTH * QUERY_NUM];
  uint32_t *query = malloc(sizeof(uint32_t) * QUERY_LENGTH * QUERY_NUM);
  //uint32_t num_query_found[DPU_NUM * QUERY_NUM];
  uint32_t *num_query_found = malloc(sizeof(uint32_t) * DPU_NUM * QUERY_NUM);
  uint32_t dpu_index = 0;
  uint32_t scale = 1;
  uint32_t num_query_found_total;
  char QUERY[CHAR_QUERY_LENGTH];

  
  struct dpu_set_t set, dpu;

  DPU_ASSERT(dpu_alloc(DPU_NUM, NULL, &set));
  DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));

  // read occ_table, L_column, F_offsets
  FILE *input_table = fopen("../table.txt", "r");
  for(uint32_t i = 0; i < DPU_NUM; i++){
    for(uint32_t j = 0; j < OCC_INDEX_NUM; j++){
      fscanf(input_table, "%d", &offsets[i * OCC_INDEX_NUM + j]);
    }
    for(uint32_t j = 0; j < L_LENGTH; j++){
      fscanf(input_table, "%d", &L[i * L_LENGTH + j]);
    }
    for(uint32_t j = 0; j < ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM; j++){
      fscanf(input_table, "%d", &sampled_OCC[i * ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM + j]);
    }
  }
  fclose(input_table);


  // prepare host buffer
  DPU_FOREACH(set, dpu) {
    DPU_ASSERT(dpu_prepare_xfer(dpu, &L[dpu_index * L_LENGTH]));
    dpu_index ++;
  }
  // push contents of host buffer to DPUs
  DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "L", 0, sizeof(uint32_t) * L_LENGTH, DPU_XFER_ASYNC));
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

  free(L);
  free(sampled_OCC);
  free(offsets);

  // DPU_FOREACH(set, dpu) {
  //   DPU_ASSERT(dpu_copy_to(dpu, "L", 0, L, sizeof(uint16_t) * L_LENGTH));
  //   DPU_ASSERT(dpu_copy_to(dpu, "sampled_OCC", 0, sampled_OCC, sizeof(uint32_t) * ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM));
  //   DPU_ASSERT(dpu_copy_to(dpu, "offsets", 0, offsets, sizeof(uint32_t) * OCC_INDEX_NUM));
  // }

  FILE *input_query = fopen("../query.txt", "r");

  for(uint32_t query_num = 0; query_num < QUERY_NUM; query_num++){
    fscanf(input_query, "%s\n", QUERY);
    //printf("QUERY %d: %s\n", query_num, QUERY);
    for(uint32_t i = 0; i < QUERY_LENGTH; i++){
      scale = 1;
      query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] = 0;
      for(uint32_t j = 0; j < STEP; j++){
        if(QUERY[STEP * i + STEP - j - 1] == 'A') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 1;
        if(QUERY[STEP * i + STEP - j - 1] == 'C') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 2;
        if(QUERY[STEP * i + STEP - j - 1] == 'G') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 3;
        if(QUERY[STEP * i + STEP - j - 1] == 'T') query[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 4;
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
  
    // DPU_FOREACH(set, dpu)
    // {  
    //     //DPU_ASSERT(dpu_log_read(dpu, stdout));
    //     //DPU_ASSERT(dpu_copy_from(dpu, "num_query_found", 0, &num_query_found[dpu_index][i], sizeof(int)));
    //     DPU_ASSERT(dpu_prepare_xfer(dpu, &num_query_found[dpu_index * QUERY_NUM + i]));
    //     dpu_index ++;
    // }
    // DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, "num_query_found", 0, sizeof(uint32_t), DPU_XFER_ASYNC));
    // dpu_index = 0;
    //DPU_ASSERT(dpu_sync(set));
  }

  DPU_FOREACH(set, dpu) {
    DPU_ASSERT(dpu_prepare_xfer(dpu, &num_query_found[dpu_index * QUERY_NUM]));
    dpu_index ++;
  }
  DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, "num_query_found", 0, sizeof(uint32_t) * QUERY_NUM, DPU_XFER_ASYNC));
  dpu_index = 0;

  DPU_ASSERT(dpu_sync(set));

  DPU_ASSERT(dpu_free(set));
  free(query);

  num_query_found_total = 0; 

  for(uint32_t i = 0; i < QUERY_NUM; i++){
    for(uint32_t j = 0; j < DPU_NUM; j++){
      num_query_found_total += num_query_found[j * QUERY_NUM + i];
      //printf("QUERY %d found in DPU %d: %d   ", j, i, num_query_found[i * QUERY_NUM + j]);
    }
    printf("QUERY %d found: %d\n", i, num_query_found_total);
    num_query_found_total = 0;
  }
  free(num_query_found);

  return 0;
}