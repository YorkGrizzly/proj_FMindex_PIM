#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <math.h>
#include <fstream>



int main()
{
    std::ifstream in_ReadFile("../dataset/extracted_sequences_6400.txt");
    std::ofstream out_ReadFile("../dataset/extracted_sorted_sequences_6400.txt");
    
    std::vector<std::string> reads;
    // const std::string reference_string = "ATCGAGCGCGCATCG$";
    // std::cout << "size of reference: " << reference_string.length() << std::endl;

    //std::vector<std::string> sorted_strings;

    // rotate and sort reads from table.txt
    for (std::string read; getline(in_ReadFile, read);)
    {
        reads.push_back(read);
    }

    std::sort(reads.begin(), reads.end());

    for(int i = 0; i < reads.size(); i++){
        out_ReadFile << reads[i] << "\n";
    }

    return 0;
}
