#!/bin/bash
for i in `seq 2 10`;
do
    for a in `seq 1 10`;
    do
	{ time ./mfind -p $i /pkg comsol > /dev/null 2>&1 ; } 2>> test.txt
    done
done 
