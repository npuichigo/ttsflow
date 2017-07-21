# ttsflow external dependencies that can be loaded in WORKSPACE
# files.

load('@org_tensorflow//tensorflow:workspace.bzl', 'tf_workspace')

# All ttsflow external dependencies.
# workspace_dir is the absolute path to the ttsflow repo. If linked
# as a submodule, it'll likely be '__workspace_dir__ + "/ttsflow"'
def ttsflow_workspace():
  tf_workspace(path_prefix = "", tf_repo_name = "org_tensorflow")
