#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/.." && pwd)"
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) preset="benchmark" ;;
    *) preset="ninja-benchmark" ;;
esac

if [[ -n "${PYTHON:-}" ]]; then
    python_command="$PYTHON"
elif command -v python3 >/dev/null 2>&1; then
    python_command="python3"
else
    python_command="python"
fi

cd "$repo_root"
cmake --preset "$preset"
cmake --build --preset "$preset"
"$python_command" scripts/bench_history.py "$@"
