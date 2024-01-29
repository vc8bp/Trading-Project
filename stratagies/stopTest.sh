#!/bin/bash

# Base command used in the run_instances.sh script
base_command="/home/vivek/Desktop/projects/cpp/Trading-Project/stratagies/sampleStrat"

# Kill all processes related to the base command
pkill -f "$base_command"

echo "All instances of $base_command killed."