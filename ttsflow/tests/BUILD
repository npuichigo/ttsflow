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

cc_binary(
    name = "feat_trans_test",
    srcs = [
        "feat_trans_test.cc",
    ],
    linkopts = ["-lm"],
    deps = [
        "//ttsflow/utils:feat_trans",
        "@org_tensorflow//tensorflow/core:tensorflow",
    ],
)

cc_binary(
    name = "world_vocoder_test",
    srcs = [
        "world_vocoder_test.cc",
    ],
    linkopts = ["-lm"],
    deps = [
        "//ttsflow/vocoder:world_vocoder",
        "@org_tensorflow//tensorflow/core:tensorflow",
    ],
)
