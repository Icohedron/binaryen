# Taint Analysis Tests
In this directory are tests for the intraprocedural taint analysis pass.

Running a test requires the [WebAssembly Binary Toolkit (wabt)](https://github.com/WebAssembly/wabt), as it uses the `wat2wasm` CLI tool to translate from WebAssembly text format (.wat) to the WebAssembly binary format (.wasm).

To run a test, simply use the provided `test.sh` script, passing in the .wat file to run the analysis on.
```
./test.sh <wat_file>
```

Each of these tests (except for `if_else.wat` and `if_end.wat`) originate from [Wasabi's GitHub](https://github.com/danleh/wasabi/blob/50a1244d57b7a945d5a1e646beba22f67ac9c695/tests/inputs/taint/simple) and are slightly modified due to differences in the way we mark taints.
