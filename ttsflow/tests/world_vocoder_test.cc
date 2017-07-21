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

#include "ttsflow/vocoder/world_vocoder/world_vocoder.h"

#include <fstream>
#include <iostream>
#include <string>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"

using tensorflow::Tensor;
using tensorflow::TensorShape;
using tensorflow::DT_FLOAT;

using namespace ttsflow;

std::ifstream::pos_type filesize(const char* filename) {
  std::ifstream in(filename, std::ifstream::binary | std::ifstream::ate);
  return in.tellg();
}

int main(int argc, char* argv[]) {
  std::string filename = "ttsflow/tests/test.cmp";
  int feature_dim = 75;
  int num_frames = filesize(filename.c_str()) / (sizeof(float) * feature_dim);
  LOG(INFO) << "num_frames: " << num_frames;

  float **cmp = new float*[num_frames];
  for (int i = 0; i < num_frames; i++)
    cmp[i] = new float[feature_dim];

  std::ifstream is_cmp(filename.c_str(), std::ios::binary | std::ios::in);
  if (!is_cmp.is_open()) {
    LOG(ERROR) << "Can't open file";
    return -1;
  }

  for (int i = 0; i < num_frames; i++) {
    is_cmp.read(reinterpret_cast<char*>(cmp[i]),
                std::streamsize(feature_dim * sizeof(float)));
  }

  Tensor features(DT_FLOAT, TensorShape({num_frames, feature_dim}));
  auto features_mat = features.matrix<float>();
  for (int i = 0; i < num_frames; i++) {
    for (int j = 0; j < feature_dim; j++) {
      features_mat(i, j) = cmp[i][j];
    }
  }

  std::unique_ptr<IVocoder> vocoder(new WorldVocoder(60, 5, 4));
  vocoder->Generate(features, "ttsflow/tests/test.wav");

  return 0;
}
