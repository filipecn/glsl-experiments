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
#include <json11.hpp>

using namespace json11;
using namespace circe::gl;

struct Model {
  circe::gl::Program program;
  circe::Material material;
  ponos::Transform transform;
  // new way
  circe::gl::VertexArrayObject vao;
  circe::gl::VertexBuffer vb;
  circe::gl::IndexBuffer ib;
  void draw() {
    vao.bind();
    vb.bind();
    ib.bind();
    ib.draw();
  }
};

class vec3 : public ponos::vec3 {
public:
  vec3() {
    this->x = this->y = this->z = 0;
  }
  vec3(real_t x, real_t y, real_t z) {
    this->x = x;
    this->y = y;
    this->z = z;

  }
  [[nodiscard]] Json to_json() const { return Json::array{x, y, z}; }
};

struct Light {
  vec3 direction;
  vec3 point;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  [[maybe_unused]] [[nodiscard]] Json to_json() const {
    return Json::object{
        {"direction", direction},
        {"point", point},
        {"ambient", ambient},
        {"diffuse", diffuse},
        {"specular", specular}};
  }
};

struct Material {
  vec3 kAmbient;
  vec3 kDiffuse;
  vec3 kSpecular;
  float shininess{};
  [[maybe_unused]] [[nodiscard]] Json to_json() const {
    return Json::object{
        {"kAmbient", kAmbient},
        {"kDiffuse", kDiffuse},
        {"kSpecular", kSpecular},
        {"shininess", shininess}
    };
  }
};

struct LightObject {
  LightObject() {
    // setup shader program
    Shader vs_shader(
        "#version 440 core\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 11) uniform mat4 model;\n"
        "layout(location = 12) uniform mat4 view;\n"
        "layout(location = 13) uniform mat4 projection;\n"
        "void main() {\n"
        "  gl_Position = projection * view * model * vec4(position, 1.0);\n"
        "}",
        GL_VERTEX_SHADER);
    Shader fs_shader("#version 440 core\n"
                     "layout(location = 0) out vec4 fragColor;"
                     "layout(location = 1) uniform vec3 color;"
                     "void main() {\n"
                     "  fragColor = vec4(color, 1.0);\n"
                     "}"
                     "", GL_FRAGMENT_SHADER);
    program.attach(vs_shader);
    program.attach(fs_shader);
    FATAL_ASSERT(program.link());
    // setup point mesh
    {
      auto mesh = ponos::RawMeshes::icosphere(ponos::point3(), 1., 3, false, false);
      std::vector<float> vertex_data;
      std::vector<u32> index_data;
      setup_buffer_data_from_mesh(*mesh, vertex_data, index_data);
      model.vb.attributes.push<ponos::point3>("position");
      // upload data
      model.vb = vertex_data;
      model.ib = index_data;
      model.ib.element_count = index_data.size() / 3;
      // bind attributes
      model.vao.bind();
      model.vb.bindAttributeFormats();
    }
    // setup direction mesh
    ponos::RawMesh mesh_d;
    mesh_d.primitiveType = ponos::GeometricPrimitiveType::LINES;
    mesh_d.positionDescriptor.elementSize = 3;
    mesh_d.positionDescriptor.count = 2;
    mesh_d.meshDescriptor.elementSize = 2;
    mesh_d.meshDescriptor.count = 1;
    mesh_d.addPosition({0.0, 0.0, 0.0, 1.0, 1.0, 1.0});
    mesh_d.addFace({0, 1});
    mesh_d.buildInterleavedData();
    std::vector<float> vertex_data_d;
    std::vector<u32> index_data_d;
    setup_buffer_data_from_mesh(mesh_d, vertex_data_d, index_data_d);
    // upload data
    model_d.vb = vertex_data_d;
    model_d.ib = index_data_d;
    model_d.ib.element_count = 1;
    // bind attributes
    model_d.vao.bind();
    model_d.vb.bindAttributeFormats();
  }
  void render(circe::CameraInterface *camera) {
    if (show_point)
      drawPoint(camera);
    if (show_direction)
      drawDirection(camera);
  }
  void drawPoint(circe::CameraInterface *camera) {
    program.use();
    program.setUniform("model", ponos::transpose(
        (ponos::translate(light->point) * ponos::scale(0.2, 0.2, 0.2)).matrix()));
    program.setUniform("view",
                       ponos::transpose(camera->getViewTransform().matrix()));
    program.setUniform("projection",
                       ponos::transpose(camera->getProjectionTransform().matrix()));
    program.setUniform("color", light->ambient);
    glEnable(GL_DEPTH_TEST);
    model.draw();
  }
  void drawDirection(circe::CameraInterface *camera) {
    f32 scale_factor = 10;
    program.use();
    program.setUniform("model",
                       ponos::transpose(ponos::scale(light->direction.x * scale_factor,
                                                     light->direction.y * scale_factor,
                                                     light->direction.z * scale_factor).matrix()));
    program.setUniform("view",
                       ponos::transpose(camera->getViewTransform().matrix()));
    program.setUniform("projection",
                       ponos::transpose(camera->getProjectionTransform().matrix()));
    program.setUniform("color", light->ambient);
    glEnable(GL_DEPTH_TEST);
    model_d.draw();
  }
  Model model;
  Model model_d;
  Program program;
  Light *light{nullptr};
  bool show_point{true};
  bool show_direction{true};
};

