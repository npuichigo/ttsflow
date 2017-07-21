# Description:
#   TensorFlow C++ inference example for acoustic decoding.

package(
    default_visibility = ["//ttsflow:internal"],
)

licenses(["notice"])  # Apache 2.0

filegroup(
    name = "all_files",
    srcs = glob(
        ["**/*"],
        exclude = [
            "**/METADATA",
            "**/OWNERS",
        ],
    ),
)

cc_library(
    name = "sptk",
    srcs = glob(["lib/*.cc"]),
    hdrs = ["include/sptk.h"],
    visibility = [
        "//visibility:public",
    ],
)
