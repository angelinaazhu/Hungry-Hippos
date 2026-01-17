#!/bin/bash

#name without .csv
input=$1

grep -v ROUND "$input.csv" | awk -F' ' '{print $3}' > "${input}_formatted.csv"
cat header "${input}_formatted.csv" > "${input}_formatted_header.csv"