class TextureUnit {
public:
  TextureUnit() {
    // load default check texture
  }
  void show(const ponos::Path &config_path) {
    if (ImGui::Button(unit_name.c_str()))
      igfd::ImGuiFileDialog::Instance()->OpenDialog(unit_name,
                                                    "Choose File",
                                                    ".jpg",
                                                    config_path.cwd().fullName());
    if (igfd::ImGuiFileDialog::Instance()->FileDialog(unit_name)) {
      if (igfd::ImGuiFileDialog::Instance()->IsOk)
        load(ponos::Path(igfd::ImGuiFileDialog::Instance()->GetFilePathName()));
      igfd::ImGuiFileDialog::Instance()->CloseDialog(unit_name);
    }
    ImGui::SameLine();
    if (ImGui::Button(ponos::concat("View ", unit_name).c_str()))
      show_view_ = !show_view_;
    if (show_view_) {
      ImGui::Begin(unit_name.c_str(), &show_view_);
      texture.bind(GL_TEXTURE0);
      ImGui::Image((void *) (intptr_t) (texture.textureObjectId()), {256, 256},
                   {0, 1}, {1, 0});
      ImGui::End();
    }
  }
  void load(const ponos::Path &path) {
    path_ = path;
    texture = Texture::fromFile(path);
  }
  [[nodiscard]] Json to_json() const { return Json::object{{"unit_name", unit_name}, {"path", path_.fullName()}}; }

  std::string unit_name = "GL_TEXTURE0";
  Texture texture;
private:
  bool show_view_{false};
  ponos::Path path_;
};

class ShaderEditor : public BaseApp {
public:
  ShaderEditor() : BaseApp(800, 800) {
    // texture views
    for (int i = 0; i < 2; ++i)
      texture_views[i].unit_name = ponos::concat("GL_TEXTURE", i);
    // editor
    auto lang = TextEditor::LanguageDefinition::GLSL();
    vertex_editor.SetLanguageDefinition(lang);
    fragment_editor.SetLanguageDefinition(lang);
    loadConfig(ponos::Path(std::string(SHADERS_PATH) + "/blinn_phong/blinn_phong.json"));
    // setup scene
    setupMesh();
    buildShader();
  }

