(module
    (func $source (result i32) i32.const 69)
    (export "taint_source" (func $source))
    (func $sink (param i32))
    (export "taint_sink" (func $sink))
    (global $g i32 i32.const 0)

    (func $main (local $0 i32)
        call $source
        local.set $0

        global.get $g
        i32.eqz
        if
            i32.const 32
            local.set $0
        end

        local.get $0
        call $sink
    )

    (start $main)
)

;; Expected outcome: Taint reaches sink.