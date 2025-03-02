#!/bin/bash
array=(jerry tom alice keven julie)
index=0
while [ $index -lt ${#array[@]} ]; do
    echo "array[$index]=${array[index]}"
    index=$(($index+1))
done
echo "all array is ${array[*]}"
array[10]="hello"
array[20]="world"
echo "array2[10]=${array2[10]}"
echo "array2[15]=${array2[15]}"
echo "array2[20]=${array2[20]}"
