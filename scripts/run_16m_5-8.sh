#!/bin/bash

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
RESULTS_DIR="$ROOT_DIR/results_16m"

mkdir -p "$RESULTS_DIR"

for t in 5 6 7 8
do
    echo "Running 16M with $t threads"

    "$ROOT_DIR/parallel_histogram" 16000000 1024 32 $t 10 -tb2 \
        > "$RESULTS_DIR/histo_16m_t${t}.txt"
done
