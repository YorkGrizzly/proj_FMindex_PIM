#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include <mram.h>
#include <perfcounter.h>
#define STEP 1
#define L_LENGTH 16
#define SAMPLE_RATE 5
#define OCC_INDEX_NUM 5
#define CHAR_QUERY_LENGTH 4
#define QUERY_LENGTH (CHAR_QUERY_LENGTH / STEP)

__mram uint16_t L[L_LENGTH];
__mram uint32_t sampled_OCC[((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM];
__host uint32_t offsets[OCC_INDEX_NUM];
__host uint32_t query[QUERY_LENGTH];
__host uint32_t num_query_found;

int main() {
    // for(uint32_t i = 0; i < L_LENGTH; i++){
    //     printf("L: %d\n", L[i]);
    // }
    // for(uint32_t i = 0; i < OCC_INDEX_NUM; i++){
    //     printf("offsets: %d\n", offsets[i]);
    // }
    // for(uint32_t i = 0; i < ((L_LENGTH - 1) / (SAMPLE_RATE) + 1) * OCC_INDEX_NUM; i++){
    //     printf("sampled_OCC: %d\n", sampled_OCC[i]);
    // }
    // for(uint32_t i = 0; i < QUERY_LENGTH; i++){
    //     printf("query: %d\n", query[i]);
    // }
    uint32_t range_min;
    uint32_t range_max;
    uint32_t update_range_min;
    uint32_t update_range_max;
    uint32_t SEARCH_ROUND = QUERY_LENGTH - 1;
    
    num_query_found = 0;

    if(offsets[query[0]] == 0) SEARCH_ROUND = 0;
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
        if(offsets[query[search_round + 1]] == 0) break;
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

        if(update_range_min > update_range_max){
            num_query_found = 0;
            break;
        }

        if(search_round == SEARCH_ROUND - 1){
            num_query_found = update_range_max - update_range_min + 1;
        }

        range_min = update_range_min;
        range_max = update_range_max;
    }
    //printf("num_query_found: %d\n", num_query_found);
    return 0;
}