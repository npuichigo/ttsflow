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

#ifndef TTSFLOW_BACKEND_TF_MODEL_H_
#define TTSFLOW_BACKEND_TF_MODEL_H_

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

namespace ttsflow {

class TfModel {
 public:
  virtual ~TfModel();
  virtual bool LoadGraph(const std::string& graph_file_name);
  bool Eval(const std::vector<std::pair<std::string, tensorflow::Tensor> >& inputs,
            const std::vector<std::string>& output_tensor_names,
            std::vector<tensorflow::Tensor>* outputs);

 protected:
  std::unique_ptr<tensorflow::Session> session_;
};

}  // namespace ttsflow

#endif  // TTSFLOW_BACKEND_TF_MODEL_H_
