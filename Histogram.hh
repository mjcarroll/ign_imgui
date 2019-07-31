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

#ifndef IGN_IMGUI__HISTOGRAM_HH_
#define IGN_IMGUI__HISTOGRAM_HH_

#include <cstdlib>

#include <mutex>
#include <string>
#include <vector>

#include <imgui/imgui.h>

namespace ign_imgui
{

class Histogram
{
  public: Histogram() = default;

  public: void SetNumBins(size_t _numBins);
  public: void SetRange(float _min, float _max);
  public: void InsertData(float _data);
  public: void Draw();
  public: void Reset();

  public: void PlotHistogram(const std::string &_label,
                             ImVec2 _graphSize=ImVec2(0,0));


  protected: void Update();

  protected: size_t numBins;
  protected: float minBin;
  protected: float maxBin;
  protected: float binStep;
  protected: std::vector<float> counts;
  protected: std::vector<float> bins;
  protected: std::mutex dataMutex;
};

}  // namespace ign_imgui

#endif  // IGN_IMGUI__HISTOGRAM_HH_

