#include "ViewerApplication.hpp"

#include <iostream>
#include <numeric>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include "utils/cameras.hpp"
#include "utils/cube.hpp"
#include "utils/quad.hpp"
#include "utils/skybox.hpp"

#include <stb_image.h>
#include <stb_image_write.h>
#include <tiny_gltf.h>

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

bool ViewerApplication::loadGltfFile(tinygltf::Model &model)
{
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;
  bool ret =
      loader.LoadASCIIFromFile(&model, &err, &warn, m_gltfFilePath.string());

  if (ret) {
    printf("glTF has been loaded sucessfully\n");
  }

  if (!ret) {
    printf("Failed to parse glTF\n");
    return -1;
  }

  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
  }

  return ret;
}

std::vector<GLuint> ViewerApplication::createBufferObjects(
    const tinygltf::Model &model)
{
  std::vector<GLuint> bufferObjects(
      model.buffers.size(), 0); // Assuming buffers is a std::vector of Buffer
  glGenBuffers(model.buffers.size(), bufferObjects.data());
  for (size_t i = 0; i < model.buffers.size(); ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[i]);
    glBufferStorage(GL_ARRAY_BUFFER,
        model.buffers[i].data.size(), // Assume a Buffer has a data member
                                      // variable of type std::vector
        model.buffers[i].data.data(), 0);
  }
  glBindBuffer(
      GL_ARRAY_BUFFER, 0); // Cleanup the binding point after the loop only

  return bufferObjects;
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
  const auto projMatrix =
      glm::perspective(70.f, float(m_nWindowWidth) / m_nWindowHeight,
          0.001f * maxDistance, 1.5f * maxDistance);

  std::unique_ptr<CameraController> cameraController =
      std::make_unique<FirstPersonCameraController>(
          m_GLFWHandle.window(), 0.5f * maxDistance);
  if (m_hasUserCamera) {
    cameraController->setCamera(m_userCamera);
  } else {
    const auto center = glm::vec3(0.f, 0.f, 0.f);
    const auto up = glm::vec3(0, 1, 0);
    const auto eye = glm::vec3(1.f, 1.f, 1.f);
    cameraController->setCamera(
        Camera{glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)});
  }

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
  CubeCustom cube(1, 1, 1);
  Skybox skybox(faces, cube, m_ShadersRootPath);

  quad.initObj(0, 1, 2);
  // cube.initObj(0, 1, 2);

  const auto drawScene = [&](const Camera &camera) {
    glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const auto viewMatrix = camera.getViewMatrix();
    const auto modelMatrix = glm::mat4(1.0f);

    glslProgram.use();

    getUniform(glslProgram);

    quad.draw(modelMatrix, viewMatrix, projMatrix, modelMatrixLocation,
        modelViewProjMatrixLocation, modelViewMatrixLocation,
        normalMatrixLocation);

    // cube.draw(modelMatrix, viewMatrix, projMatrix, modelMatrixLocation,
    //     modelViewProjMatrixLocation, modelViewMatrixLocation,
    //     normalMatrixLocation);

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

  // if (!m_OutputPath.empty()) {
  //   const auto numComponents = 3;
  //   std::vector<unsigned char> pixels(
  //       m_nWindowWidth * m_nWindowHeight * numComponents);
  //   renderToImage(
  //       m_nWindowWidth, m_nWindowHeight, numComponents, pixels.data(), [&]()
  //       {
  //         const auto camera = cameraController->getCamera();
  //         drawScene(
  //             camera, lightDirection, lightIntensity, lightCam,
  //             occlusionState);
  //       });
  //   flipImageYAxis(
  //       m_nWindowWidth, m_nWindowHeight, numComponents, pixels.data());

  //   // Write png on disk
  //   const auto strPath = m_OutputPath.string();
  //   stbi_write_png(
  //       strPath.c_str(), m_nWindowWidth, m_nWindowHeight, 3, pixels.data(),
  //       0);

  //   return 0; // Exit, in that mode we don't want to run interactive
  //             // viewer
  // }

  // Loop until the user closes the window
  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose();
      ++iterationCount) {
    const auto seconds = glfwGetTime();

    const auto camera = cameraController->getCamera();
    // drawScene(camera, lightDirection, lightIntensity, lightCam,
    // occlusionState);

    drawScene(camera);

    // GUI code:
    imguiNewFrame();

    {
      ImGui::Begin("GUI");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
          1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("eye: %.3f %.3f %.3f", camera.eye().x, camera.eye().y,
            camera.eye().z);
        ImGui::Text("center: %.3f %.3f %.3f", camera.center().x,
            camera.center().y, camera.center().z);
        ImGui::Text(
            "up: %.3f %.3f %.3f", camera.up().x, camera.up().y, camera.up().z);

        ImGui::Text("front: %.3f %.3f %.3f", camera.front().x, camera.front().y,
            camera.front().z);
        ImGui::Text("left: %.3f %.3f %.3f", camera.left().x, camera.left().y,
            camera.left().z);

        ImGui::End();
      }
    }

    imguiRenderFrame();

    glfwPollEvents(); // Poll for and process events

    auto ellapsedTime = glfwGetTime() - seconds;
    auto guiHasFocus =
        ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
    if (!guiHasFocus) {
      cameraController->update(float(ellapsedTime));
    }

    m_GLFWHandle.swapBuffers(); // Swap front and back buffers
  }

  // TODO clean up allocated GL data

  return 0;
}

ViewerApplication::ViewerApplication(const fs::path &appPath, uint32_t width,
    uint32_t height, const std::string &vertexShader,
    const std::string &fragmentShader) :
    m_nWindowWidth(width),
    m_nWindowHeight(height),
    m_AppPath{appPath},
    m_AppName{m_AppPath.stem().string()},
    m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
    m_ShadersRootPath{m_AppPath.parent_path() / "shaders"}
{
  if (!vertexShader.empty()) {
    m_vertexShader = vertexShader;
  }

  if (!fragmentShader.empty()) {
    m_fragmentShader = fragmentShader;
  }

  ImGui::GetIO().IniFilename =
      m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows
                                  // positions in this file

  glfwSetKeyCallback(m_GLFWHandle.window(), keyCallback);

  printGLVersion();
}