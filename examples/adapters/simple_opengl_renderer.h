#pragma once

#include <bombfork/prong/rendering/irenderer.h>

// GLFW will include OpenGL headers
#include "font_renderer.h"
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace bombfork::prong::examples {

// OpenGL 3.3 Core function pointer types
using PFNGLGENBUFFERSPROC = void(GLAPIENTRY*)(GLsizei, GLuint*);
using PFNGLDELETEBUFFERSPROC = void(GLAPIENTRY*)(GLsizei, const GLuint*);
using PFNGLBINDBUFFERPROC = void(GLAPIENTRY*)(GLenum, GLuint);
using PFNGLBUFFERDATAPROC = void(GLAPIENTRY*)(GLenum, GLsizeiptr, const void*, GLenum);
using PFNGLBUFFERSUBDATAPROC = void(GLAPIENTRY*)(GLenum, GLintptr, GLsizeiptr, const void*);
using PFNGLGENVERTEXARRAYSPROC = void(GLAPIENTRY*)(GLsizei, GLuint*);
using PFNGLDELETEVERTEXARRAYSPROC = void(GLAPIENTRY*)(GLsizei, const GLuint*);
using PFNGLBINDVERTEXARRAYPROC = void(GLAPIENTRY*)(GLuint);
using PFNGLVERTEXATTRIBPOINTERPROC = void(GLAPIENTRY*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
using PFNGLENABLEVERTEXATTRIBARRAYPROC = void(GLAPIENTRY*)(GLuint);
using PFNGLCREATESHADERPROC = GLuint(GLAPIENTRY*)(GLenum);
using PFNGLDELETESHADERPROC = void(GLAPIENTRY*)(GLuint);
using PFNGLSHADERSOURCEPROC = void(GLAPIENTRY*)(GLuint, GLsizei, const GLchar**, const GLint*);
using PFNGLCOMPILESHADERPROC = void(GLAPIENTRY*)(GLuint);
using PFNGLGETSHADERIVPROC = void(GLAPIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETSHADERINFOLOGPROC = void(GLAPIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLCREATEPROGRAMPROC = GLuint(GLAPIENTRY*)();
using PFNGLDELETEPROGRAMPROC = void(GLAPIENTRY*)(GLuint);
using PFNGLATTACHSHADERPROC = void(GLAPIENTRY*)(GLuint, GLuint);
using PFNGLLINKPROGRAMPROC = void(GLAPIENTRY*)(GLuint);
using PFNGLGETPROGRAMIVPROC = void(GLAPIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETPROGRAMINFOLOGPROC = void(GLAPIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLUSEPROGRAMPROC = void(GLAPIENTRY*)(GLuint);
using PFNGLGETUNIFORMLOCATIONPROC = GLint(GLAPIENTRY*)(GLuint, const GLchar*);
using PFNGLUNIFORMMATRIX4FVPROC = void(GLAPIENTRY*)(GLint, GLsizei, GLboolean, const GLfloat*);
using PFNGLUNIFORM4FPROC = void(GLAPIENTRY*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
using PFNGLUNIFORM1IPROC = void(GLAPIENTRY*)(GLint, GLint);

/**
 * @brief Modern OpenGL 3.3 Core Profile renderer implementation for Prong
 *
 * This renderer uses modern OpenGL features:
 * - Shaders (GLSL 330)
 * - Vertex Array Objects (VAO)
 * - Vertex Buffer Objects (VBO)
 * - No deprecated fixed-function pipeline
 *
 * This is still a minimal implementation focused on clarity.
 * A production renderer would include:
 * - Texture atlas management
 * - Batch rendering for sprites
 * - Font rendering with FreeType
 * - Advanced shader management
 * - Vertex buffer optimization
 */
class SimpleOpenGLRenderer : public bombfork::prong::rendering::IRenderer {
private:
  struct GLTextureHandle : public bombfork::prong::rendering::TextureHandle {
    GLuint textureId = 0;
    ~GLTextureHandle() {
      if (textureId != 0) {
        glDeleteTextures(1, &textureId);
      }
    }
  };

  bool initialized = false;
  int windowWidth = 1280;
  int windowHeight = 720;

  // Shader programs
  GLuint rectShaderProgram = 0;
  GLuint spriteShaderProgram = 0;

  // VAO/VBO for rendering
  GLuint rectVAO = 0;
  GLuint rectVBO = 0;
  GLuint spriteVAO = 0;
  GLuint spriteVBO = 0;

  // Uniform locations
  GLint rectProjLoc = -1;
  GLint rectColorLoc = -1;
  GLint spriteProjLoc = -1;
  GLint spriteTextureLoc = -1;
  GLint spriteColorLoc = -1;

  // Projection matrix (orthographic)
  float projectionMatrix[16];

  // OpenGL 3.3 Core function pointers
  PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
  PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
  PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
  PFNGLBUFFERDATAPROC glBufferData = nullptr;
  PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
  PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
  PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
  PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
  PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
  PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
  PFNGLCREATESHADERPROC glCreateShader = nullptr;
  PFNGLDELETESHADERPROC glDeleteShader = nullptr;
  PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
  PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
  PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
  PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
  PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
  PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
  PFNGLATTACHSHADERPROC glAttachShader = nullptr;
  PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
  PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
  PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
  PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
  PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
  PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
  PFNGLUNIFORM4FPROC glUniform4f = nullptr;
  PFNGLUNIFORM1IPROC glUniform1i = nullptr;

  // Font renderer for text
  FontRenderer* fontRenderer = nullptr;

public:
  SimpleOpenGLRenderer() = default;

  ~SimpleOpenGLRenderer() override { cleanup(); }

  /**
   * @brief Set the font renderer to use for text rendering
   */
  void setFontRenderer(FontRenderer* fr) { fontRenderer = fr; }

  bool initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;

    // Load OpenGL 3.3 Core functions dynamically
    if (!loadGLFunctions()) {
      std::cerr << "Failed to load OpenGL functions" << std::endl;
      return false;
    }

    // Create shaders
    if (!createShaders()) {
      std::cerr << "Failed to create shaders" << std::endl;
      return false;
    }

    // Create buffers
    if (!createBuffers()) {
      std::cerr << "Failed to create buffers" << std::endl;
      return false;
    }

    // Initialize OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, width, height);

    // Set up orthographic projection
    updateProjectionMatrix();

    initialized = true;
    return true;
  }

  // === Frame Lifecycle ===

  bool beginFrame() override {
    if (!initialized)
      return false;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return true;
  }

  void endFrame() override {
    // Flush any remaining rendering commands
    glFlush();
  }

  void present() override {
    // In a real application, this would swap buffers
    // But that's handled by GLFW or the window system
  }

  void onWindowResize(int width, int height) override {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    updateProjectionMatrix();
  }

  // === Texture Management ===

  std::unique_ptr<bombfork::prong::rendering::TextureHandle> createTexture(uint32_t width, uint32_t height,
                                                                           const uint8_t* data) override {
    auto handle = std::make_unique<GLTextureHandle>();
    handle->width = width;
    handle->height = height;

    glGenTextures(1, &handle->textureId);
    glBindTexture(GL_TEXTURE_2D, handle->textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return handle;
  }

  void updateTexture(bombfork::prong::rendering::TextureHandle* texture, const uint8_t* data) override {
    auto* glTexture = static_cast<GLTextureHandle*>(texture);
    if (!glTexture || glTexture->textureId == 0)
      return;

    glBindTexture(GL_TEXTURE_2D, glTexture->textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, glTexture->width, glTexture->height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void deleteTexture(std::unique_ptr<bombfork::prong::rendering::TextureHandle> /* texture */) override {
    // Destructor will handle cleanup via RAII
  }

  // === Drawing Primitives ===

  void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) override {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void drawRect(int x, int y, int width, int height, float r, float g, float b, float a = 1.0f) override {
    if (!initialized)
      return;

    // Use rect shader
    glUseProgram(rectShaderProgram);
    glUniformMatrix4fv(rectProjLoc, 1, GL_FALSE, projectionMatrix);
    glUniform4f(rectColorLoc, r, g, b, a);

    // Update vertex data
    float x1 = static_cast<float>(x);
    float y1 = static_cast<float>(y);
    float x2 = static_cast<float>(x + width);
    float y2 = static_cast<float>(y + height);

    float vertices[] = {
      x1, y1, // Top-left
      x2, y1, // Top-right
      x1, y2, // Bottom-left
      x2, y1, // Top-right
      x2, y2, // Bottom-right
      x1, y2  // Bottom-left
    };

    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);
  }

  void drawSprite(bombfork::prong::rendering::TextureHandle* texture, int x, int y, int width = 0, int height = 0,
                  float alpha = 1.0f) override {
    if (!initialized)
      return;

    auto* glTexture = static_cast<GLTextureHandle*>(texture);
    if (!glTexture || glTexture->textureId == 0)
      return;

    if (width == 0)
      width = glTexture->width;
    if (height == 0)
      height = glTexture->height;

    // Use sprite shader
    glUseProgram(spriteShaderProgram);
    glUniformMatrix4fv(spriteProjLoc, 1, GL_FALSE, projectionMatrix);
    glUniform4f(spriteColorLoc, 1.0f, 1.0f, 1.0f, alpha);
    glUniform1i(spriteTextureLoc, 0);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glTexture->textureId);

    // Update vertex data
    float x1 = static_cast<float>(x);
    float y1 = static_cast<float>(y);
    float x2 = static_cast<float>(x + width);
    float y2 = static_cast<float>(y + height);

    float vertices[] = {
      // Position  // TexCoords
      x1, y1, 0.0f, 0.0f, // Top-left
      x2, y1, 1.0f, 0.0f, // Top-right
      x1, y2, 0.0f, 1.0f, // Bottom-left
      x2, y1, 1.0f, 0.0f, // Top-right
      x2, y2, 1.0f, 1.0f, // Bottom-right
      x1, y2, 0.0f, 1.0f  // Bottom-left
    };

    glBindVertexArray(spriteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
  }

  void drawSprites(const std::vector<SpriteDrawCmd>& sprites) override {
    // Simple implementation - just draw each sprite
    // A real implementation would batch these for better performance
    for (const auto& sprite : sprites) {
      drawSprite(sprite.texture, sprite.x, sprite.y, sprite.width, sprite.height, sprite.alpha);
    }
  }

  // === Text Rendering ===

  int drawText(const std::string& text, int x, int y, float r = 1.0f, float g = 1.0f, float b = 1.0f,
               float a = 1.0f) override {
    if (fontRenderer) {
      // Components pass y as top of text, but renderText expects baseline
      // Add ascent to convert from top to baseline
      int baseline = y + fontRenderer->getAscent();
      fontRenderer->renderText(text, static_cast<float>(x), static_cast<float>(baseline), r, g, b, a);
      return static_cast<int>(fontRenderer->measureText(text));
    }
    // Fallback: return estimated width
    return text.length() * 8;
  }

  std::pair<int, int> measureText(const std::string& text) override {
    if (fontRenderer) {
      int width = static_cast<int>(fontRenderer->measureText(text));
      int height = fontRenderer->getFontHeight();
      return {width, height};
    }
    // Fallback: return estimated dimensions
    return {static_cast<int>(text.length() * 8), 16};
  }

  // === Clipping Support ===

  void enableScissorTest(int x, int y, int width, int height) override {
    glEnable(GL_SCISSOR_TEST);
    // OpenGL scissor uses bottom-left origin, so convert from top-left
    int glY = windowHeight - (y + height);
    glScissor(x, glY, width, height);
  }

  void disableScissorTest() override { glDisable(GL_SCISSOR_TEST); }

  void flushPendingBatches() override { glFlush(); }

  // === Information ===

  std::string getName() const override { return "Modern OpenGL 3.3 Renderer"; }

  bool isInitialized() const override { return initialized; }

  uint64_t getGPUMemoryUsageMB() const override {
    return 0; // Not tracked in this simple implementation
  }

  float getFrameTimeMs() const override {
    return 0.0f; // Not tracked in this simple implementation
  }

  float getFPS() const override {
    return 0.0f; // Not tracked in this simple implementation
  }

private:
  bool loadGLFunctions() {
    // Load all OpenGL 3.3 Core functions using glfwGetProcAddress
    // This is required because system OpenGL headers may not expose modern functions

#define LOAD_GL_FUNC(name)                                                                                             \
  name = reinterpret_cast<decltype(name)>(glfwGetProcAddress(#name));                                                  \
  if (!name) {                                                                                                         \
    std::cerr << "Failed to load OpenGL function: " << #name << std::endl;                                             \
    return false;                                                                                                      \
  }

    // Buffer functions
    LOAD_GL_FUNC(glGenBuffers);
    LOAD_GL_FUNC(glDeleteBuffers);
    LOAD_GL_FUNC(glBindBuffer);
    LOAD_GL_FUNC(glBufferData);
    LOAD_GL_FUNC(glBufferSubData);

    // Vertex array functions
    LOAD_GL_FUNC(glGenVertexArrays);
    LOAD_GL_FUNC(glDeleteVertexArrays);
    LOAD_GL_FUNC(glBindVertexArray);
    LOAD_GL_FUNC(glVertexAttribPointer);
    LOAD_GL_FUNC(glEnableVertexAttribArray);

    // Shader functions
    LOAD_GL_FUNC(glCreateShader);
    LOAD_GL_FUNC(glDeleteShader);
    LOAD_GL_FUNC(glShaderSource);
    LOAD_GL_FUNC(glCompileShader);
    LOAD_GL_FUNC(glGetShaderiv);
    LOAD_GL_FUNC(glGetShaderInfoLog);

    // Program functions
    LOAD_GL_FUNC(glCreateProgram);
    LOAD_GL_FUNC(glDeleteProgram);
    LOAD_GL_FUNC(glAttachShader);
    LOAD_GL_FUNC(glLinkProgram);
    LOAD_GL_FUNC(glGetProgramiv);
    LOAD_GL_FUNC(glGetProgramInfoLog);
    LOAD_GL_FUNC(glUseProgram);

    // Uniform functions
    LOAD_GL_FUNC(glGetUniformLocation);
    LOAD_GL_FUNC(glUniformMatrix4fv);
    LOAD_GL_FUNC(glUniform4f);
    LOAD_GL_FUNC(glUniform1i);

#undef LOAD_GL_FUNC

    std::cout << "Successfully loaded all OpenGL 3.3 Core functions" << std::endl;
    return true;
  }

  void updateProjectionMatrix() {
    // Create orthographic projection matrix (top-left origin)
    // This is a simple 2D orthographic projection
    float left = 0.0f;
    float right = static_cast<float>(windowWidth);
    float top = 0.0f;
    float bottom = static_cast<float>(windowHeight);
    float near = -1.0f;
    float far = 1.0f;

    // Initialize to identity
    std::memset(projectionMatrix, 0, sizeof(projectionMatrix));
    projectionMatrix[0] = 2.0f / (right - left);
    projectionMatrix[5] = 2.0f / (top - bottom);
    projectionMatrix[10] = -2.0f / (far - near);
    projectionMatrix[12] = -(right + left) / (right - left);
    projectionMatrix[13] = -(top + bottom) / (top - bottom);
    projectionMatrix[14] = -(far + near) / (far - near);
    projectionMatrix[15] = 1.0f;
  }

  bool createShaders() {
    // Vertex shader for rectangles
    const char* rectVertexShader = R"(
      #version 330 core
      layout (location = 0) in vec2 aPos;

      uniform mat4 uProjection;

      void main() {
        gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
      }
    )";

    // Fragment shader for rectangles
    const char* rectFragmentShader = R"(
      #version 330 core
      out vec4 FragColor;

      uniform vec4 uColor;

      void main() {
        FragColor = uColor;
      }
    )";

    // Create rect shader program
    rectShaderProgram = createShaderProgram(rectVertexShader, rectFragmentShader);
    if (rectShaderProgram == 0)
      return false;

    rectProjLoc = glGetUniformLocation(rectShaderProgram, "uProjection");
    rectColorLoc = glGetUniformLocation(rectShaderProgram, "uColor");

    // Vertex shader for sprites
    const char* spriteVertexShader = R"(
      #version 330 core
      layout (location = 0) in vec2 aPos;
      layout (location = 1) in vec2 aTexCoord;

      uniform mat4 uProjection;

      out vec2 TexCoord;

      void main() {
        gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
      }
    )";

    // Fragment shader for sprites
    const char* spriteFragmentShader = R"(
      #version 330 core
      out vec4 FragColor;

      in vec2 TexCoord;

      uniform sampler2D uTexture;
      uniform vec4 uColor;

      void main() {
        FragColor = texture(uTexture, TexCoord) * uColor;
      }
    )";

    // Create sprite shader program
    spriteShaderProgram = createShaderProgram(spriteVertexShader, spriteFragmentShader);
    if (spriteShaderProgram == 0)
      return false;

    spriteProjLoc = glGetUniformLocation(spriteShaderProgram, "uProjection");
    spriteTextureLoc = glGetUniformLocation(spriteShaderProgram, "uTexture");
    spriteColorLoc = glGetUniformLocation(spriteShaderProgram, "uColor");

    return true;
  }

  bool createBuffers() {
    // Create VAO and VBO for rectangles
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);

    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Create VAO and VBO for sprites
    glGenVertexArrays(1, &spriteVAO);
    glGenBuffers(1, &spriteVBO);

    glBindVertexArray(spriteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
  }

  GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0)
      return 0;

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0) {
      glDeleteShader(vertexShader);
      return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[512];
      glGetProgramInfoLog(program, 512, nullptr, infoLog);
      std::cerr << "Shader program linking failed: " << infoLog << std::endl;
      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
      glDeleteProgram(program);
      return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
  }

  GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[512];
      glGetShaderInfoLog(shader, 512, nullptr, infoLog);
      std::cerr << "Shader compilation failed: " << infoLog << std::endl;
      glDeleteShader(shader);
      return 0;
    }

    return shader;
  }

  void cleanup() {
    if (rectShaderProgram != 0) {
      glDeleteProgram(rectShaderProgram);
      rectShaderProgram = 0;
    }

    if (spriteShaderProgram != 0) {
      glDeleteProgram(spriteShaderProgram);
      spriteShaderProgram = 0;
    }

    if (rectVAO != 0) {
      glDeleteVertexArrays(1, &rectVAO);
      rectVAO = 0;
    }

    if (rectVBO != 0) {
      glDeleteBuffers(1, &rectVBO);
      rectVBO = 0;
    }

    if (spriteVAO != 0) {
      glDeleteVertexArrays(1, &spriteVAO);
      spriteVAO = 0;
    }

    if (spriteVBO != 0) {
      glDeleteBuffers(1, &spriteVBO);
      spriteVBO = 0;
    }
  }
};

} // namespace bombfork::prong::examples
