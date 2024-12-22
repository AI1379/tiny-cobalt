add_requires(
    "microsoft-proxy 3.1.0"
)
add_requires(
    "cmake::LLVM",
    {
        alias = "llvm-dev",
        system = true
    }
)

target("tiny-cobalt-library")
    add_defines("TINYCOBALT_LIBRARY")
    set_kind("static")
    add_files("**.cpp")
    add_includedirs("$(projectdir)/include", { public = true })
    add_headerfiles("**.h")
    add_packages(
        "llvm-dev",
        "microsoft-proxy",
        {
            public = true
        }
    )

target("tiny-cobalt")
    set_kind("binary")
    add_deps("tiny-cobalt-library")
    add_files("Driver/Driver.cpp")