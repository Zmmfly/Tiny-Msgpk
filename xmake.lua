target("Tiny-Msgpk")
    set_kind("static")
    add_files("src/*.c|test.c")
    add_includedirs("include")

target("Tiny-Msgpk_Test")
    set_kind("binary")
    add_files("src/*.c")
    add_includedirs("include")