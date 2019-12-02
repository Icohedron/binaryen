(module ;; Based on unary_op.wat from Wasabi's GitHub: https://github.com/danleh/wasabi/blob/50a1244d57b7a945d5a1e646beba22f67ac9c695/tests/inputs/taint/simple/unary_op.wat
    (import "imports" "output" (func $print (param i32)))

    (func $source (result i32) i32.const 5)
    (export "taint_source" (func $source))
    (func $sink (param i32))
    (export "taint_sink" (func $sink))

    (func $f (local $locA i32) (local $locB i32)

        ;; mark locA as tainted
        call $source
        local.set $locA

        ;; unary operation involving tainted locA
        local.get $locA
        i32.eqz

        ;; pass result to sink
        call $sink
    )

    (start $f)
)

;; Expected outcome: Taint reaches sink.