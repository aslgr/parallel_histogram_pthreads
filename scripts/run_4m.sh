#!/bin/bash

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
RESULTS_DIR="$ROOT_DIR/results_4m"

mkdir -p "$RESULTS_DIR"

echo "Hostname: $(hostname)" > "$RESULTS_DIR/machine_info.txt"
echo "" >> "$RESULTS_DIR/machine_info.txt"
echo "=== lscpu ===" >> "$RESULTS_DIR/machine_info.txt"
lscpu >> "$RESULTS_DIR/machine_info.txt"

lstopo "$RESULTS_DIR/lstopo.pdf" || true

for t in 1 2 3 4 5 6 7 8
do
    echo "Running 4M with $t threads"

    "$ROOT_DIR/parallel_histogram" 4000000 1024 32 $t 10 -tb2 \
        > "$RESULTS_DIR/histo_4m_t${t}.txt"
done
