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

#include "ttsflow/utils/feat_trans.h"

#include <iostream>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"

using namespace ttsflow;
using tensorflow::Tensor;
using tensorflow::TensorShape;
using tensorflow::DT_FLOAT;

int main(int argc, char* argv[]) {
  Tensor phone_features(DT_FLOAT, TensorShape({2, 3}));
  phone_features.matrix<float>().setValues({{1, 2, 3}, {4, 5, 6}});

  Tensor phone_durations(DT_FLOAT, TensorShape({2, 5}));
  phone_durations.matrix<float>().setValues({{1, 1, 6, 8, 1}, {1, 6, 8, 1, 1}});

  Tensor state_durations(DT_FLOAT, TensorShape({2, 5}));
  state_durations.matrix<float>().setValues({{1, 1, 2, 1, 1}, {1, 2, 1, 1, 3}});

  std::unique_ptr<Tensor> frame_features_p;
  std::unique_ptr<Tensor> frame_features_s;

  AddPhoneDurationInfo(phone_features,
                       phone_durations,
                       &frame_features_p);
  LOG(INFO) << "Add phone duration info";
  std::cout << frame_features_p->matrix<float>() << std::endl;

  AddStateDurationInfo(phone_features,
                       state_durations,
                       &frame_features_s);
  LOG(INFO) << "Add state duration info";
  std::cout << frame_features_s->matrix<float>() << std::endl;

  return 0;
}
