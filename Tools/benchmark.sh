#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <test executable>"
    exit 1
fi

BINARY="$1"
RUNS=5
RESULTS=$(mktemp)

trap 'rm -f "$RESULTS"' EXIT

if [ ! -f "$BINARY" ] || [ ! -x "$BINARY" ]; then
    echo "Error: '$BINARY' is not executable."
    exit 1
fi

run_benchmark()
{
    "$BINARY" 2>&1 \
        | sed -n \
            's/.*BENCHMARK_RESULT name=\([^ ]*\) Mcycles\/s=\([0-9.]*\).*/\1 \2/p'
}

#
# Warm up CPU, caches and runtime environment.
#
echo "Warm-up..."

WARMUP=$(run_benchmark)

if [ -z "$WARMUP" ]; then
    echo "Error: No BENCHMARK_RESULT found."
    exit 1
fi

NAME=$(echo "$WARMUP" | awk '{print $1}')

echo "Benchmark: $NAME"
echo

#
# Execute independent benchmark runs.
#
for ((i = 1; i <= RUNS; ++i)); do
    RESULT=$(run_benchmark)

    if [ -z "$RESULT" ]; then
        echo "Run $i: ERROR - no BENCHMARK_RESULT found."
        exit 1
    fi

    RUN_NAME=$(echo "$RESULT" | awk '{print $1}')
    VALUE=$(echo "$RESULT" | awk '{print $2}')

    if [ "$RUN_NAME" != "$NAME" ]; then
        echo "Run $i: ERROR - benchmark name changed."
        exit 1
    fi

    echo "Run $i: $VALUE Mcycles/s"
    echo "$VALUE" >> "$RESULTS"
done

#
# Calculate statistics.
#
awk '
{
    sum += $1

    if (NR == 1 || $1 < min)
        min = $1

    if (NR == 1 || $1 > max)
        max = $1
}
END {
    printf "\nMinimum: %.3f Mcycles/s\n", min
    printf "Maximum: %.3f Mcycles/s\n", max
    printf "Average: %.3f Mcycles/s\n", sum / NR
}
' "$RESULTS"

