import random

selected_reads = random.sample(range(0, 999999), 640)

fastq_input = open("ERR174310.fastq", "r")
output_file = open("extracted.fastq", "w")
output_file2 = open("extracted_sequences.txt", "w")

for line_nb, line in enumerate(fastq_input):
    if line_nb // 4 in selected_reads:
        output_file.write(line)
    if (line_nb - 1) / 4 in selected_reads:
        output_file2.write(line)
        

        


fastq_input.close()
output_file.close()
