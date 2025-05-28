cd ~/graph-analytics

# results.csv
rm -f results.csv
echo "file,model,elapsed,maxRSS" > results.csv
for f in bench_tc_bench_*.out; do
  line=$(head -n1 "$f")
  # skip any bad scripts
  [[ "$line" == *\$* ]] && continue
  model=$(echo "$line" | awk '{print $2}')
  elapsed=$(grep -m1 '^elapsed=' "$f" | sed -E 's/elapsed=([^ ]+).*/\1/')
  rss=$(grep -m1 'maxRSS'   "$f" | sed -E 's/.*maxRSS\(MB\)=([0-9]+).*/\1/')
  echo "$f,$model,$elapsed,$rss" >> results.csv
done