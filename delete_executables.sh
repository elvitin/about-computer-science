#!/bin/bash

# This script will execute the 'file -h' command on all files in the current directory, check if the output contains the string "ELF", and delete the file if it does

for file in *
do
    if [[ $(file -h "$file") == *"ELF"* ]]
    then
        rm "$file"
        echo "Deleted $file"
    fi
done