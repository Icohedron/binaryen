(module
    (func $source (param i32))
    (export "taint_source" (func $source))
    (func $sink (param i32))
    (export "taint_sink" (func $sink))
    (global $global i32 i32.const 0)

    (func $main (local $local i32)
        global.get $global
        i32.eqz
        if 
            i32.const 69
            local.set $local
            local.get $local
            call $source
        else
            i32.const 24
            local.set $local
        end
        local.get $local
        call $sink
    )

    (start $main)
)

;; Expected outcome: Taint reaches sink.