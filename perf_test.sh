#!/bin/bash

START=$SECONDS
./walk --if=dataset/facebook_1_1.txt --of=dataset/walk/facebook_1_1.walk --length=100 --walks=100
ELAPSED=$(($SECONDS - $START))

echo "$(($ELAPSED/60)) min $(($ELAPSED%60)) sec"
