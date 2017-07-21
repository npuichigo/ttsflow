# Description:
#   TensorFlow C++ inference example for speech synthesis.

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
    name = "world_vocoder",
    srcs = ["world_vocoder/world_vocoder.cc"],
    hdrs = [
        "vocoder_interface.h",
        "world_vocoder/world_vocoder.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//ttsflow/vocoder/sptk:sptk",
        "@world_vocoder//:world",
        "@org_tensorflow//tensorflow/core:tensorflow",
    ],
    alwayslink=1,
)
