#!/bin/bash

MODELS=(
  #openai_o3
  gemini_2.5_flash
  #claude_3.7_extended
  #p2_openai_o3
  deepseek_r1
)

GRAPHS=(
  "-r 12"
  #"-f /scratch/bader/oc59/graphs/soc-LiveJournal1.mtx"
  #"-f /scratch/bader/oc59/graphs/cit-Patents.mtx"
  #"-f /scratch/bader/oc59/graphs/ca-GrQc.mtx"
)

for m in "${MODELS[@]}"; do
  exe=~/graph-analytics/llm_variants/${m}/tc_${m}
  for g in "${GRAPHS[@]}"; do
      sbatch bench_tc.slurm "$exe" "$g"
  done
done