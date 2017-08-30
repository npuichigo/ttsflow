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

#include <vector>

#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"

namespace ttsflow {

using tensorflow::Tensor;
using tensorflow::TensorShape;
using tensorflow::DT_FLOAT;

void AddPhoneDurationInfo(const Tensor& phone_features,
                          const Tensor& phone_durations,
                          std::unique_ptr<Tensor>* frame_features) {
  auto phone_features_mat = phone_features.matrix<float>();
  auto phone_durations_mat = phone_durations.matrix<float>();

  int num_phones = phone_features_mat.dimension(0);
  int phone_dim = phone_features_mat.dimension(1);
  int offset = 0;

  int total_frames = 0;
  for(int i = 0; i < num_phones; ++i) {
    int duration = static_cast<int>(phone_durations_mat(i, 2) + 0.5);
    if (duration <= 0)
      duration = 1;
    total_frames += duration;
  }

  LOG(INFO) << "Total frames: " << total_frames;

  // Construct the output tensor
  frame_features->reset(new Tensor(DT_FLOAT, TensorShape{total_frames, phone_dim + 3}));
  auto frame_features_mat = (*frame_features)->matrix<float>();

  // Fill frame_features without duration information
  for (int i = 0; i < num_phones; ++i) {
    int duration = static_cast<int>(phone_durations_mat(i, 2) + 0.5);
    if (duration <= 0)
      duration = 1;
    Eigen::array<int, 2> offsets_lhs = {offset, 0}, extents_lhs = {duration, phone_dim};
    Eigen::array<int, 2> offsets_rhs = {i, 0}, extents_rhs{1, phone_dim};
    Eigen::array<int, 2> bcast({duration, 1});
    auto phone_feature = phone_features_mat.slice(offsets_rhs, extents_rhs);
    frame_features_mat.slice(offsets_lhs, extents_lhs) = phone_feature.broadcast(bcast);

    // Add duration information to frame_features
    for (int j = 0; j < duration; ++j) {
      frame_features_mat(offset, phone_dim) = j + 1;
      frame_features_mat(offset, phone_dim + 1) = duration;
      frame_features_mat(offset, phone_dim + 2) = duration - j;
      ++offset;
    }
  }
}

void AddStateDurationInfo(const Tensor& phone_features,
                          const Tensor& state_durations,
                          std::unique_ptr<Tensor>* frame_features) {
  auto phone_features_mat = phone_features.matrix<float>();
  auto state_durations_mat = state_durations.matrix<float>();

  int num_phones = phone_features_mat.dimension(0);
  int phone_dim = phone_features_mat.dimension(1);
  int num_states = state_durations_mat.dimension(1);
  int offset = 0;

  //calculate phone duration
  std::vector<int> phone_durations(num_phones, 0);
  int total_frames = 0;
  for(int i = 0; i < num_phones; ++i) {
    for(int j = 0; j < 5; ++j) {
      int state_duration = static_cast<int>(state_durations_mat(i, j) + 0.5);
      if (state_duration < 0)
        state_duration = 0;
      phone_durations[i] += state_duration;
      total_frames += state_duration;
    }
  }

  LOG(INFO) << "Total frames: " << total_frames;

  // Construct the output tensor
  frame_features->reset(new Tensor(DT_FLOAT, TensorShape{total_frames, phone_dim + 9}));
  auto frame_features_mat = (*frame_features)->matrix<float>();

  // Fill frame_features without duration information
  for (int i = 0; i < num_phones; ++i) {
    int phone_duration = phone_durations[i];
    Eigen::array<int, 2> offsets_lhs = {offset, 0}, extents_lhs{phone_duration, phone_dim};
    Eigen::array<int, 2> offsets_rhs = {i, 0}, extents_rhs{1, phone_dim};
    Eigen::array<int, 2> bcast({phone_duration, 1});
    auto phone_feature = phone_features_mat.slice(offsets_rhs, extents_rhs);
    frame_features_mat.slice(offsets_lhs, extents_lhs) = phone_feature.broadcast(bcast);

    int accumulated_state_duration = 0;
    // Add duration information to frame_features
    for (int j = 0; j < num_states; ++j) {
      int state_duration = static_cast<int>(state_durations_mat(i, j) + 0.5);
      if (state_duration < 0)
        state_duration = 0;
      for (int k = 0; k < state_duration; ++k) {
        frame_features_mat(offset, phone_dim) = float(k + 1) / float(state_duration);
        frame_features_mat(offset, phone_dim + 1) = float(state_duration - k) / float(state_duration);
        frame_features_mat(offset, phone_dim + 2) = float(state_duration);
        frame_features_mat(offset, phone_dim + 3) = float(j + 1);
        frame_features_mat(offset, phone_dim + 4) = float(num_states - j);
        frame_features_mat(offset, phone_dim + 5) = float(phone_duration);
        frame_features_mat(offset, phone_dim + 6) = float(state_duration) / float(phone_duration);
        frame_features_mat(offset, phone_dim + 7) = float(phone_duration - k - accumulated_state_duration) / float(phone_duration);
        frame_features_mat(offset, phone_dim + 8) = float(accumulated_state_duration + k + 1) / float(phone_duration);
        ++offset;
      }
      accumulated_state_duration += state_duration;
    }
  }
}

}  // namespace ttsflow
