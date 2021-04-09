target("Tiny-Msgpk")
    set_kind("static")
    add_files("src/*.c|main.c")
    add_includedirs("include")

target("Tiny-Msgpk")
    set_kind("binary")
    add_files("src/*.c")
    add_includedirs("include")