(module
    (import "imports" "global" (global $g i32))
    (table 2 funcref)
    (elem (i32.const 0) $bar $baz)
    (export "foo" (func $foo))
    (export "bar" (func $bar))
    (export "baz" (func $baz))
    (export "main" (func $main))

    (func $bar) 
    (func $baz
        call $bar)
    (func $foo (param $p i32)
        local.get $p
        call_indirect)
    (func $main
        global.get $g
        call $foo
        call $baz)
)