/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "Histogram.hh"

#include <algorithm>

namespace ign_imgui
{

//////////////////////////////////////////////////
void Histogram::SetNumBins(size_t _numBins)
{
  this->numBins = _numBins;
  this->Update();
}

//////////////////////////////////////////////////
void Histogram::SetRange(float _min, float _max)
{
  this->minBin = _min;
  this->maxBin = _max;
  this->Update();
}

//////////////////////////////////////////////////
void Histogram::InsertData(float _data)
{
  std::lock_guard<std::mutex> lock(this->dataMutex);
  for (size_t ii = 0; ii < this->numBins; ++ii)
  {
    if (_data <= this->bins[ii])
    {
      this->counts[ii - 1] += 1;
      break;
    }
  }
}

//////////////////////////////////////////////////
void Histogram::Reset()
{
  this->Update();
}

//////////////////////////////////////////////////
void Histogram::Update()
{
  std::lock_guard<std::mutex> lock(this->dataMutex);
  this->binStep = (this->maxBin - this->minBin) / this->numBins;
  this->counts = std::vector<float>(this->numBins, 0);
  this->bins = std::vector<float>(this->numBins, 0.0);
  for (size_t ii = 0; ii < this->numBins; ++ii)
  {
    this->bins[ii] = ii * this->binStep;
  }
}

//////////////////////////////////////////////////
void Histogram::PlotHistogram(const std::string &_label, ImVec2 _graphSize)
{
  std::lock_guard<std::mutex> lock(this->dataMutex);
  auto maxCount = *std::max_element(this->counts.begin(), this->counts.end());
  auto minCount = *std::min_element(this->counts.begin(), this->counts.end());

  ImGui::PlotHistogram(_label.c_str(),
                       &this->counts[0],
                       this->counts.size(),
                       0,
                       NULL,
                       minCount,
                       maxCount,
                       _graphSize);
}


}  // namespace ign_imgui

