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


#include <ignition/msgs.hh>
#include <ignition/common/Console.hh>
#include <ignition/common/Time.hh>
#include <ignition/math/SignalStats.hh>
#include <ignition/transport/Node.hh>

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>

#include "Histogram.hh"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace ignition;

const size_t kDefaultHistBins = 100;
const float kDefaultHistMin = 0.0f;
const float kDefaultHistMax = 1.1f;

const float kDefaultRTFMin = 0.0f;
const float kDefaultRTFMax = 1.1f;

//////////////////////////////////////////////////
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

//////////////////////////////////////////////////
int main(int _argc, char** _argv)
{
  // Initialize OpenGL
  glfwSetErrorCallback(glfw_error_callback);
  if(!glfwInit())
    return 1;

  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(400, 400, "ign_imgui", NULL, NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync
  bool err = glewInit() != 0;

  // Set verbosity
  ignition::common::Console::SetVerbosity(4);
  ignition::transport::Node node;

  std::mutex rtfsMutex;
  ignition::msgs::Clock msg_z;
  bool first = true;

  bool animate = true;

  std::vector<float> rtfs;

  ignition::math::SignalStats stats;
  stats.InsertStatistic("max");
  stats.InsertStatistic("min");
  stats.InsertStatistic("mean");
  stats.InsertStatistic("var");

  ign_imgui::Histogram hist;

  hist.SetNumBins(100);
  hist.SetRange(0.0f, 1.1f);

  std::function<void(const ignition::msgs::Clock&)> cb =
    [&](const ignition::msgs::Clock &_msg)
    {
      std::lock_guard<std::mutex> lock(rtfsMutex);

      if (first)
      {
        msg_z = _msg;
        first = false;
        return;
      }

      ignition::common::Time real_z(msg_z.real().sec(), msg_z.real().nsec());
      ignition::common::Time real(_msg.real().sec(), _msg.real().nsec());
      ignition::common::Time sim_z(msg_z.sim().sec(), msg_z.sim().nsec());
      ignition::common::Time sim(_msg.sim().sec(), _msg.sim().nsec());

      auto real_dt = (real - real_z);
      auto sim_dt = (sim - sim_z);
      auto rtf = sim_dt.Double() / real_dt.Double();

      msg_z = _msg;

      if (animate)
      {
        stats.InsertData(rtf);
        hist.InsertData(rtf);

        if (rtfs.size() > 250)
        {
          for(size_t ii = 1; ii < rtfs.size(); ++ii)
          {
            rtfs[ii-1] = rtfs[ii];
          }
          rtfs[rtfs.size() - 1] = rtf;
        }
        else
        {
          rtfs.push_back(rtf);
        }
      }

    };

  double progress = 0;

  node.Subscribe("/clock", cb);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  float rtfMin = kDefaultRTFMin;
  float rtfMax = kDefaultRTFMax;

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //ImGui::ShowDemoWindow();

    {
      std::lock_guard<std::mutex> lock(rtfsMutex);

      bool p_open;
      ImGui::Begin("RTF", &p_open, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Checkbox("Animate", &animate);

      // Line plot
      ImGui::PlotLines("RTF", &rtfs[0], rtfs.size(), 0, NULL, rtfMin, rtfMax,
          ImVec2(400, 400));

      ImGui::InputFloat("RTF Y-axis min", &rtfMin, 0.0f, 10.0f, "%.3f");
      ImGui::InputFloat("RTF Y-axis max", &rtfMax, 0.0f, 10.0f, "%.3f");

      // Histogram
      ImGui::Separator();
      hist.PlotHistogram("RTF Histogram", ImVec2(400, 400));

      if (ImGui::Button("Reset Histogram"))
      {
        hist.Reset();
      }

      // Statistics
      ImGui::Separator();
      ImGui::Text("Samples: %zi", stats.Count());
      ImGui::Text("Mean: %f", stats.Map()["mean"]);
      ImGui::Text("Var: %f", stats.Map()["var"]);
      ImGui::Text("Max: %f", stats.Map()["max"]);
      ImGui::Text("Min: %f", stats.Map()["min"]);
      if (ImGui::Button("Reset Statistics"))
      {
        stats.Reset();
      }

      ImGui::Separator();

      ignition::common::Time real_z(msg_z.real().sec(), msg_z.real().nsec());
      ignition::common::Time sim_z(msg_z.sim().sec(), msg_z.sim().nsec());

      ImGui::Text("Real Time: %.3f", real_z.Double());
      ImGui::Text("Sim Time: %.3f", sim_z.Double());
      ImGui::Text("Elapsed RTF: %.3f", sim_z.Double() / real_z.Double());


      ImGui::End();
    }

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
