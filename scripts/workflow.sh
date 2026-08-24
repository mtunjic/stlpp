#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/.." && pwd)"
preset=""
fresh=0
clean_first=0
warnings_as_errors=0
no_test=0
jobs=""

usage() {
    cat <<'EOF'
Usage: scripts/workflow.sh [options]

Options:
  --preset NAME          CMake build/test preset (default: platform Debug)
  --fresh                Recreate CMake configure metadata
  --clean-first          Clean compiled artifacts before building
  --warnings-as-errors   Configure STLPP_WARNINGS_AS_ERRORS=ON
  --no-test              Configure and build without running CTest
  --jobs N               Maximum parallel build jobs (1-256)
  -h, --help             Show this help
EOF
}

while (($#)); do
    case "$1" in
        --preset)
            preset="${2:?--preset requires a value}"
            shift 2
            ;;
        --fresh)
            fresh=1
            shift
            ;;
        --clean-first)
            clean_first=1
            shift
            ;;
        --warnings-as-errors)
            warnings_as_errors=1
            shift
            ;;
        --no-test)
            no_test=1
            shift
            ;;
        --jobs)
            jobs="${2:?--jobs requires a value}"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ -n "$jobs" ]] && { [[ ! "$jobs" =~ ^[0-9]+$ ]] || ((jobs < 1 || jobs > 256)); }; then
    echo "--jobs must be an integer from 1 through 256" >&2
    exit 2
fi

if [[ -z "$preset" ]]; then
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*) preset="debug" ;;
        *) preset="ninja-debug" ;;
    esac
fi

case "$preset" in
    debug|release|relwithdebinfo|sanitize|ninja-debug|ninja-release|ninja-relwithdebinfo|ninja-sanitize) ;;
    *)
        echo "Preset '$preset' is not a build-and-test preset" >&2
        exit 2
        ;;
esac

cd "$repo_root"
configure=(cmake --preset "$preset")
((fresh)) && configure+=(--fresh)
((warnings_as_errors)) && configure+=(-DSTLPP_WARNINGS_AS_ERRORS=ON)
printf '> %q ' "${configure[@]}"
printf '\n'
"${configure[@]}"

build=(cmake --build --preset "$preset")
((clean_first)) && build+=(--clean-first)
[[ -n "$jobs" ]] && build+=(--parallel "$jobs")
printf '> %q ' "${build[@]}"
printf '\n'
"${build[@]}"

if ((!no_test)); then
    printf '> %q ' ctest --preset "$preset" --output-on-failure
    printf '\n'
    ctest --preset "$preset" --output-on-failure
fi
