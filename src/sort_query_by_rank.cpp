#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <math.h>
#include <fstream>



int main()
{
    std::ifstream in_QueryFile("../query_64000.txt");
    std::ofstream out_QueryFile("../query_sorted_64000.txt");
    
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

    for(int i = 0; i < querys.size() / 10; i++){
        for(int j = 0; j < 10; j++){
            std::string query = querys[(querys.size() / 10) * j + i];
            querys_sorted.push_back(query);
        }
    }

    for(int i = 0; i < querys_sorted.size(); i++){
        out_QueryFile << querys_sorted[i] << "\n";
    }

    return 0;
}