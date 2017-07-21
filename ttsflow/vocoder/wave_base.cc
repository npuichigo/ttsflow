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

#include "ttsflow/vocoder/wave_base.h"

#include <stdio.h>
#include <assert.h>
#include <memory.h>

MWaveBase::MWaveBase()
    : m_iSamplesNum(0),
      m_iTotalTime(0),
      m_iFileLength(0),
      m_pvWaveFile(NULL),
      m_pvWaveData(NULL),
      m_pWaveFormatEx(NULL),
      m_waveheader(def_waveheader) {}

MWaveBase::~MWaveBase() {
  if (m_pvWaveFile != NULL) {
    delete[] static_cast<char*>(m_pvWaveFile);
    m_pvWaveFile = NULL;
    m_pvWaveData = NULL;
    m_pWaveFormatEx = NULL;
  }
}

bool MWaveBase::ReadWaveFile(const char* pszFileName) {
  assert(pszFileName != NULL);

  FILE * fp = NULL;
  if ((fp = fopen(pszFileName, "rb")) == NULL) {
    return false;
  }
  fseek(fp, 0, SEEK_END);
  m_iFileLength = ftell(fp);
  m_pvWaveFile = static_cast<void*>(new char[m_iFileLength]);
  fseek(fp, 0, SEEK_SET);
  fread(m_pvWaveFile, sizeof(char), m_iFileLength, fp);
  fclose(fp);

  int iDataSize = 0;
  GetWaveData(m_pvWaveFile, m_pWaveFormatEx, m_pvWaveData, iDataSize);

  m_waveheader.iAvgBytesPerSecond = m_pWaveFormatEx->iAvgBytesPerSec;
  m_waveheader.iSamplingRate = m_pWaveFormatEx->iSamplesPerSec;
  m_waveheader.nBitsPerSample = m_pWaveFormatEx->nBitsPerSample;
  m_waveheader.nBlockAlign = m_pWaveFormatEx->nBlockAlign;
  m_waveheader.nChannels = m_pWaveFormatEx->nChannels;
  m_waveheader.nFormatTag = m_pWaveFormatEx->nFormatTag;
  m_waveheader.iLength3 = iDataSize;
  m_waveheader.iLength1 = 0;

  m_iSamplesNum = iDataSize / m_pWaveFormatEx->nBlockAlign;
  m_iTotalTime = static_cast<int>(iDataSize * 1000.0f / m_pWaveFormatEx->iAvgBytesPerSec);
  return true;
}

bool MWaveBase::GetWaveData(void* pvWaveFile, MWAVEFORMATEX*& pWaveFormatEx,
                            void*& pvWaveData, int& iDataSize) {
  assert(pvWaveFile != NULL);

  pWaveFormatEx = NULL;
  pvWaveData    = NULL;
  iDataSize     = 0;

  int* piWaveFile = static_cast<int*>(pvWaveFile);
  int  iRiff      = *piWaveFile++;
  int  iLength    = *piWaveFile++;
  int  iType      = *piWaveFile++;

  if (iRiff != MERGE4CHAR('R', 'I', 'F', 'F')) {
    return false;     // not even RIFF
  }

  if(iType != MERGE4CHAR('W', 'A', 'V', 'E')) {
    return false;      // not a WAV
  }

  int* piWaveFileEnd = reinterpret_cast<int*>(static_cast<char*>(pvWaveFile) + iLength + 8);

  while(piWaveFile < piWaveFileEnd) {
    iType   = *piWaveFile++;
    iLength = *piWaveFile++;

    switch(iType) {
      case MERGE4CHAR('f', 'm', 't', ' '):
        if(!pWaveFormatEx) {
          if(iLength < sizeof(MWAVEFORMAT)) {
            return false;     // not a WAV
          }
          pWaveFormatEx = reinterpret_cast<MWAVEFORMATEX*>(piWaveFile);
          if(pvWaveData && iDataSize) {
            return true;
          }
        }
        break;

      case MERGE4CHAR('d', 'a', 't', 'a'):
        pvWaveData = static_cast<void*>(piWaveFile);
        iDataSize  = iLength;
        if(pWaveFormatEx) {
          return true;
        }
        break;
      }
    int iOffset = (iLength + 1) & ~1;  // trick : ensure to be a even number
    piWaveFile = reinterpret_cast<int*>( reinterpret_cast<char*>(piWaveFile) + iOffset);
  }
  return true;
}
