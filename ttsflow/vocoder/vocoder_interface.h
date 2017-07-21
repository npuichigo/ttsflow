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

#ifndef TTSFLOW_VOCODER_VOCODER_INTERFACE_H_
#define TTSFLOW_VOCODER_VOCODER_INTERFACE_H_

#include <cstdlib>
#include <string>

//#include "ttsflow/vocoder/wave_base.h"
#include "tensorflow/core/framework/tensor.h"

namespace ttsflow {

class IVocoder {
 public:
  IVocoder(int sample_rate, int fperiod) : sample_rate_(sample_rate), fperiod_(fperiod) {}
  virtual void Generate(const tensorflow::Tensor& feats, const char* wav_file) = 0;
  //std::string WriteWaveHeader(int sample_number);

 protected:
  int sample_rate_;
  int fperiod_;
};
/*
std::string IVocoder::WriteWaveHeader(int sample_number, int sample_rate); {
  MWaveBase* pwave = new MWaveBase;
  pwave->m_waveheader.iSamplingRate = sample_rate_;
  pwave->m_waveheader.iAvgBytesPerSecond = 2 * sample_rate_;
  pwave->m_waveheader.nBlockAlign = 2;
  pwave->m_waveheader.nBitsPerSample = 16;
  pwave->m_waveheader.iLength3 = sample_number * 2;
  pwave->m_waveheader.iLength1 = sample_number * 2 + 36;

  //write wave header to string
  std::string wave_header;
  char *bytes = reinterpret_cast<char*>(&(pwave->m_waveheader));
  std::size_t size = sizeof(WAVE_FILE_HEADER) / sizeof(char);
  for(std::size_t i = 0; i < size; ++i) {
    wave_header += bytes[i];
  }
  delete pwave;
  return wave_header;
}*/

}  // namespace ttsflow

#endif  // #define TTSFLOW_VOCODER_VOCODER_INTERFACE_H_
