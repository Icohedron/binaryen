(module ;; Based on global_copy.wat from Wasabi's GitHub: https://github.com/danleh/wasabi/blob/50a1244d57b7a945d5a1e646beba22f67ac9c695/tests/inputs/taint/simple/global_copy.wat
    (func $source (result i32) i32.const 69)
    (export "taint_source" (func $source))
    (func $sink (param i32))
    (export "taint_sink" (func $sink))

    (global $globA (mut i32) i32.const 55)

    (func $f
        ;; mark globA as tainted
        call $source
        global.set $globA

        global.get $globA
        call $sink
    )

    (start $f)
)

;; Expected outcome: Taint reaches sink.