#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <signal.h>

#define STEP 4  // step size of L column
#define L_LENGTH 102 * READ_NUM  // length of L column (rows)
#define SAMPLE_RATE 64  // sample rate of occ
#define OCC_INDEX_NUM 625  // number of occs per occ entry (depends on step)
#define CHAR_QUERY_LENGTH 48  // length of searching genome
#define QUERY_NUM 640  // number of queries
#define READ_NUM 640  // number of DPUs
#define QUERY_LENGTH (CHAR_QUERY_LENGTH / STEP)  // length of encoded queries

 
int main() {
  
  clock_t start, finish;
  double duration;
  //uint16_t L[L_LENGTH];
  uint32_t *L = malloc(sizeof(uint32_t) * L_LENGTH);
  //uint32_t sampled_OCC[(((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM) * READ_NUM];
  uint32_t *sampled_OCC = malloc(sizeof(uint32_t) * (((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM));
  //uint32_t offsets[OCC_INDEX_NUM * READ_NUM];
  uint32_t *offsets = malloc(sizeof(uint32_t) * OCC_INDEX_NUM);
  //uint32_t query[QUERY_LENGTH * QUERY_NUM];
  uint32_t *query_all = malloc(sizeof(uint32_t) * QUERY_LENGTH * QUERY_NUM);
  uint32_t *query = malloc(sizeof(uint32_t) * QUERY_LENGTH);
  //uint32_t num_query_found[READ_NUM * QUERY_NUM];
  uint32_t *num_query_found = malloc(sizeof(uint32_t) * QUERY_NUM);

  uint32_t scale = 1;
  char QUERY[CHAR_QUERY_LENGTH];

  start = clock();

  FILE *input_table = fopen("../table_soft.txt", "r");
  for(uint32_t j = 0; j < OCC_INDEX_NUM; j++){
    fscanf(input_table, "%d", &offsets[j]);
  }
  for(uint32_t j = 0; j < L_LENGTH; j++){
    fscanf(input_table, "%d", &L[j]);
  }
  for(uint32_t j = 0; j < (((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM); j++){
    fscanf(input_table, "%d", &sampled_OCC[j]);
  }
  fclose(input_table);



  FILE *input_query = fopen("../query.txt", "r");

  for(uint32_t query_num = 0; query_num < QUERY_NUM; query_num++){
    fscanf(input_query, "%s\n", QUERY);
    //printf("QUERY %d: %s\n", query_num, QUERY);
    for(uint32_t i = 0; i < QUERY_LENGTH; i++){
      scale = 1;
      query_all[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] = 0;
      for(uint32_t j = 0; j < STEP; j++){
        if(QUERY[STEP * i + STEP - j - 1] == 'A') query_all[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 1;
        if(QUERY[STEP * i + STEP - j - 1] == 'C') query_all[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 2;
        if(QUERY[STEP * i + STEP - j - 1] == 'G') query_all[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 3;
        if(QUERY[STEP * i + STEP - j - 1] == 'T') query_all[query_num * QUERY_LENGTH + (QUERY_LENGTH - 1 - i)] += scale * 4;
        scale *= 5;
      }
    }
  }

  finish = clock();
  duration = (double)(finish - start) / CLOCKS_PER_SEC;
  printf("%f seconds\n", duration);

  uint32_t range_min;
  uint32_t range_max;
  uint32_t update_range_min;
  uint32_t update_range_max;
  uint32_t SEARCH_ROUND = QUERY_LENGTH - 1;
  bool not_found_flag = 0;

  start = clock();


  for(uint32_t query_index = 0; query_index < QUERY_NUM; query_index++){

    for(uint32_t j = 0; j < QUERY_LENGTH; j++){
        query[j] = query_all[QUERY_LENGTH * query_index + j];
    }

    num_query_found[query_index] = 0;
    // num_query_found = 0;

    if(offsets[query[0]] == 0) {
        SEARCH_ROUND = 0;
        not_found_flag = 1;
    }
    else range_min = offsets[query[0]] - 1;

    if(query[0] == OCC_INDEX_NUM - 1) range_max = L_LENGTH - 1;
    else{
        for(uint32_t i = 0; i < OCC_INDEX_NUM - query[0] - 1; i++){
            if(offsets[query[0] + i + 1] != 0){
                range_max = offsets[query[0] + i + 1] - 1 - 1;
                break;
            }
            if(i == OCC_INDEX_NUM - query[0] - 2) range_max = L_LENGTH - 1;
        }
    }

    //printf("range_min: %d, range_max: %d\n", range_min, range_max);

    for(uint32_t search_round = 0; search_round < SEARCH_ROUND; search_round++){
        if(offsets[query[search_round + 1]] == 0) {
            not_found_flag = 1;
            break;
        }
        //update range_min
        if((range_min % SAMPLE_RATE <= SAMPLE_RATE / 2) || (range_min / SAMPLE_RATE >= (L_LENGTH - 1) / SAMPLE_RATE)){
            update_range_min = offsets[query[search_round + 1]] - 1 + sampled_OCC[((range_min / SAMPLE_RATE) * OCC_INDEX_NUM) + query[search_round + 1]] - 1;
            for(uint32_t k = 1; k <= range_min % SAMPLE_RATE; k++){
                if(L[SAMPLE_RATE * (range_min / SAMPLE_RATE) + k] == query[search_round + 1]){
                update_range_min += 1;
                }
            }
        }
        else if((range_min % SAMPLE_RATE > SAMPLE_RATE / 2) && (range_min / SAMPLE_RATE < (L_LENGTH - 1) / SAMPLE_RATE)){
            update_range_min = offsets[query[search_round + 1]] - 1 + sampled_OCC[((range_min / SAMPLE_RATE + 1) * OCC_INDEX_NUM) + query[search_round + 1]] - 1;
            for(uint32_t k = 0; k < SAMPLE_RATE - range_min % SAMPLE_RATE; k++){
                if(L[SAMPLE_RATE * (range_min / SAMPLE_RATE + 1) - k] == query[search_round + 1]){
                update_range_min -= 1;
                }
            }
        }
        if(L[range_min] != query[search_round + 1]) update_range_min += 1;

        //update range_max
        if((range_max % SAMPLE_RATE <= SAMPLE_RATE / 2) || (range_max / SAMPLE_RATE >= (L_LENGTH - 1) / SAMPLE_RATE)){
            update_range_max = offsets[query[search_round + 1]] - 1 + sampled_OCC[((range_max / SAMPLE_RATE) * OCC_INDEX_NUM) + query[search_round + 1]] - 1;
            for(uint32_t k = 1; k <= range_max % SAMPLE_RATE; k++){
                if(L[SAMPLE_RATE * (range_max / SAMPLE_RATE) + k] == query[search_round + 1]){
                update_range_max += 1;
                }
            }
        }
        else if((range_max % SAMPLE_RATE > SAMPLE_RATE / 2) && (range_max / SAMPLE_RATE < (L_LENGTH - 1) / SAMPLE_RATE)){
            update_range_max = offsets[query[search_round + 1]] - 1 + sampled_OCC[((range_max / SAMPLE_RATE + 1) * OCC_INDEX_NUM) + query[search_round + 1]] - 1;
            for(uint32_t k = 0; k < SAMPLE_RATE - range_max % SAMPLE_RATE; k++){
                if(L[SAMPLE_RATE * (range_max / SAMPLE_RATE + 1) - k] == query[search_round + 1]){
                update_range_max -= 1;
                }
            }
        }

        //printf("updated range_min: %d, updated range_max: %d\n", update_range_min, update_range_max);

        range_min = update_range_min;
        range_max = update_range_max;

        if(range_min > range_max) break;

        
        //printf("real search_round: %d\n", search_round);

    }
    //printf("real search_round: %d\n", search_round_act);
    if(range_min > range_max || not_found_flag == 1) num_query_found[query_index] = 0;
    else num_query_found[query_index] = range_max - range_min + 1;

  }
  
  finish = clock();
  duration = (double)(finish - start) / CLOCKS_PER_SEC;
  printf("%f seconds\n", duration);




  for(uint32_t i = 0; i < QUERY_NUM; i++){
    //printf("QUERY %d found: %d\n", i, num_query_found[i]);
  }
  free(num_query_found);

  return 0;
}