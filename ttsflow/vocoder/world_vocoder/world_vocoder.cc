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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <iostream>
#include <fstream>

#include <stdint.h>
#include <sys/time.h>

#include "world/d4c.h"
#include "world/dio.h"
#include "world/matlabfunctions.h"
#include "world/cheaptrick.h"
#include "world/stonemask.h"
#include "world/synthesis.h"
#include "world/synthesisrealtime.h"
#include "audioio.h"

#include "ttsflow/vocoder/sptk/include/sptk.h"

namespace ttsflow {

using tensorflow::Tensor;
using tensorflow::TensorShape;
using tensorflow::DT_FLOAT;
using Eigen::array;

WorldVocoder::WorldVocoder(int mgc_dim, int bap_dim, int f0_context_len,
                           int sample_rate, int fperiod)
    : IVocoder(sample_rate, fperiod),
      mgc_dim_(mgc_dim),
      bap_dim_(bap_dim),
      f0_context_len_(f0_context_len) {}

WorldVocoder::~WorldVocoder() = default;

void WorldVocoder::Generate(const Tensor& features,
                            const char* wav_file) {
  auto features_mat = features.matrix<float>();

  int frame_num = features_mat.dimension(0);

  Tensor mgc(DT_FLOAT, TensorShape{frame_num, mgc_dim_});
  Tensor mgc_repair(DT_FLOAT, TensorShape{frame_num, mgc_dim_});
  Tensor vuv(DT_FLOAT, TensorShape{frame_num, 1});
  Tensor lf0(DT_FLOAT, TensorShape{frame_num, 1});
  Tensor bap(DT_FLOAT, TensorShape{frame_num, bap_dim_});

  auto mgc_mat = mgc.matrix<float>();
  auto mgc_repair_mat = mgc_repair.matrix<float>();
  auto bap_mat = bap.matrix<float>();
  auto lf0_mat = lf0.matrix<float>();
  auto vuv_mat = vuv.matrix<float>();

  mgc_mat = features_mat.slice(array<int, 2>{0, 0},
                               array<int, 2>{frame_num, mgc_dim_});
  vuv_mat = features_mat.slice(array<int, 2>{0, mgc_dim_},
                               array<int, 2>{frame_num, 1});
  lf0_mat = features_mat.slice(array<int, 2>{0, mgc_dim_ + f0_context_len_ + 1},
                               array<int, 2>{frame_num, 1});
  bap_mat = features_mat.slice(array<int, 2>{0, mgc_dim_ + f0_context_len_ * 2 + 2},
                               array<int, 2>{frame_num, bap_dim_});

  for (int i = 0; i < vuv_mat.dimension(0); ++i) {
    if (i <= 1 || i >= vuv_mat.dimension(0) - 2) {
      for (int j = 0; j < mgc_mat.dimension(1); ++j) {
        mgc_repair_mat(i, j) = mgc_mat(i, j);
      }
    } else {
      for (int j = 0; j < mgc_mat.dimension(1); ++j) {
        mgc_repair_mat(i, j) = (mgc_mat(i - 1, j) + mgc_mat(i, j) + mgc_mat(i + 1, j)) / 3;
      }
    }
    if (vuv_mat(i, 0) < 0.5) {
      lf0_mat(i, 0) =  -1e10;
    }
  }

  std::unique_ptr<Tensor> f0, sp, ap;
  SptkSopr(lf0, &f0);
  SptkMgc2Sp(mgc_repair, &sp);
  SptkBap2Ap(bap, &ap);
  WorldSynthesis(*f0, *sp, *ap, wav_file);
}

void WorldVocoder::SptkSopr(const Tensor& lf0,
                            std::unique_ptr<Tensor>* f0) {
  auto lf0_mat = lf0.matrix<float>();

  f0->reset(new Tensor(DT_FLOAT, TensorShape{lf0_mat.dimension(0), 1}));
  auto f0_mat = (*f0)->matrix<float>();

  for (int i = 0; i < lf0_mat.dimension(0); ++i) {
    if (lf0_mat(i, 0) == -1e10) {
      f0_mat(i, 0) = 0;
    } else {
      f0_mat(i, 0) = exp(lf0_mat(i, 0));
    }
  }
}

void WorldVocoder::SptkMgc2Sp(const Tensor& mgc, std::unique_ptr<Tensor>* sp) {
  auto mgc_mat = mgc.matrix<float>();
  int m = mgc_mat.dimension(1) - 1, l = 1024;
  double alpha = 0.58, gamma = 0, *x, *y, *c;

  x = dgetmem(l + l + m + 1);
  y = x + l;
  c = y + l;

  int no = l / 2 + 1;

  sp->reset(new Tensor(DT_FLOAT, TensorShape{mgc_mat.dimension(0), no}));
  auto sp_mat = (*sp)->matrix<float>();

  for (int i = 0; i < mgc_mat.dimension(0); ++i) {
    for (int j = 0; j < mgc_mat.dimension(1); ++j) {
      c[j] = static_cast<double>(mgc_mat(i, j));
    }
    mgc2sp(c, m, alpha, gamma, x, y, l);
    for (int j = no; j--;) {
      x[j] = exp(x[j]) / 32768.0;
      x[j] *= x[j];
      sp_mat(i, j) = static_cast<float>(x[j]);
    }
  }
}

void WorldVocoder::SptkBap2Ap(const Tensor& bap, std::unique_ptr<Tensor>* ap) {
  auto bap_mat = bap.matrix<float>();
  int m = bap_mat.dimension(1) - 1, l = 1024;
  double alpha = 0.58, gamma = 0, *x, *y, *c;

  x = dgetmem(l + l + m + 1);
  y = x + l;
  c = y + l;

  int no = l / 2 + 1;

  ap->reset(new Tensor(DT_FLOAT, TensorShape{bap_mat.dimension(0), no}));
  auto ap_mat = (*ap)->matrix<float>();

  for (int i = 0; i < bap_mat.dimension(0); ++i) {
    for (int j = 0; j < bap_mat.dimension(1); ++j) {
      c[j] = static_cast<double>(bap_mat(i, j));
    }
    mgc2sp(c, m, alpha, gamma, x, y, l);
    for (int j = no; j--;) {
      x[j] = exp(x[j]) / 32768.0;
      x[j] *= x[j];
      ap_mat(i, j) = static_cast<float>(x[j]);
    }
  }
}

void WorldVocoder::WorldSynthesis(const Tensor& f0,
                                  const Tensor& sp,
                                  const Tensor& ap,
                                  const char* wav_file) {
  auto f0_mat = f0.matrix<float>();
  auto sp_mat = sp.matrix<float>();
  auto ap_mat = ap.matrix<float>();

  // Define a default filled structures
  WorldParameters world_parameters;
  world_parameters.fs = sample_rate_;
  world_parameters.frame_period = float(fperiod_ * 1000) / float(sample_rate_);
  world_parameters.f0_length = f0_mat.dimension(0);
  // Be careful that .sp contains only first half of the spectrum
  world_parameters.fft_size = (sp_mat.dimension(1) - 1 ) * 2;

  // Prepare memory
  world_parameters.f0 = new double[world_parameters.f0_length];

  world_parameters.spectrogram = new double*[world_parameters.f0_length];
  for (int i = 0; i < world_parameters.f0_length; i++)
    world_parameters.spectrogram[i] = new double[world_parameters.fft_size / 2 + 1];

  world_parameters.aperiodicity = new double*[world_parameters.f0_length];
  for (int i = 0; i < world_parameters.f0_length; i++)
    world_parameters.aperiodicity[i] = new double[world_parameters.fft_size / 2 + 1];

  // Loading
  // read the pitch data
  for (int i = 0; i < world_parameters.f0_length; i++) {
    world_parameters.f0[i] = static_cast<double>(f0_mat(i, 0));
  }

  // read the spectrogram data
  for (int i = 0; i < world_parameters.f0_length; i++) {
    for (int j = 0; j < world_parameters.fft_size / 2 + 1; j++) {
      world_parameters.spectrogram[i][j] = static_cast<double>(sp_mat(i, j));
    }
  }

  // read the aperiodicity data
  for (int i = 0; i < world_parameters.f0_length; i++) {
    for (int j = 0; j < world_parameters.fft_size / 2 + 1; j++) {
      world_parameters.aperiodicity[i][j] = static_cast<double>(ap_mat(i, j));
    }
  }

  // Synthesis
  int y_length = static_cast<int>((world_parameters.f0_length - 1) *
                                  world_parameters.frame_period / 1000.0 * world_parameters.fs) + 1;
  double *y = new double[y_length];
  for (int i = 0; i < y_length; ++i) y[i] = 0.0;
  WaveformSynthesis(&world_parameters, y);
  wavwrite(y, y_length, world_parameters.fs, 16, wav_file);

  // Cleaning part
  delete[] y;
  DestroyMemory(&world_parameters);
}

void WorldVocoder::WaveformSynthesis(WorldParameters *world_parameters, double *y) {
  int y_length = static_cast<int>((world_parameters->f0_length - 1) *
                                  world_parameters->frame_period / 1000.0 * world_parameters->fs) + 1;
  // Synthesis by the aperiodicity
  Synthesis(world_parameters->f0, world_parameters->f0_length,
            world_parameters->spectrogram, world_parameters->aperiodicity,
            world_parameters->fft_size, world_parameters->frame_period,
            world_parameters->fs, y_length, y);
}

void WorldVocoder::DestroyMemory(WorldParameters *world_parameters) {
  delete[] world_parameters->f0;
  for (int i = 0; i < world_parameters->f0_length; ++i) {
    delete[] world_parameters->spectrogram[i];
    delete[] world_parameters->aperiodicity[i];
  }
  delete[] world_parameters->spectrogram;
  delete[] world_parameters->aperiodicity;
}

}  // namespace ttsflow
