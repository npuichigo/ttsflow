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

cc_binary(
    name = "ttsflow_main",
    srcs = [
        "main.cc",
    ],
    linkopts = ["-lm"],
    deps = [
        "//ttsflow/backend:tf_model",
        "//ttsflow/utils:feat_trans",
        "//ttsflow/vocoder:world_vocoder",
        "@org_tensorflow//tensorflow/core:tensorflow",
    ],
)
