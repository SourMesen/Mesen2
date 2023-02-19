#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$(realpath "$0")")"

TSAN_OPTIONS="suppressions=$PWD/suppressions log_path=$PWD/tsan.log" LD_PRELOAD=/usr/lib/libtsan.so ../bin/linux-x64/Release/linux-x64/publish/Mesen
