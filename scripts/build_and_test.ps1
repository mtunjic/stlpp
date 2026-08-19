# build_and_test.ps1
$ErrorActionPreference = "Stop"

$SCRIPT_DIR = (Get-Item -Path ".\" -ErrorAction Stop).FullName
$WORKDIR = "$SCRIPT_DIR\.."
$BENCH_FILE = "$WORKDIR\bench.md"
$DATE = Get-Date -Format yyyyMMdd_HHmmss

# Step 1: Clean build
Set-Location -Path $WORKDIR
mkdir -Force build
Set-Location -Path build
cmake --build . --target clean
cmake ..

Set-Location -Path $WORKDIR

# Step 2: Run tests
if (Test-Path "build\ctest.exe") {
    & "build\ctest.exe" --output-on-failure
} else {
    Write-Error "No test binary found in build directory"
    exit 1
}

# Step 3: Execute benchmarks and update bench.md
if (Test-Path "build\benchmarks.exe") {
    $benchOutput = "$BENCH_FILE.$DATE.txt"
    & "build\benchmarks.exe" | Out-File -FilePath $benchOutput

    Add-Content -Path $BENCH_FILE -Value "## Benchmark Results ($DATE)"
    Get-Content -Path $benchOutput | Add-Content -Path $BENCH_FILE
    Remove-Item -Path $bench优化内容
} else {
    Write-Error "No benchmark binary found in build directory"
    exit 1
}

# Step 4: Generate performance graphs
if (Test-Path "C:\Program Files\gnuplot\bin\pgnuplot.exe") {
    & "C:\Program Files\gnuplot\bin\pgnuplot.exe" -e "set output '$WORKDIR\bench\graph_$DATE.png'; plot 'bench.md' using 1:2 with lines"
} else {
    Write-Warning "Gnuplot not found at expected location. Graph generation skipped"
}