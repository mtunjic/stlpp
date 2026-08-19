#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKDIR="$SCRIPT_DIR/../"
BENCH_FILE="$WORKDIR/bench.md"
DATE=$(date +%Y-%m-%d_%H%M%S)

# Step 1: Clean build and configure CMake
cd "$WORKDIR"
mkdir -p build
cd build
cmake --build . --target clean && cmake .. -G "Ninja"

# Ensure we're back in the main directory for subsequent steps
cd ..

# Step 2: Run tests
test -f "build/ctest" && ./build/ctest --output-on-failure || (echo "No test binary found in build directory" && exit 1)

# Step 3: Execute benchmarks and update bench.md
if [ -f "build/benchmarks" ]; then
    mkdir -p "$WORKDIR/bench"
    ./build/benchmarks > "$BENCH_FILE.$DATE.txt"

echo "## Benchmark Results ($DATE)" >> "$BENCH_FILE"
cat "$BENCH_FILE.$DATE.txt" >> "$BENCH_FILE"
rm "$BENCH_FILE.$DATE.txt"
else
    echo "No benchmark binary found in build directory" && exit 1
fi

# Step 4: Generate performance graphs
gnuplot -e "set output '$WORKDIR/bench/graph_$DATE.png'; plot 'bench.md' using 1:2 with lines"
