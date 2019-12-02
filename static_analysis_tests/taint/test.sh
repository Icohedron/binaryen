#!/bin/bash

TMP_WASM_OUT="${1}.tmp.wasm"

wat2wasm ${1} --debug-names -o TMP_WASM_OUT
wasm-opt TMP_WASM_OUT --taint
rm -f TMP_WASM_OUT
