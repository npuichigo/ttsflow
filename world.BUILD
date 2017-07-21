cc_library(
    name = "world",
    srcs = glob([
        "tools/*.cpp",
        "src/*.cpp",
    ]),
    hdrs = glob([
        "tools/*.h",
        "src/world/*.h",
    ]),
    copts = [
        "-Wno-unused-result",
    ],
    includes = [
      "src",
      "tools",
    ],
    visibility = [
        "//visibility:public",
    ],
)