  void render(circe::CameraInterface *camera) override {
    showInfo();
    showControls();
    showEditor(vertex_editor, "Vertex Shader", &show_vertex_editor);
    showEditor(fragment_editor, "Fragment Shader", &show_fragment_editor);
    // render object
    mesh_.program.use();
    // attach textures
    for (int i = 0; i < 2; ++i) {
      auto uniform_name = ponos::concat("channel", i);
      if (mesh_.program.hasUniform(uniform_name)) {
        mesh_.program.setUniform(uniform_name, i);
        texture_views[i].texture.bind(GL_TEXTURE0 + i);
      }
    }
    if (mesh_.program.hasUniform("light.point"))
      mesh_.program.setUniform("light.point", light.point);
    if (mesh_.program.hasUniform("light.direction"))
      mesh_.program.setUniform("light.direction", light.direction);
    if (mesh_.program.hasUniform("light.ambient"))
      mesh_.program.setUniform("light.ambient", light.ambient);
    if (mesh_.program.hasUniform("light.diffuse"))
      mesh_.program.setUniform("light.diffuse", light.diffuse);
    if (mesh_.program.hasUniform("light.specular"))
      mesh_.program.setUniform("light.specular", light.specular);
    if (mesh_.program.hasUniform("material.kAmbient"))
      mesh_.program.setUniform("material.kAmbient", material.kAmbient);
    if (mesh_.program.hasUniform("material.diffuse"))
      mesh_.program.setUniform("material.kDiffuse", material.kDiffuse);
    if (mesh_.program.hasUniform("material.kSpecular"))
      mesh_.program.setUniform("material.kSpecular", material.kSpecular);
    if (mesh_.program.hasUniform("material.shininess"))
      mesh_.program.setUniform("material.shininess", material.shininess);
    if (mesh_.program.hasUniform("model"))
      mesh_.program.setUniform("model", ponos::transpose(mesh_.transform.matrix()));
    if (mesh_.program.hasUniform("view"))
      mesh_.program.setUniform("view",
                               ponos::transpose(camera->getViewTransform().matrix()));
    if (mesh_.program.hasUniform("projection"))
      mesh_.program.setUniform("projection",
                               ponos::transpose(camera->getProjectionTransform().matrix()));
    if (mesh_.program.hasUniform("cameraPosition"))
      mesh_.program.setUniform("cameraPosition", camera->getPosition());
    if (mesh_.program.hasUniform("screenResolution"))
      mesh_.program.setUniform("screenResolution",
                               ponos::vec2(this->app_->viewports[0].width, this->app_->viewports[0].height));
    glEnable(GL_DEPTH_TEST);
//    mesh_.draw();
    model.draw();
//    light_object.light = &light;
//    light_object.render(camera);
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
      auto t = 2 - timer.tack() / 1000;
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
    showConfigFileButtons();
    showLoadFile(vertex_editor, "Open Vertex Shader", "OpenFileVSKey", ".vert", show_vertex_editor);
    showLoadFile(fragment_editor, "Open Fragment Shader", "OpenFileFSKey", ".frag", show_fragment_editor);
    ImGui::Separator();
    ImGui::Text("Mesh\n");
    int mesh_option = 0;
    if (ImGui::Combo("Mesh", (int *) &mesh_option, "Sphere\0Plane\0Terrain\0"))
      setupMesh(mesh_option);
//    ImGui::SliderFloat3("Scale", &mesh.scale[0], 0.0001, 100);
    ImGui::Separator();
    ImGui::Text("Light\n");
    ImGui::SliderFloat3("Position", &light.point.x, -5, 5);
    ImGui::SameLine();
//    if (ImGui::Button("Show Position"))
//      light_object.show_point = !light_object.show_point;
    ImGui::SliderFloat3("Direction", &light.direction.x, -1, 1);
    ImGui::SameLine();
//    if (ImGui::Button("Show Direction"))
//      light_object.show_direction = !light_object.show_direction;
    ImGui::ColorEdit3("Ambient", &light.ambient[0]);
    ImGui::SameLine();
    if (ImGui::ArrowButton("light_ambient", ImGuiDir_Down))
      light.diffuse = light.ambient;
    ImGui::ColorEdit3("Diffuse", &light.diffuse[0]);
    ImGui::ColorEdit3("Specular", &light.specular[0]);
    ImGui::Separator();
    ImGui::Text("Material\n");
    ImGui::ColorEdit3("M Ambient", &material.kAmbient[0]);
    ImGui::SameLine();
    if (ImGui::ArrowButton("material_ambient", ImGuiDir_Down))
      material.kDiffuse = material.kAmbient;
    ImGui::ColorEdit3("M Diffuse", &material.kDiffuse[0]);
    ImGui::ColorEdit3("M Specular", &material.kSpecular[0]);
    ImGui::SliderFloat("M Shininess", &material.shininess, 0.0, 300.0);
    ImGui::Separator();
    // Textures
    for (auto &v : texture_views)
      v.show(config_path);
  }

