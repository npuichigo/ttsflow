workspace(name = "ttsflow")

# Use world vocoder to synthesize.
new_local_repository(
    name = "world_vocoder",
    path = "third_party/World",
    build_file = "world.BUILD",
)

local_repository(
    name = "org_tensorflow",
    path = "third_party/tensorflow",
)

# TensorFlow depends on "io_bazel_rules_closure" so we need this here.
# Needs to be kept in sync with the same target in TensorFlow's WORKSPACE file.
http_archive(
    name = "io_bazel_rules_closure",
    sha256 = "4be8a887f6f38f883236e77bb25c2da10d506f2bf1a8e5d785c0f35574c74ca4",
    strip_prefix = "rules_closure-aac19edc557aec9b603cd7ffe359401264ceff0d",
    urls = [
        "http://mirror.bazel.build/github.com/bazelbuild/rules_closure/archive/aac19edc557aec9b603cd7ffe359401264ceff0d.tar.gz",  # 2017-05-10
        "https://github.com/bazelbuild/rules_closure/archive/aac19edc557aec9b603cd7ffe359401264ceff0d.tar.gz",
    ],
)

# Please add all new ttsflow dependencies in workspace.bzl.
load('//ttsflow:workspace.bzl', 'ttsflow_workspace')
ttsflow_workspace()

# Specify the minimum required bazel version.
load("@org_tensorflow//tensorflow:workspace.bzl", "check_version")
check_version("0.4.5")
