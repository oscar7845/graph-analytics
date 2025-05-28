#!/bin/bash
#SBATCH -p general
#SBATCH --qos=low
#SBATCH -N1
#SBATCH -n1
#SBATCH -t 00:40:00
#SBATCH -J tc_bench
#SBATCH -o bench_%x_%j.out

module purge
module load easybuild
module load foss/2023b

exe=$1
graph=$2

echo "=== $(basename "$exe") on $graph ==="

#set the shell’s built‑in timer format
TIMEFORMAT='elapsed=%6R'
# run the shell-built-in time (not /usr/bin/time) and merge its stderr into stdout
{ time "$exe" $graph; } 2>&1

# one-shot GNU time for memory
#/usr/bin/time -f 'maxRSS(MB)=%M' $exe $graph
/usr/bin/timeout --preserve-status 1100s { time "$exe" $graph; } 2>&1

perf stat -e cache-misses,cycles,instructions -- $exe $graph
