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

#ifndef TTSFLOW_VOCODER_VOCODER_WAVE_BASE_H_
#define TTSFLOW_VOCODER_VOCODER_WAVE_BASE_H_

#include <string>

struct WAVE_FILE_HEADER {
  char  szRIFF[4];
  int   iLength1;
  char  szWAVE[4];
  char  szFMT[4];
  int   iLength2;
  short nFormatTag;
  short nChannels;
  int   iSamplingRate;
  int   iAvgBytesPerSecond;
  short nBlockAlign;
  short nBitsPerSample;
  char  szDATA[4];
  int   iLength3;
};

static const WAVE_FILE_HEADER def_waveheader = {
  'R','I','F','F',
  0,
  'W','A','V','E',
  'f','m','t',' ',
  16,
  1,
  1,
  0,
  0,
  0,
  0,
  'd','a','t','a',
  0
};

struct MWAVEFORMAT {
  short nFormatTag ;
  short nChannels ;
  int   iSamplesPerSec ;
  int   iAvgBytesPerSec ;
  short nBlockAlign ;
};

struct MWAVEFORMATEX {
  short nFormatTag ;
  short nChannels ;
  int   iSamplesPerSec ;
  int   iAvgBytesPerSec ;
  short nBlockAlign ;
  short nBitsPerSample ;
  short ncbSize ;
};

#define MERGE4CHAR(ch0, ch1, ch2, ch3)  \
  (  (int)(char)(ch0)        \
  | ((int)(char)(ch1) << 8)  \
  | ((int)(char)(ch2) << 16) \
  | ((int)(char)(ch3) << 24 ))

class MWaveBase {
 public:
  MWaveBase();
  ~MWaveBase();

  bool ReadWaveFile( const char* filename ) ;

  int   m_iFileLength ;                        // total file length in bytes
  int   m_iSamplesNum ;                        // number of samples
  int   m_iTotalTime ;                         // total time in ms
  void* m_pvWaveFile ;                         // including wave header
  void* m_pvWaveData ;                         // pointer to data chunk only
  MWAVEFORMATEX* m_pWaveFormatEx ;
  WAVE_FILE_HEADER m_waveheader ;

 private:
  bool GetWaveData(void* pvWaveFile, MWAVEFORMATEX*& pWaveFormatEx, void*& pvWaveData, int& iDataSIze);
};

#endif  // TTSFLOW_VOCODER_VOCODER_WAVE_BASE_H_
