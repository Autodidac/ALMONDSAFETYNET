#!/bin/bash
# Prompt for input
read -p "Enter the sample id tag: " sample_id

# Run the Vulkan Samples executable with the input sample id tag
./vulkan_samples sample "$sample_id"

# Pause to view output
read -n 1 -s -r -p "Press any key to continue..."
echo
