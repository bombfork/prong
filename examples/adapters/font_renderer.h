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
   * @brief Get or create a glyph texture for a Unicode code point
   * @param codepoint Unicode code point to render
   * @return Glyph information
   */
  const Glyph& getGlyph(uint32_t codepoint) {
    // Check if glyph already cached
    auto it = glyphs_.find(codepoint);
    if (it != glyphs_.end()) {
      return it->second;
    }

    // Render new glyph
    Glyph glyph = renderGlyph(codepoint);
    glyphs_[codepoint] = glyph;
    return glyphs_[codepoint];
  }

  /**
   * @brief Render text at specified position
   * @param text Text to render (UTF-8 encoded)
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

    // Decode UTF-8 and render each code point
    std::vector<uint32_t> codepoints = decodeUTF8(text);
    for (uint32_t codepoint : codepoints) {
      const Glyph& glyph = getGlyph(codepoint);

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
   * @param text Text to measure (UTF-8 encoded)
   * @return Width in pixels
   */
  float measureText(const std::string& text) {
    float width = 0.0f;
    std::vector<uint32_t> codepoints = decodeUTF8(text);
    for (uint32_t codepoint : codepoints) {
      const Glyph& glyph = getGlyph(codepoint);
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
   * @brief Decode UTF-8 string into Unicode code points
   * @param text UTF-8 encoded string
   * @return Vector of Unicode code points
   */
  std::vector<uint32_t> decodeUTF8(const std::string& text) const {
    std::vector<uint32_t> codepoints;
    size_t i = 0;
    while (i < text.length()) {
      uint32_t codepoint = 0;
      uint8_t byte = static_cast<uint8_t>(text[i]);

      if (byte < 0x80) {
        // 1-byte sequence (ASCII)
        codepoint = byte;
        i += 1;
      } else if ((byte & 0xE0) == 0xC0) {
        // 2-byte sequence
        if (i + 1 < text.length()) {
          codepoint = ((byte & 0x1F) << 6) | (static_cast<uint8_t>(text[i + 1]) & 0x3F);
          i += 2;
        } else {
          i += 1; // Skip invalid byte
        }
      } else if ((byte & 0xF0) == 0xE0) {
        // 3-byte sequence
        if (i + 2 < text.length()) {
          codepoint = ((byte & 0x0F) << 12) | ((static_cast<uint8_t>(text[i + 1]) & 0x3F) << 6) |
                      (static_cast<uint8_t>(text[i + 2]) & 0x3F);
          i += 3;
        } else {
          i += 1; // Skip invalid byte
        }
      } else if ((byte & 0xF8) == 0xF0) {
        // 4-byte sequence
        if (i + 3 < text.length()) {
          codepoint = ((byte & 0x07) << 18) | ((static_cast<uint8_t>(text[i + 1]) & 0x3F) << 12) |
                      ((static_cast<uint8_t>(text[i + 2]) & 0x3F) << 6) | (static_cast<uint8_t>(text[i + 3]) & 0x3F);
          i += 4;
        } else {
          i += 1; // Skip invalid byte
        }
      } else {
        // Invalid UTF-8 sequence, skip
        i += 1;
      }

      if (codepoint > 0) {
        codepoints.push_back(codepoint);
      }
    }
    return codepoints;
  }

  /**
   * @brief Render a single glyph to an OpenGL texture
   */
  Glyph renderGlyph(uint32_t codepoint) {
    Glyph glyph{};

    // Get glyph metrics
    int advance, leftSideBearing;
    stbtt_GetCodepointHMetrics(&fontInfo_, static_cast<int>(codepoint), &advance, &leftSideBearing);

    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(&fontInfo_, static_cast<int>(codepoint), scale_, scale_, &x0, &y0, &x1, &y1);

    glyph.width = x1 - x0;
    glyph.height = y1 - y0;
    glyph.bearingX = x0;
    glyph.bearingY = -y0;
    glyph.advance = static_cast<int>(advance * scale_);

    // Render glyph to bitmap
    std::vector<uint8_t> bitmap(glyph.width * glyph.height);
    if (glyph.width > 0 && glyph.height > 0) {
      stbtt_MakeCodepointBitmap(&fontInfo_, bitmap.data(), glyph.width, glyph.height, glyph.width, scale_, scale_,
                                static_cast<int>(codepoint));
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
  std::unordered_map<uint32_t, Glyph> glyphs_;
  float fontSize_ = 0.0f;
  float scale_ = 0.0f;
  int ascent_ = 0;
  int descent_ = 0;
  int lineGap_ = 0;
};

} // namespace bombfork::prong::examples

#endif // BOMBFORK_PRONG_EXAMPLES_FONT_RENDERER_H
