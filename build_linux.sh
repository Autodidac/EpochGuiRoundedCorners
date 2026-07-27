#!/usr/bin/env bash
set -euo pipefail

preset="${1:-linux-gcc-release}"
case "$preset" in
  linux-gcc-release)
    build_dir="build/linux-gcc"
    ;;
  linux-clang-release)
    build_dir="build/linux-clang"
    ;;
  *)
    echo "Supported presets: linux-gcc-release or linux-clang-release" >&2
    exit 2
    ;;
esac

cmake --preset "$preset"
cmake --build --preset "$preset"
ctest --test-dir "$build_dir" --output-on-failure
printf 'Built: %s/EpochGuiRoundedCorners\n' "$build_dir"
