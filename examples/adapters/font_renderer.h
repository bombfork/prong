/**
 * @file font_renderer.h
 * @brief TrueType font rendering using STB truetype and OpenGL
 */

#ifndef BOMBFORK_PRONG_EXAMPLES_FONT_RENDERER_H
#define BOMBFORK_PRONG_EXAMPLES_FONT_RENDERER_H

#include <GLFW/glfw3.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <stb_truetype.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace bombfork::prong::examples {

/**
 * @brief Glyph information for rendered characters
 */
struct Glyph {
  GLuint textureID; // OpenGL texture ID for this glyph
  int width;        // Width of glyph in pixels
  int height;       // Height of glyph in pixels
  int bearingX;     // Offset from baseline to left of glyph
  int bearingY;     // Offset from baseline to top of glyph
  int advance;      // Horizontal distance to next glyph
};

/**
 * @brief TrueType font renderer using STB truetype
 */
class FontRenderer {
public:
  /**
   * @brief Load a TrueType font from file
   * @param fontPath Path to the .ttf font file
   * @param fontSize Desired font size in pixels
   * @return true if font loaded successfully
   */
  bool loadFont(const std::string& fontPath, float fontSize) {
    // Read font file into memory
    std::ifstream fontFile(fontPath, std::ios::binary | std::ios::ate);
    if (!fontFile.is_open()) {
      std::cerr << "Failed to open font file: " << fontPath << std::endl;
      return false;
    }

    std::streamsize size = fontFile.tellg();
    fontFile.seekg(0, std::ios::beg);

    fontBuffer_.resize(size);
    if (!fontFile.read(reinterpret_cast<char*>(fontBuffer_.data()), size)) {
      std::cerr << "Failed to read font file: " << fontPath << std::endl;
      return false;
    }

    // Initialize STB truetype
    if (!stbtt_InitFont(&fontInfo_, fontBuffer_.data(), 0)) {
      std::cerr << "Failed to initialize font" << std::endl;
      return false;
    }

    fontSize_ = fontSize;
    scale_ = stbtt_ScaleForPixelHeight(&fontInfo_, fontSize_);

    // Get vertical metrics
    stbtt_GetFontVMetrics(&fontInfo_, &ascent_, &descent_, &lineGap_);
    ascent_ = static_cast<int>(ascent_ * scale_);
    descent_ = static_cast<int>(descent_ * scale_);

    std::cout << "Font loaded: " << fontPath << " (size: " << fontSize << "px)" << std::endl;
    return true;
  }

  /**
   * @brief Get or create a glyph texture for a character
   * @param ch Character to render
   * @return Glyph information
   */
  const Glyph& getGlyph(char ch) {
    // Check if glyph already cached
    auto it = glyphs_.find(ch);
    if (it != glyphs_.end()) {
      return it->second;
    }

    // Render new glyph
    Glyph glyph = renderGlyph(ch);
    glyphs_[ch] = glyph;
    return glyphs_[ch];
  }

  /**
   * @brief Render text at specified position
   * @param text Text to render
   * @param x X position (screen coordinates)
   * @param y Y position (screen coordinates, baseline)
   * @param r Red color component (0-1)
   * @param g Green color component (0-1)
   * @param b Blue color component (0-1)
   * @param a Alpha component (0-1)
   */
  void renderText(const std::string& text, float x, float y, float r = 1.0f, float g = 1.0f, float b = 1.0f,
                  float a = 1.0f) {
    // Save OpenGL state
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);

    // Set up for legacy rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set texture environment to modulate texture with color
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f(r, g, b, a);

    // Set up matrices for legacy rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Get viewport to set up proper orthographic projection
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int viewportWidth = viewport[2];
    int viewportHeight = viewport[3];

