#!/usr/bin/env bash
set -euo pipefail

preset="${1:-linux-gcc-release}"
case "$preset" in
  linux-gcc-release|linux-clang-release)
    ;;
  *)
    echo "Supported presets: linux-gcc-release or linux-clang-release" >&2
    exit 2
    ;;
esac

cmake --preset "$preset"
cmake --build --preset "$preset"
ctest --test-dir "build/${preset#linux-}" --output-on-failure
printf 'Built: build/%s/EpochGuiRoundedCorners\n' "${preset#linux-}"
