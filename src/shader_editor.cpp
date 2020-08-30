/// Copyright (c) 2020, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file shader_editor.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-26-08
///
///\brief

#include <circe/circe.h>

using namespace circe::gl;

struct Light {
  ponos::vec3 position;
  ponos::vec3 ambient;
  ponos::vec3 diffuse;
  ponos::vec3 specular;
};

struct Material {
  ponos::vec3 kAmbient;
  ponos::vec3 kDiffuse;
  ponos::vec3 kSpecular;
  float shininess{};
};

class ShaderEditor : public BaseApp {
public:
  ShaderEditor() : BaseApp(800, 800) {
    auto lang = TextEditor::LanguageDefinition::GLSL();
    vertex_editor.SetLanguageDefinition(lang);
    fragment_editor.SetLanguageDefinition(lang);
    reset();
    // setup program
    program.addVertexAttribute("position", 0);
    program.addVertexAttribute("normal", 1);
    program.addUniform("Light.position", 2);
    program.addUniform("Light.ambient", 3);
    program.addUniform("Light.diffuse", 4);
    program.addUniform("Light.specular", 5);
    program.addUniform("Material.kAmbient", 6);
    program.addUniform("Material.kDiffuse", 7);
    program.addUniform("Material.kSpecular", 8);
    program.addUniform("Material.shininess", 9);
    program.addUniform("cameraPosition", 10);
    program.addUniform("model", 11);
    program.addUniform("view", 12);
    program.addUniform("projection", 13);
    // setup scene
    grid = CartesianGrid(5);
    this->app_->scene.add(&grid);
    glGenVertexArrays(1, &VAO);
    setupMesh();
    buildShader();
  }

  void render(circe::CameraInterface *camera) override {
    showInfo();
    showControls();
    showEditor(vertex_editor, "Vertex Shader");
    showEditor(fragment_editor, "Fragment Shader");
    // render object
    glBindVertexArray(VAO);
    vertex_buffer_.bind();
    index_buffer_.bind();
    vertex_buffer_.locateAttributes(program);

    program.use();
    program.setUniform("Light.position", light.position);
    program.setUniform("Light.ambient", light.ambient);
    program.setUniform("Light.diffuse", light.diffuse);
    program.setUniform("Light.specular", light.specular);
    program.setUniform("Material.kAmbient", material.kAmbient);
    program.setUniform("Material.kDiffuse", material.kDiffuse);
    program.setUniform("Material.kSpecular", material.kSpecular);
    program.setUniform("Material.shininess", material.shininess);
    program.setUniform("model", ponos::Transform());
    program.setUniform("view",
                       ponos::transpose(camera->getViewTransform().matrix()));
    program.setUniform("projection",
                       ponos::transpose(camera->getProjectionTransform().matrix()));
    program.setUniform("cameraPosition", camera->getPosition());

    glEnable(GL_DEPTH_TEST);
    glDrawElements(index_buffer_.bufferDescriptor.elementType,
                   index_buffer_.bufferDescriptor.elementCount *
                       index_buffer_.bufferDescriptor.elementSize,
                   GL_UNSIGNED_INT, 0);

  }