    // Orthographic projection: (0,0) at top-left, matching the modern renderer
    glOrtho(0, viewportWidth, viewportHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float cursorX = x;
    float cursorY = y;

    for (char ch : text) {
      const Glyph& glyph = getGlyph(ch);

      float xpos = cursorX + glyph.bearingX;
      // For top-left origin: y is the baseline, bearingY is distance from baseline to top
      // We need to subtract bearingY to go up from baseline to top of glyph
      float ypos = cursorY - glyph.bearingY;

      // Render glyph texture
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, glyph.textureID);

      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(xpos, ypos);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(xpos + glyph.width, ypos);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(xpos + glyph.width, ypos + glyph.height);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(xpos, ypos + glyph.height);
      glEnd();

      glBindTexture(GL_TEXTURE_2D, 0);

      // Advance cursor
      cursorX += glyph.advance;
    }

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Restore OpenGL state
    glDisable(GL_TEXTURE_2D);
    if (!blendEnabled)
      glDisable(GL_BLEND);
    if (!textureEnabled)
      glDisable(GL_TEXTURE_2D);
  }

  /**
   * @brief Measure the width of rendered text
   * @param text Text to measure
   * @return Width in pixels
   */
  float measureText(const std::string& text) {
    float width = 0.0f;
    for (char ch : text) {
      const Glyph& glyph = getGlyph(ch);
      width += glyph.advance;
    }
    return width;
  }

  /**
   * @brief Get font height in pixels
   */
  int getFontHeight() const { return ascent_ - descent_; }

  /**
   * @brief Get font ascent (distance from baseline to top)
   */
  int getAscent() const { return ascent_; }

  /**
   * @brief Clean up OpenGL textures
   */
  ~FontRenderer() {
    for (auto& pair : glyphs_) {
      glDeleteTextures(1, &pair.second.textureID);
    }
  }

private:
  /**
   * @brief Render a single glyph to an OpenGL texture
   */
  Glyph renderGlyph(char ch) {
    Glyph glyph{};

    // Get glyph metrics
    int advance, leftSideBearing;
    stbtt_GetCodepointHMetrics(&fontInfo_, ch, &advance, &leftSideBearing);

    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(&fontInfo_, ch, scale_, scale_, &x0, &y0, &x1, &y1);

    glyph.width = x1 - x0;
    glyph.height = y1 - y0;
    glyph.bearingX = x0;
    glyph.bearingY = -y0;
    glyph.advance = static_cast<int>(advance * scale_);

    // Render glyph to bitmap
    std::vector<uint8_t> bitmap(glyph.width * glyph.height);
    if (glyph.width > 0 && glyph.height > 0) {
      stbtt_MakeCodepointBitmap(&fontInfo_, bitmap.data(), glyph.width, glyph.height, glyph.width, scale_, scale_, ch);
    }

    // Convert single-channel bitmap to RGBA format
    std::vector<uint8_t> rgbaBitmap(glyph.width * glyph.height * 4);
    for (int i = 0; i < glyph.width * glyph.height; ++i) {
      rgbaBitmap[i * 4 + 0] = 255;       // R
      rgbaBitmap[i * 4 + 1] = 255;       // G
      rgbaBitmap[i * 4 + 2] = 255;       // B
      rgbaBitmap[i * 4 + 3] = bitmap[i]; // A (from glyph bitmap)
    }

    // Create OpenGL texture
    glGenTextures(1, &glyph.textureID);
    glBindTexture(GL_TEXTURE_2D, glyph.textureID);

    // Upload as RGBA texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glyph.width, glyph.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBitmap.data());

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return glyph;
  }

  stbtt_fontinfo fontInfo_;
  std::vector<uint8_t> fontBuffer_;
  std::unordered_map<char, Glyph> glyphs_;
  float fontSize_ = 0.0f;
  float scale_ = 0.0f;
  int ascent_ = 0;
  int descent_ = 0;
  int lineGap_ = 0;
};

} // namespace bombfork::prong::examples

#endif // BOMBFORK_PRONG_EXAMPLES_FONT_RENDERER_H
