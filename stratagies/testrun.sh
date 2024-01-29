#!/bin/bash

# Number of times to run the process
num_instances=10

# Base command
base_command="/home/vivek/Desktop/projects/cpp/Trading-Project/stratagies/sampleStrat"

# Output file path
output_file_base="/home/vivek/Desktop/projects/cpp/Trading-Project/temp/instance"

# Alternative for loop syntax
for i in $(seq 1 $num_instances); do
    output_file="${output_file_base}${i}.txt"
    nohup $base_command > $output_file 2>&1 &
    echo "Instance $i started. Output file: $output_file"
done
