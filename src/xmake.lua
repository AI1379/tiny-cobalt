add_requires(
    "microsoft-proxy 3.1.0",
    "nlohmann_json",
    "magic_enum",
    "range-v3"
)
add_requires(
    "cmake::LLVM",
    {
        alias = "llvm-dev",
        system = true
    }
)

target("tiny-cobalt-library")
    set_kind("static")
    add_rules("lex", "yacc")
    add_cxxflags("clang::-fsized-deallocation")
    add_defines("TINYCOBALT_LIBRARY")
    add_defines("TINYCOBALT_ENABLE_JSON")
    add_files("**.cpp", "**.yy", "**.ll")
    add_includedirs("$(projectdir)/include", { public = true })
    add_includedirs("$(projectdir)/src", { public = false })
    add_headerfiles("**.h")
    add_packages(
        "llvm-dev",
        "nlohmann_json",
        "magic_enum",
        "microsoft-proxy",
        "range-v3",
        {
            public = true
        }
    )

target("tiny-cobalt")
    set_kind("binary")
    add_deps("tiny-cobalt-library")
    add_files("Driver/Driver.cpp")