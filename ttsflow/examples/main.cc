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

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/command_line_flags.h"

#include "ttsflow/backend/tf_model.h"
#include "ttsflow/utils/feat_trans.h"
#include "ttsflow/vocoder/world_vocoder/world_vocoder.h"

// These are all common classes it's handy to reference with no namespace.
using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::TensorShape;
using tensorflow::string;
using tensorflow::DT_FLOAT;

using namespace ttsflow;

int main(int argc, char* argv[]) {
  // These are the command-line flags the program can understand.
  // They define where the graph and input data is located, and what kind of
  // input the model expects.
  string duration_graph = "ttsflow/model/frozen_duration.pb";
  string acoustic_graph = "ttsflow/model/frozen_acoustic.pb";
  string input_layer = "input";
  string output_layer = "output";
  string root_dir = "";
  string label_path = "ttsflow/tests/test.lab";
  string cmp_path = "ttsflow/tests/test.cmp";
  int mgc_dim = 60;
  int bap_dim = 5;
  int f0_context = 4;
  std::vector<Flag> flag_list = {
      Flag("duration_graph", &duration_graph, "duration graph to be executed"),
      Flag("acoustic_graph", &acoustic_graph, "acoustic graph to be executed"),
      Flag("input_layer", &input_layer, "name of input layer"),
      Flag("output_layer", &output_layer, "name of output layer"),
      Flag("root_dir", &root_dir,
           "interpret label and graph file names relative to this directory"),
      Flag("label_path", &label_path, "path of label with text format"),
      Flag("cmp_path", &cmp_path, "path of cmp to write"),
      Flag("mgc_dim", &mgc_dim, "dimension of mgc feature"),
      Flag("bap_dim", &bap_dim, "dimension of bap feature"),
      Flag("f0_context", &f0_context, "length of f0 context"),
  };
  string usage = tensorflow::Flags::Usage(argv[0], flag_list);
  const bool parse_result = tensorflow::Flags::Parse(&argc, argv, flag_list);
  if (!parse_result) {
    LOG(ERROR) << usage;
    return -1;
  }

  // We need to call this to set up global state for TensorFlow.
  tensorflow::port::InitMain(argv[0], &argc, &argv);
  if (argc > 1) {
    LOG(ERROR) << "Unknown argument " << argv[1] << "\n" << usage;
    return -1;
  }

  TfModel duration_model;
  // Load and initialize the duration model.
  string graph_path = tensorflow::io::JoinPath(root_dir, duration_graph);
  bool load_graph_status = duration_model.LoadGraph(graph_path);
  if (!load_graph_status) {
    return -1;
  }

  TfModel acoustic_model;
  // Load and initialize the acoustic model.
  graph_path = tensorflow::io::JoinPath(root_dir, acoustic_graph);
  load_graph_status = acoustic_model.LoadGraph(graph_path);
  if (!load_graph_status) {
    return -1;
  }

  std::ifstream label(label_path.c_str());
  std::vector<string> label_vec;
  string line;
  while(getline(label, line)) {
    label_vec.push_back(line);
  }

  int num_phones = label_vec.size();

  Tensor duration_input(DT_FLOAT, TensorShape({num_phones, 136}));
  auto duration_input_matrix = duration_input.matrix<float>();
  for (int i = 0; i != num_phones; ++i) {
    int j = 0;
    std::stringstream ssline(label_vec[i]);
    float value;
    while (ssline >> value) {
      duration_input_matrix(i, j) = value;
      ++j;
    }
  }

  // Duration decode
  LOG(INFO) << "Duration decode";
  std::vector<Tensor> duration_outputs;
  bool run_status = duration_model.Eval({{input_layer, duration_input}},
                                        {output_layer}, &duration_outputs);
  if (!run_status) {
    return -1;
  }
  Tensor& duration_output = duration_outputs[0];

  std::unique_ptr<Tensor> acoustic_input;
  AddStateDurationInfo(duration_input, duration_output, &acoustic_input);

  // Acoustic decode
  LOG(INFO) << "Acoustic decode";
  std::vector<Tensor> acoustic_outputs;
  run_status = acoustic_model.Eval({{input_layer, *acoustic_input}},
                                   {output_layer}, &acoustic_outputs);
  if (!run_status) {
    return -1;
  }
  const Tensor& acoustic_output = acoustic_outputs[0];

  std::ofstream out_cmp(cmp_path.c_str(), std::ios::out | std::ios::binary);
  out_cmp.write(reinterpret_cast<const char*>(acoustic_output.tensor_data().data()),
                std::streamsize(acoustic_output.NumElements() * sizeof(float)));

  LOG(INFO) << "Vocoder synthesize";
  std::unique_ptr<IVocoder> vocoder(new WorldVocoder(mgc_dim, bap_dim, f0_context));
  vocoder->Generate(acoustic_output, "ttsflow/tests/test.wav");

  return 0;
}
