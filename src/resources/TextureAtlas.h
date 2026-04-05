#pragma once

#include <string>
#include <vector>

#include "math/Vec4.h"


namespace resources {
    class TextureAtlas {
        // fwd
        class Node;
        struct ImageRect;
        struct ImageOutOfRangeCallback;

    public:
        TextureAtlas(const std::string& directory);
        ~TextureAtlas();

        const u8* data() const { return m_atlasData.data(); }
        i32 width() const { return m_width; }
        i32 height() const { return m_height; }

        std::vector<std::pair<std::string, Vec4>> textures;

    private:
        void insertImages(const std::string& filePath);
        void addImage(const u8* data, const ImageRect& rect, std::string name);
        void addImageData(const u8* data, const ImageRect& rect) const;

        std::vector<u8> m_atlasData;
        Node* m_root = nullptr;
        i32 m_width = 0;
        i32 m_height = 0;
    };
}
