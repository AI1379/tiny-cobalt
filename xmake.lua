set_project("tiny-cobalt")

set_version("1.0.0")

add_rules("mode.debug", "mode.release")

set_languages("c89", "cxx20")

includes("src")
includes("test")
