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
    name = "feat_trans",
    srcs = ["feat_trans.cc"],
    hdrs = ["feat_trans.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "@org_tensorflow//tensorflow/core:tensorflow",
    ],
)
