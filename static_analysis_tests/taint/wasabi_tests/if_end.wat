(module
    (func $source (param i32))
    (export "taint_source" (func $source))
    (func $sink (param i32))
    (export "taint_sink" (func $sink))
    (global $g i32 i32.const 0)

    (func $main (local $0 i32)
        i32.const 69
        local.set $0
        local.get $0
        call $source

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

;; Expected outcome: Taint does not reach sink.