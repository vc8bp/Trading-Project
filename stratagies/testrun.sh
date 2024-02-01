#!/bin/bash

# Number of times to run the process
num_instances=100

# Base command
base_command="/home/finrise/project/Trading-Project/stratagies/sampleStrat"

# Output file path
output_file_base="/home/finrise/project/Trading-Project/stratagies/temp"

# Alternative for loop syntax
for i in $(seq 1 $num_instances); do
    output_file="${output_file_base}${i}.txt"
    nohup $base_command > $output_file 1 2>&1 &
    echo "Instance $i started. Output file: $output_file"
done
