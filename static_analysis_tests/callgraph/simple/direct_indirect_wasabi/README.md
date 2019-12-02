# Wasabi direct_indirect.wat Test

A pre-built test for call graph construction of `direct_indirect.wat` using Wasabi.

Simply run open `direct_indirect.html` in a web browser. (You may need to start a web server for resources to load correctly.)

In the web browser console, you should get the following message
```
Starting call tracing. Check for results in Wasabi.analysisResult.
```
From there, you can run the command `saveCallGraph(Wasabi.analysisResult)` to download a Graphviz DOT file that can be used to generate the visual call graph.