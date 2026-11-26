#!/bin/sh
set -e
ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
CXX="${CXX:-g++}"
OUT="$ROOT/tests/test_sendcommand_safety"
$CXX -std=c++11 -Wall -Wextra -fsanitize=address,undefined \
  -I "$ROOT/tests/stubs" -I "$ROOT" \
  "$ROOT/tests/test_sendcommand_safety.cpp" "$ROOT/mmwave_for_xiao.cpp" \
  -o "$OUT"
"$OUT"
