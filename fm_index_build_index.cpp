#include<iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <math.h>

#define STEP 1
#define OCC_SAMPLING_DIST 5

// void print_vector(std::vector<auto> my_vec)
// {
//     for(auto &el : my_vec)
//     {
//         std::cout << my_vec << " ";
//     }
//     std::cout << std::endl;
// }

std::vector<std::string> rotate_and_sort(const std::string &reference_string)
{
    std::vector<std::string> rotated_and_sorted_strings;

    std::string current_rotation = reference_string;
    // std::cout << current_rotation << std::endl;
    for(size_t i = 0; i < reference_string.length(); ++i)
    {
        current_rotation.push_back(current_rotation[0]);
        current_rotation.erase(current_rotation.begin());
        rotated_and_sorted_strings.push_back(current_rotation);
    }
    std::sort(rotated_and_sorted_strings.begin(), rotated_and_sorted_strings.end());
    return rotated_and_sorted_strings;
}

uint32_t encode_to_int(std::string string_to_encode)
{
    uint32_t encoded_value = 0;
    for(size_t i = 0; i < string_to_encode.size(); ++i)
    {
        switch(string_to_encode[i])
        {
            case '$':
                // encoded_value += 0 * pow(5, i);
            break;
            case 'A':
                encoded_value += 1 * pow(5, i);
            break;
            case 'C':
                encoded_value += 2 * pow(5, i);
            break;
            case 'G':
                encoded_value += 3 * pow(5, i);
            break;
            case 'T':
                encoded_value += 4 * pow(5, i);
            break;
        }
    }
    return encoded_value;
}

void extract_tables(const std::vector<std::string> &rotated_and_sorted_strings, std::vector<size_t> &F_offsets, std::vector<uint32_t> &L_column, std::vector<std::vector<uint32_t>> &occ_table)
{
    std::vector<uint32_t> running_occ(pow(5, STEP), 0);
    std::cout << running_occ.size() << std::endl;
    for(size_t i = 0; i < rotated_and_sorted_strings.size(); ++i)
    {
        // F_offsets
        char first_char = rotated_and_sorted_strings[i][0];
        switch(first_char)
        {
            case '$':
                if(F_offsets[0] == 0)
                {
                    F_offsets[0] = i;
                }
                break;
            case 'A':
                if(F_offsets[1] == 0)
                {
                    F_offsets[1] = i;
                }
                break;
            case 'C':
                if(F_offsets[2] == 0)
                {
                    F_offsets[2] = i;
                }
                break;
            case 'G':
                if(F_offsets[3] == 0)
                {
                    F_offsets[3] = i;
                }
                break;
            case 'T':
                if(F_offsets[4] == 0)
                {
                    F_offsets[4] = i;
                }
                break;
        }

        uint32_t encoded_L = encode_to_int(rotated_and_sorted_strings[i].substr(rotated_and_sorted_strings.size() - STEP));
        // L_column
        L_column.push_back(encoded_L);

        // occ_table
        ++running_occ[encoded_L];
        if(i % OCC_SAMPLING_DIST == 0)
        {
            std::cout << "sampled at " <<  i << std::endl;
            occ_table.push_back(running_occ);
        }


    }

}


int main()
{
    const std::string reference_string = "TCGTTCAGGACCTAC$";
    // std::cout << reference_string.length() << std::endl;

    std::vector<std::string> sorted_strings = rotate_and_sort(reference_string);
    std::vector<size_t> F_offsets = {0, 0, 0, 0, 0};  // $, A, C, G, T
    std::vector<uint32_t> L_column;
    std::vector<std::vector<uint32_t>> occ_table;

    // print rotated strings
    for(std::string& _str : sorted_strings)
    {
        std::cout << _str << std::endl;
    }
   
    extract_tables(sorted_strings, F_offsets, L_column, occ_table);

    // print F offsets
    std::cout << "printing F offsets:" << std::endl;
    for(int a : F_offsets)
    {
        std::cout << a << std::endl;
    }
    // print L column
    std::cout << "printing L column:" << std::endl;
    for(uint32_t a : L_column)
    {
        std::cout << a << std::endl;
    }
    // print occ table
    std::cout << "printing Occ table:" << std::endl;
    for(auto &occ_entry : occ_table)
    {
        for(uint32_t &occ : occ_entry)
        {
            std::cout << occ << " ";
        }
        std::cout << std::endl;
    }


}