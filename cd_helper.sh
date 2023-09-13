#!/bin/bash

# Set the initial directory (change this to your desired directory)
initial_directory="$PWD"

# Check if the MY_CUSTOM_DIR variable is not set and set it to the initial directory
if [ -z "$MY_CUSTOM_DIR" ]; then
    export MY_CUSTOM_DIR="$initial_directory"
fi

# Define the mycd function to change the directory
aclcd() {

    # Check if the C program executable exists
    /usr/bin/change_dir "$1"

    return_value=$?

    # Print the return value
    # echo "Return value of 'myprogram': $return_value"

    if [ $return_value -eq 0 ]; then
        cd "$1" && export MY_CUSTOM_DIR="$PWD"
        echo "Successfully Changed Directory"
    fi
}

# Change to the initial directory when the script is executed
# cd "$1" && export MY_CUSTOM_DIR="$PWD"