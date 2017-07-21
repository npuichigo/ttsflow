// Copyright 2016 ASLP@NPU.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: npuichigo@gmail.com (zhangyuchao)

#include "ttsflow/backend/tf_model.h"

#include <cstdlib>
#include <fstream>
#include <utility>
#include <vector>

#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"

// These are all common classes it's handy to reference with no namespace.
using tensorflow::Tensor;
using tensorflow::TensorShape;
using tensorflow::Status;
using tensorflow::string;

namespace ttsflow {

TfModel::~TfModel() = default;

// Reads a model graph definition from disk, and creates a session object you
// can use to run it.
bool TfModel::LoadGraph(const string& graph_file_name) {
  tensorflow::GraphDef graph_def;
  Status load_graph_status =
      ReadBinaryProto(tensorflow::Env::Default(), graph_file_name, &graph_def);
  if (!load_graph_status.ok()) {
    LOG(ERROR) <<  "Failed to load compute graph at '" << graph_file_name << "'";
    return false;
  }

  session_.reset(tensorflow::NewSession(tensorflow::SessionOptions()));
  Status session_create_status = session_->Create(graph_def);
  if (!session_create_status.ok()) {
    LOG(ERROR) << "Filed to create tensorflow graph: " << session_create_status;
    return false;
  }
  return true;
}

bool TfModel::Eval(const std::vector<std::pair<string, Tensor> >& inputs,
                   const std::vector<string>& output_tensor_names,
                   std::vector<Tensor>* outputs) {
  Status run_status = session_->Run(inputs, output_tensor_names, {}, outputs);
  if (!run_status.ok()) {
    LOG(ERROR) << "Running model failed: " << run_status;
    return false;
  }
  return true;
}

}  // namespace ttsflow