  void showInfo() {
    static ponos::Timer timer;
    static bool show = true;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("Example: Simple overlay", &show, window_flags)) {
      auto t = 10 - timer.tack() / 1000;
      static std::string compilation_status;
      if (t < 0) {
        if (buildShader())
          compilation_status = "Ok";
        else
          compilation_status = "Err";
        timer.tick();
      }
      ImGui::Text("Compilation %s (next %.2f)\n", compilation_status.c_str(), t);
      ImGui::Separator();
      ImGui::Text("FPS %u\n", this->last_FPS_);
      ImGui::Separator();
      if (ImGui::IsMousePosValid())
        ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
      else
        ImGui::Text("Mouse Position: <invalid>");
      ImGui::Separator();
      ImGui::Text("Vertex Shader Compilation\n%s\n", vertex_shader.err.c_str());
      ImGui::Separator();
      ImGui::Text("Fragment Shader Compilation\n%s\n", fragment_shader.err.c_str());

    }
    ImGui::End();
  }

  void showControls() {
    ImGui::Begin("Controls");
    if (ImGui::Button("reset"))
      reset();
    showLoadFile(vertex_editor, "Open Vertex Shader", "OpenFileVSKey");
    showLoadFile(fragment_editor, "Open Fragment Shader", "OpenFileFSKey");
    ImGui::Separator();
    ImGui::Text("Light\n");
    ImGui::ColorEdit3("Ambient", &light.ambient[0]);
    ImGui::ColorEdit3("Diffuse", &light.diffuse[0]);
    ImGui::ColorEdit3("Specular", &light.specular[0]);
    ImGui::Separator();
    ImGui::Text("Material\n");
    ImGui::ColorEdit3("M Ambient", &material.kAmbient[0]);
    ImGui::ColorEdit3("M Diffuse", &material.kDiffuse[0]);
    ImGui::ColorEdit3("M Specular", &material.kSpecular[0]);
    ImGui::SliderFloat("M Shininess", &material.shininess, 0.0, 300.0);
    ImGui::End();
  }

  static void showEditor(TextEditor &editor, const std::string &title) {
    ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    auto cpos = editor.GetCursorPosition();
    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
                editor.IsOverwrite() ? "Ovr" : "Ins",
                editor.CanUndo() ? "*" : " ",
                editor.GetLanguageDefinition().mName.c_str(), title.c_str());

    editor.Render("TextEditor");
    ImGui::End();
  }

  static void showLoadFile(TextEditor &editor, const std::string &label, const std::string &key) {
    if (ImGui::Button(label.c_str()))
      igfd::ImGuiFileDialog::Instance()->OpenDialog(key, "Choose File", "", ".");
    if (igfd::ImGuiFileDialog::Instance()->FileDialog(key)) {
      if (igfd::ImGuiFileDialog::Instance()->IsOk)
        editor.SetText(ponos::FileSystem::readFile(igfd::ImGuiFileDialog::Instance()->GetFilePathName()));
      igfd::ImGuiFileDialog::Instance()->CloseDialog(key);
    }
  }

  void reset() {
    auto vs = std::string(SHADERS_PATH) + "/basic.vert";
    auto fs = std::string(SHADERS_PATH) + "/basic.frag";
    vertex_editor.SetText(ponos::FileSystem::readFile(vs));
    fragment_editor.SetText(ponos::FileSystem::readFile(fs));
    // reset scene
    light.position = ponos::vec3(1, 1, 1);
    light.ambient = ponos::vec3(1, 1, 1);
    light.diffuse = ponos::vec3(1, 1, 1);
    light.specular = ponos::vec3(1, 1, 1);
    material.kAmbient = ponos::vec3(1, 1, 1);
    material.kDiffuse = ponos::vec3(1, 1, 1);
    material.kSpecular = ponos::vec3(1, 1, 1);
    material.shininess = 200;
  }

  bool buildShader() {
    bool fail = false;
    fail &= !vertex_shader.compile(vertex_editor.GetText(), GL_VERTEX_SHADER);
    fail &= !fragment_shader.compile(fragment_editor.GetText(), GL_FRAGMENT_SHADER);
    if (fail)
      return false;
    Program program_attempt;
    program_attempt.attach(vertex_shader);
    program_attempt.attach(fragment_shader);
    if (!program_attempt.link())
      return false;
    program.destroy();
    program.attach(vertex_shader);
    program.attach(fragment_shader);
    return true;
  }

  void setupMesh() {
    auto mesh = ponos::RawMeshes::icosphere(ponos::point3(), 1., 5, true, true);
    mesh->buildInterleavedData();
    std::vector<float> vertex_data;
    std::vector<u32> index_data;
    setup_buffer_data_from_mesh(*mesh, vertex_data, index_data);
    create_buffer_description_from_mesh(*mesh, vertex_buffer_.bufferDescriptor, index_buffer_.bufferDescriptor);
    glBindVertexArray(VAO);
    vertex_buffer_.set(vertex_data.data());
    index_buffer_.set(index_data.data());
  }

  TextEditor vertex_editor, fragment_editor;
  // scene
  Light light;
  Material material;
  CartesianGrid grid;
  // object
  GLuint VAO;
  VertexBuffer vertex_buffer_;
  IndexBuffer index_buffer_;
  // shader
  Shader vertex_shader;
  Shader fragment_shader;
  Program program;
};

int main() {
  ShaderEditor app;
  return app.run();
}