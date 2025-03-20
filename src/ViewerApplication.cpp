#include "ViewerApplication.hpp"

#include <iostream>
#include <numeric>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include "utils/Player.hpp"
#include "utils/cameras.hpp"
#include "utils/cube.hpp"
#include "utils/quad.hpp"
#include "utils/skybox.hpp"

#include <stb_image.h>
#include <stb_image_write.h>
#include <tiny_gltf.h>

#include <klein/klein.hpp>

bool first_mouse = true;
Player player{{0, 2, 0}};
float last_xpos = 0;
float last_ypos = 0;

GLuint modelMatrixLocation, modelViewProjMatrixLocation,
    modelViewMatrixLocation, normalMatrixLocation, uLightDirection,
    uLightIntensity, uBaseColorTexture, uBaseColorFactor, uMetallicFactor,
    uMetallicRoughnessTexture, uRoughnessFactor, uEmissiveFactor,
    uEmissiveTexture, uApplyOcclusion, uOcclusionFactor, uOcclusionTexture,
    uNormalTexture;

void keyCallback(
    GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    glfwSetWindowShouldClose(window, 1);
  }
}

static void cursor_position_callback(
    GLFWwindow * /*window*/, double xpos, double ypos)
{
  if (first_mouse) {
    last_xpos = xpos;
    last_ypos = ypos;
    first_mouse = false;
  }

  player.camera.rotateLeft(xpos - last_xpos);
  player.camera.rotateUp(last_ypos - ypos);

  last_xpos = xpos;
  last_ypos = ypos;
}

void process_continuous_input(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    player.jump();
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    player.moveUp(1.f);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    player.moveUp(-1.f);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    player.moveLeft(1.f);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    player.moveLeft(-1.f);
  }
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  player.update();
}

void getUniform(const GLProgram &glslProgram)
{
  modelMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelMatrix");
  modelViewProjMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelViewProjMatrix");
  modelViewMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelViewMatrix");
  normalMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uNormalMatrix");
}

int ViewerApplication::run()
{
  // Loader shaders
  auto glslProgram = compileProgram({m_ShadersRootPath / m_vertexShader,
      m_ShadersRootPath / m_fragmentShader});

  // tinygltf::Model model;
  // loadGltfFile(model);

  std::cout << "Model imported : " << m_gltfFilePath << std::endl;

  // glm::vec3 bboxMin, bboxMax;
  // computeSceneBounds(model, bboxMin, bboxMax);

  GLuint tangentVBO;
  glGenBuffers(1, &tangentVBO);

  // ComputeTangents(model);

  // Build projection matrix
  const auto diag = glm::vec3(1., 1., 1);
  auto maxDistance = glm::length(diag);
  auto farView = 500.f;
  const auto projMatrix = glm::perspective(70.f,
      float(m_nWindowWidth) / m_nWindowHeight, 0.001f * maxDistance, farView);

  // std::unique_ptr<CameraController> cameraController =
  //     std::make_unique<FirstPersonCameraController>(
  //         m_GLFWHandle.window(), 0.5f * maxDistance);
  // if (m_hasUserCamera) {
  //   cameraController->setCamera(m_userCamera);
  // } else {
  //   cameraController->setCamera(
  //       Camera{glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)});
  // }

  // const auto textureObjects = createTextureObjects(model);

  // Gen default texture for object
  float white[] = {1., 1., 1., 1.};
  GLuint whiteTexture;
  glGenTextures(1, &whiteTexture);
  glBindTexture(GL_TEXTURE_2D, whiteTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, white);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

  // const auto bufferObjects = createBufferObjects(model);
  // bool tangentAccessor = false;

  // std::vector<VaoRange> meshToVertexArrays;
  // const auto vertexArrayObjects = createVertexArrayObjects(
  //     model, bufferObjects, meshToVertexArrays, tangentAccessor);

  // Setup OpenGL state for rendering
  glEnable(GL_DEPTH_TEST);
  glslProgram.use();

  getUniform(glslProgram);

  const auto pathToFaces = "assets/";

  const std::vector<std::string> faces{"assets/skybox/right.jpg",
      "assets/skybox/left.jpg", "assets/skybox/top.jpg",
      "assets/skybox/bottom.jpg", "assets/skybox/front.jpg",
      "assets/skybox/back.jpg"};

  QuadCustom quad(1, 1);
  CubeCustom cube(farView, farView, farView);
  Skybox skybox(faces, cube, m_ShadersRootPath);

  quad.initObj(0, 1, 2);
  // cube.initObj(0, 1, 2);

  const auto drawScene = [&]() {
    glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const auto viewMatrix = player.camera.getViewMatrix();
    const auto modelMatrix = glm::mat4(1.0f);

    glslProgram.use();

    quad.draw(modelMatrix, viewMatrix, projMatrix, modelMatrixLocation,
        modelViewProjMatrixLocation, modelViewMatrixLocation,
        normalMatrixLocation);

    skybox.draw(modelMatrix, viewMatrix, projMatrix, modelMatrixLocation,
        modelViewProjMatrixLocation, modelViewMatrixLocation,
        normalMatrixLocation);
  };

  // Uniform variable for light
  glm::vec3 lightDirection(1.f, 1.f, 1.f);
  glm::vec3 lightIntensity(1.f, 1.f, 1.f);
  glm::vec3 color = {1.f, 1.f, 1.f};
  float theta = 1.f;
  float phi = 1.f;
  bool lightCam = false;

  // Uniform variable for occlusion
  bool occlusionState = true;

  // Loop until the user closes the window
  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose();
      ++iterationCount) {
    const auto seconds = glfwGetTime();

    process_continuous_input(m_GLFWHandle.window());

    drawScene();

    // GUI code:
    imguiNewFrame();

    {
      ImGui::Begin("GUI");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
          1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("position : %.3f %.3f %.3f", player.camera.getPosition().x,
            player.camera.getPosition().y, player.camera.getPosition().z);
        ImGui::End();
      }
    }

    imguiRenderFrame();

    glfwPollEvents(); // Poll for and process events

    auto ellapsedTime = glfwGetTime() - seconds;
    auto guiHasFocus =
        ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
    if (!guiHasFocus) {
      // cameraController->update(float(ellapsedTime));
    }

    m_GLFWHandle.swapBuffers(); // Swap front and back buffers
  }

  // TODO clean up allocated GL data

  return 0;
}

ViewerApplication::ViewerApplication(
    const fs::path &appPath, uint32_t width, uint32_t height) :
    m_nWindowWidth(width),
    m_nWindowHeight(height),
    m_AppPath{appPath},
    m_AppName{m_AppPath.stem().string()},
    m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
    m_ShadersRootPath{m_AppPath.parent_path() / "shaders"}
{
  ImGui::GetIO().IniFilename =
      m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows
                                  // positions in this file

  glfwSetKeyCallback(m_GLFWHandle.window(), keyCallback);
  glfwSetCursorPosCallback(m_GLFWHandle.window(), cursor_position_callback);
  glfwSetInputMode(m_GLFWHandle.window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  printGLVersion();
}