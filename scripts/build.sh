#!/bin/bash
# Script to build, test, and benchmark algorithms

BUILD_DIR="build"
DATE=$(date +%Y%m%d_%H%M%S)

# Clean previous builds
rm -rf $BUILD_DIR

# Configure and build
mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake ..
make

cd ..

# Run tests
cmake --build $BUILD_DIR --target test
ctest --output-junit test-results-$DATE.xml

# Execute benchmarks
$BUILD_DIR/benchmarks

# Update benchmark results
cp $BUILD_DIR/results/*.csv bench.md

echo "Benchmark results updated on $DATE" >> bench.md