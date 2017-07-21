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

#ifndef TTSFLOW_VOCODER_WORLD_VOCODER_WORLD_VOCODER_H_
#define TTSFLOW_VOCODER_WORLD_VOCODER_WORLD_VOCODER_H_

#include "ttsflow/vocoder/vocoder_interface.h"

#include "tensorflow/core/framework/tensor.h"

namespace ttsflow {

class WorldVocoder : public IVocoder {
 public:
  WorldVocoder(int mgc_dim, int bap_dim, int f0_context_len,
               int sample_rate = 16000, int fperiod = 80);
  ~WorldVocoder();

  void Generate(const tensorflow::Tensor& features, const char* wav_file);

 private:
  typedef struct {
    double frame_period;
    int fs;

    double *f0;
    double *time_axis;
    int f0_length;

    double **spectrogram;
    double **aperiodicity;
    int fft_size;
  } WorldParameters;

  void SptkSopr(const tensorflow::Tensor& lf0,
                std::unique_ptr<tensorflow::Tensor>* f0);
  void SptkMgc2Sp(const tensorflow::Tensor& mgc,
                  std::unique_ptr<tensorflow::Tensor>* sp);
  void SptkBap2Ap(const tensorflow::Tensor& bap,
                  std::unique_ptr<tensorflow::Tensor>* ap);

  void WaveformSynthesis(WorldParameters *world_parameters, double *y);
  void DestroyMemory(WorldParameters *world_parameters);
  void WorldSynthesis(const tensorflow::Tensor& f0,
                      const tensorflow::Tensor& sp,
                      const tensorflow::Tensor& ap,
                      const char* wav_file);

  int mgc_dim_;
  int bap_dim_;
  int f0_context_len_;
};

}  // namespace ttsflow

#endif  // TTSFLOW_VOCODER_WORLD_VOCODER_WORLD_VOCODER_H_
