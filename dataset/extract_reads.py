import random

nb_of_extracted_reads = 640000

fastq_input = open("ERR174310.fastq", "r")
selected_reads = random.sample(range(0, 999999), nb_of_extracted_reads)
selected_reads.sort()

output_file = open("extracted_" + str(nb_of_extracted_reads) + ".fastq", "w")
output_file2 = open("extracted_sequences_" + str(nb_of_extracted_reads) + ".txt", "w")

for line_nb, line in enumerate(fastq_input):
    if not selected_reads:
        break
    if line_nb // 4 == selected_reads[0]:
        output_file.write(line)
    if (line_nb - 1) / 4 == selected_reads[0]:
        output_file2.write(line)
    if line_nb // 4 > selected_reads[0]:
        selected_reads.pop(0)
    

output_file.close()
fastq_input.close()