  void showConfigFileButtons() {
    if (ImGui::Button("Load Config"))
      igfd::ImGuiFileDialog::Instance()->OpenDialog("LoadConfigKey",
                                                    "Choose File",
                                                    ".json",
                                                    config_path.cwd().fullName(),
                                                    1);
    ImGui::SameLine();
    if (ImGui::Button("Save Config"))
      igfd::ImGuiFileDialog::Instance()->OpenDialog("SaveConfigKey",
                                                    "Choose File",
                                                    ".json",
                                                    config_path.cwd().fullName(),
                                                    "config",
                                                    1);
    ImGui::SameLine();
    if (ImGui::Button("reset"))
      loadConfig(config_path);
    if (igfd::ImGuiFileDialog::Instance()->FileDialog("LoadConfigKey")) {
      if (igfd::ImGuiFileDialog::Instance()->IsOk)
        loadConfig(ponos::Path(igfd::ImGuiFileDialog::Instance()->GetFilePathName()));
      igfd::ImGuiFileDialog::Instance()->CloseDialog("LoadConfigKey");
    }
    if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveConfigKey")) {
      if (igfd::ImGuiFileDialog::Instance()->IsOk) {
        config_path = igfd::ImGuiFileDialog::Instance()->GetFilePathName(),
            ponos::FileSystem::writeFile(config_path.fullName(),
                                         Json(Json::object{{"light", light}, {"material", material},
                                                           {"t0", texture_views[0]},
                                                           {"t1", texture_views[1]}}).dump());
      }
      igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveConfigKey");
    }
  }

  static void showEditor(TextEditor &editor, const std::string &title, bool *p_open) {
    if (*p_open) {
      ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
      ImGui::Begin(title.c_str(), p_open, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
      ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
      auto cpos = editor.GetCursorPosition();
      ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
                  editor.IsOverwrite() ? "Ovr" : "Ins",
                  editor.CanUndo() ? "*" : " ",
                  editor.GetLanguageDefinition().mName.c_str(), title.c_str());
      editor.Render("TextEditor");
      ImGui::End();
    }
  }

  void showLoadFile(TextEditor &editor,
                    const std::string &label,
                    const std::string &key,
                    const std::string &file_ext, bool &p_open) const {
    if (ImGui::Button(label.c_str()))
      igfd::ImGuiFileDialog::Instance()->OpenDialog(key, "Choose File", "", config_path.cwd().fullName());
    ImGui::SameLine();
    if (ImGui::Button(ponos::concat("Save ", file_ext).c_str())) {
      auto basename = config_path.cwd() + ponos::FileSystem::basename(config_path.fullName(), ".json");
      ponos::FileSystem::writeFile(ponos::concat(basename, file_ext), editor.GetText());
    }
    if (igfd::ImGuiFileDialog::Instance()->FileDialog(key)) {
      if (igfd::ImGuiFileDialog::Instance()->IsOk) {
        editor.SetText(ponos::FileSystem::readFile(igfd::ImGuiFileDialog::Instance()->GetFilePathName() + file_ext));
      }
      igfd::ImGuiFileDialog::Instance()->CloseDialog(key);
    }
    ImGui::SameLine();
    if (ImGui::Button(ponos::concat("Toggle ", file_ext).c_str()))
      p_open = !p_open;
  }

  void loadConfig(const ponos::Path &path) {
    std::string err;
    auto json = Json::parse(ponos::FileSystem::readFile(path), err);
    if (err.empty()) {
      for (u32 d = 0; d < 3; ++d) {
        light.direction[d] = json["light"]["direction"][d].number_value();
        light.point[d] = json["light"]["point"][d].number_value();
        light.ambient[d] = json["light"]["ambient"][d].number_value();
        light.diffuse[d] = json["light"]["diffuse"][d].number_value();
        light.specular[d] = json["light"]["specular"][d].number_value();
        material.kAmbient[d] = json["material"]["kAmbient"][d].number_value();
        material.kDiffuse[d] = json["material"]["kDiffuse"][d].number_value();
        material.kSpecular[d] = json["material"]["kSpecular"][d].number_value();
      }
      material.shininess = json["material"]["shinniness"].number_value();

      config_path = path;

      // load textures
      for (int i = 0; i < 2; ++i) {
        auto tid = ponos::concat("t", i);
        texture_views[i].unit_name = json[tid]["unit_name"].string_value();
        if (texture_views[i].unit_name.empty())
          texture_views[i].unit_name = ponos::concat("GL_TEXTURE", i);
        auto tf = json[tid]["path"].string_value();
        if (!tf.empty())
          texture_views[i].load(ponos::Path(config_path.cwd() + tf));
      }

      auto basename = config_path.cwd() + ponos::FileSystem::basename(config_path.fullName(), ".json");

      vertex_editor.SetText(ponos::FileSystem::readFile(ponos::concat(basename, ".vert")));
      fragment_editor.SetText(ponos::FileSystem::readFile(ponos::concat(basename, ".frag")));
    }
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
    mesh_.program.destroy();
    mesh_.program.attach(vertex_shader);
    mesh_.program.attach(fragment_shader);
    return true;
  }

  void setupMesh(int mesh_option = 0) {
    static int last_option = -1;
    if (last_option != mesh_option) {
      model = circe::Shapes::plane(ponos::Plane::XZ(),
                                   ponos::point3(),
                                   ponos::vec3(1, 0, 0),
                                   1,
                                   circe::shape_options::normal |
                                       circe::shape_options::tangent_space);
      last_option = mesh_option;
      ponos::RawMeshSPtr raw_mesh;
      switch (mesh_option) {
      case 0:raw_mesh = ponos::RawMeshes::icosphere(ponos::point3(), 1., 3, true, true);
        break;
      case 1:
        model = circe::Shapes::plane(ponos::Plane::XY(),
                                     ponos::point3(),
                                     ponos::vec3(1, 0, 0),
                                     1,
                                     circe::shape_options::normal |
                                         circe::shape_options::tangent_space);
        raw_mesh = ponos::RawMeshes::plane(ponos::Plane::XY(), ponos::point3(), ponos::vec3(1, 0, 0), 1);
        break;
      case 2:raw_mesh = ponos::RawMeshes::plane(ponos::Plane::XZ(), ponos::point3(), ponos::vec3(1, 0, 0), 20);
        for (int i = 0; i <= 20; ++i)
          for (int j = 0; j <= 20; ++j)
            raw_mesh->positions[i * 21 * 3 + j * 3 + 1] = ponos::Perlin::at(ponos::point2(i, j) * 0.05);
        break;
      default: break;
      }
      // prepare mesh data
      std::vector<float> vertex_data;
      std::vector<u32> index_data;
      circe::gl::setup_buffer_data_from_mesh(*raw_mesh, vertex_data, index_data);
      mesh_.vb.attributes = circe::gl::VertexBuffer::Attributes();
      // describe vertex buffer
      mesh_.vb.attributes.push<ponos::point3>("position");
      if (raw_mesh->normalDescriptor.count)
        mesh_.vb.attributes.push<ponos::vec3>("normal");
      if (raw_mesh->texcoordDescriptor.count)
        mesh_.vb.attributes.push<ponos::point2>("uv");
      // upload data
      mesh_.vb = vertex_data;
      mesh_.ib = index_data;
      mesh_.ib.element_count = index_data.size() / 3;
      // bind attributes
      mesh_.vao.bind();
      mesh_.vb.bindAttributeFormats();
    }
  }

  // Textures
  TextureUnit texture_views[2];
  bool show_texture_views[2]{false, false};
  // gui
  ponos::Path config_path;
  bool show_vertex_editor{true}, show_fragment_editor{true};
  TextEditor vertex_editor, fragment_editor;
  // scene
  Light light;
  Material material;
  // object
  Model mesh_;
  circe::gl::SceneModel model;
  // shader
  Shader vertex_shader;
  Shader fragment_shader;
  // light object
//  LightObject light_object;
};

int main() {
  ShaderEditor app;
  return app.run();
}