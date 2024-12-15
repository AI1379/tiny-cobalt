add_requires("gtest")

target("tiny-cobalt-test")
    set_kind("binary")
    add_deps("tiny-cobalt-library")
    add_files("**.cpp")
    add_packages("gtest")