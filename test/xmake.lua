add_requires("gtest")

target("tiny-cobalt-unittests")
    set_kind("binary")
    add_deps("tiny-cobalt")
    add_files("**.cpp")
    add_packages("gtest")