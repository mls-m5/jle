// Copyright (c) 2023. Johan Lind

#pragma once

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "jleCamera.h"
#include "jleResourceInterface.h"
#include "jleShader.h"

class jleFontData;

class jleFont : public jleResourceInterface
{

public:
    explicit jleFont(const jlePath& path);

    jleFont() = default;

    ~jleFont() override;

    void addFontSizePixels(uint32_t sizePixels);

    jleLoadFromFileSuccessCode loadFromFile(const jlePath &path) override;

    // TODO: Move rendering to Text Rendering engine subsystem
    // TODO: Add scale as param
    void renderText(const std::string &text,
                    uint32_t fontSize,
                    float x,
                    float y,
                    float depth,
                    glm::vec3 color,
                    const jleCamera &camera);

    static void renderTargetDimensions(int width,
                                       int height,
                                       const jleCamera &camera);

    static inline glm::mat4 sProj;

private:
    class jleFontSize {
    public:
        struct jleFontCharacter {
            unsigned int _textureID;
            glm::ivec2 _size;
            glm::ivec2 _bearing;
            unsigned int _advance;
        };

        std::map<char, jleFontCharacter> _characters;
    };
    FT_Face _face{};
    bool _fontLoaded = false;
    std::unordered_map<uint32_t, jleFontSize> _fontSizeLookup;

    friend class jleCore;
    friend class jleFontData;
};

// Handle data shared between font instances
struct jleFontData {
    jleFontData();
    ~jleFontData();
    unsigned int vao = 0, vbo = 0;
    std::unique_ptr<jleShader> shader;

    FT_Library freeTypeLibrary;

    static inline jleFontData *data = nullptr;
};
