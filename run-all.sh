#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "$0 [system_arch] [jobs_file]"
    exit
fi

./launch-jobs.sh --sim $@
./launch-jobs.sh --real $@
