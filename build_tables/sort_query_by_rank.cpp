#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <math.h>
#include <fstream>

#define READS_PER_DPU 1000

int main()
{
    std::ifstream in_QueryFile("../tables_and_queries/query_640000.txt");
    std::ofstream out_QueryFile("../tables_and_queries/query_sorted_640000.txt");
    
    std::vector<std::string> querys;
    std::vector<std::string> querys_sorted;
    // const std::string reference_string = "ATCGAGCGCGCATCG$";
    // std::cout << "size of reference: " << reference_string.length() << std::endl;

    //std::vector<std::string> sorted_strings;

    // rotate and sort reads from table.txt
    for (std::string query; getline(in_QueryFile, query);)
    {
        querys.push_back(query);
    }

    for(int i = 0; i < READS_PER_DPU; i++){
        for(int j = 0; j < querys.size() / (READS_PER_DPU * 10); j++){
            for(int k = 0; k < 10; k++){
                std::string query = querys[(querys.size() / 10) * k + j * READS_PER_DPU + i];
                printf("%d\n", (querys.size() / 10) * k + j * READS_PER_DPU + i);
                querys_sorted.push_back(query);
            }
        }
    }

    for(int i = 0; i < querys_sorted.size(); i++){
        out_QueryFile << querys_sorted[i] << "\n";
    }

    return 0;
}
