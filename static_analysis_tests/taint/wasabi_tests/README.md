# Wasabi Taint Analysis Tests
These tests (aside from `if_else.wat` and `if_end.wat`) originate from [Wasabi's GitHub](https://github.com/danleh/wasabi/blob/50a1244d57b7a945d5a1e646beba22f67ac9c695/tests/inputs/taint/simple).

The included `taint.js` is a modified working version of that in Wasabi's GitHub, as their analysis did not work out of the box, perhaps due to API changes.

To prepare a taint analysis use the included script
```
./buildBrowserTest.sh myProgram.wat taint.js
```

Afterwards, load `myProgram.html` in a web browser. (Located in `myProgram_out`) You may need to start a local web server in order for the WebAssembly code (and analysis) to run correctly.

You will need to install Wasabi and all of its dependencies to use the build script. You may find instructions on [their GitHub](https://github.com/danleh/wasabi#installation-from-source)