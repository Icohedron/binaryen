(module ;; Based on no_copy_local_local.wat from Wasabi's GitHub: https://github.com/danleh/wasabi/blob/50a1244d57b7a945d5a1e646beba22f67ac9c695/tests/inputs/taint/simple/no_copy_local_local.wat
    (import "imports" "output" (func $print (param i32)))

    (func $source (result i32) i32.const 5)
    (export "taint_source" (func $source))
    (func $sink (param i32))
    (export "taint_sink" (func $sink))

    (func $f (local $locA i32) (local $locB i32)

        ;; mark locA as tainted
        call $source
        local.set $locA

        ;; put fresh, untainted value into locB
        i32.const 23
        local.set $locB

        ;; pass locB to sink
        local.get $locB
        call $sink

        ;; sanity check: should print 5
        local.get $locB
        call $print
    )

    (start $f)
)

;; Expected outcome: Taint does not reach sink.