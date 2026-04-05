#include "TextureAtlas.h"

#include <filesystem>
#include <cassert>
#include <cstring>
#include <chrono>

#include <stb_image.h>

#include "math/Math.h"

#define PNG_BYTES_COUNT 4


namespace fs = std::filesystem;

struct resources::TextureAtlas::ImageRect {
    i32 x = 0;
    i32 y = 0;
    i32 width = 0;
    i32 height = 0;
};

struct ImageResult {
    fs::path imageName;
    u8* data = nullptr;
    i32 width = 0;
    i32 height = 0;
};

struct resources::TextureAtlas::ImageOutOfRangeCallback {
    ImageRect NodeRect;
    int ImageWidth = 0;
    int ImageHeight = 0;
};

class resources::TextureAtlas::Node {
public:
    Node(i32 x, i32 y, i32 width, i32 height) {
        rect = {x, y, width, height};
    }
    ~Node() {
        delete left;
        delete right;
    }

    Node* insert(ImageOutOfRangeCallback* callback, i32 width, i32 height) {
        // Se nao e folha
        if (left != nullptr && right != nullptr)
        {
            auto node = left->insert(callback, width, height);
            return node == nullptr ? right->insert(callback, width, height) : node;
        }

        // ja ocupado
        if (used) return nullptr;

        // Nao cabe
        if (width > rect.width || height > rect.height) {
            *callback = { rect, width, height };
            return nullptr;
        }

        // perfeito
        if (width == rect.width && height == rect.height)
        {
            used = true;
            return this;
        }

        // split
        i32 dw = rect.width - width;
        i32 dh = rect.height - height;

        if (dw > dh)
        {
            left = new Node(rect.x, rect.y, width, rect.height);
            right = new Node(rect.x + width, rect.y, rect.width - width, rect.height);
        }
        else
        {
            left = new Node(rect.x, rect.y, rect.width, height);
            right = new Node(rect.x, rect.y + height, rect.width, rect.height - height);
        }

        return left->insert(callback, width, height);
    }

    ImageRect rect{};
    Node* left = nullptr;
    Node* right = nullptr;
    bool used = false;
};


resources::TextureAtlas::TextureAtlas(const std::string& directory) {
    m_width = 32;
    m_height = 32;

    m_root = new Node(0, 0, m_width, m_height);

    // inserts all directory files in atlas
    insertImages(directory);
}

resources::TextureAtlas::~TextureAtlas() { delete m_root; }

void resources::TextureAtlas::insertImages(const std::string& directory) {
    std::vector<fs::path> filesPaths;
    std::vector<ImageRect> imagesLocation;
    std::vector<ImageResult> images;

    // get all png files in directory
    for (const auto& entry : fs::directory_iterator(directory)) {
        auto& filePath = entry.path();

        // if entry is not file or extension is not png then continue
        if (!fs::is_regular_file(filePath) || filePath.extension() != ".png")
            continue;

        filesPaths.push_back(filePath);
    }

    // reserve texture
    textures.reserve(textures.size() + filesPaths.size());
    imagesLocation.reserve(filesPaths.size());
    images.reserve(filesPaths.size());

    // read all images data in directory
    for (const auto& filePath : filesPaths) {
        auto path = filePath.string();

        ImageResult result;
        result.imageName = filePath.filename();
        result.data = stbi_load(path.c_str(), &result.width, &result.height, nullptr, 0);

        images.push_back(result);
    }

    for (int i = 0; i < images.size(); ) {
        ImageOutOfRangeCallback callback;

        auto& image = images[i];
        auto node = m_root->insert(&callback, image.width, image.height);

        // if null then node is too small to contains image, then we need to create other node
        if (node == nullptr) {
            int finalX = callback.NodeRect.x + callback.ImageWidth;
            int finalY = callback.NodeRect.y + callback.ImageHeight;

            // grow in x axis
            if (finalX >= m_width) {
                i32 newWidth = math::alignUp(finalX == m_width ? finalX + 1 : finalX, m_width);
                m_width = newWidth;
            }
            // grow in y axis
            else if (finalY >= m_height) {
                i32 newHeight = math::alignUp(finalY == m_height ? finalY + 1 : finalY, m_height);
                m_height = newHeight;
            }

            delete m_root;
            m_root = new Node(0, 0, m_width, m_height);
            imagesLocation.clear();
            i = 0;

        }
        else {
            imagesLocation.push_back(node->rect);
            i++;
        }
    }

    
    // create atlas data
    m_atlasData = std::vector<u8>(m_width * m_height * PNG_BYTES_COUNT);

    for (int i = 0; i < images.size(); i++) {
        addImage(images[i].data, imagesLocation[i], images[i].imageName.string());
    }
}

void resources::TextureAtlas::addImage(const u8* data, const ImageRect& rect, std::string name) {
    // insert image data to atlas data
    addImageData(data, rect);

    Vec4 coords = {
        rect.x / (f32)rect.width,
        rect.y / (f32)rect.height,
        rect.width / (f32)rect.width,
        rect.height / (f32)rect.height,
    };

    textures.push_back(std::make_pair(name, coords));
}

void resources::TextureAtlas::addImageData(const u8* data, const ImageRect& rect) const {
    const i32 atlasBytesCount = m_width * m_height * PNG_BYTES_COUNT;
    const i32 imageBytesCount = rect.width * rect.height * PNG_BYTES_COUNT;

    u32* const atlasDataAsPngData = (u32*)m_atlasData.data();
    const u32* const imageDataAsPngData = (u32*)data;

    // copy image to atlas
    for (i32 x = 0; x < rect.width; x++)
    for (i32 y = 0; y < rect.height; y++) {
        i32 imageIndex = x + rect.width * y;
        i32 atlasIndex = (rect.x + x) + m_width * (rect.y + y);

        // return if index out of range
        if (imageIndex >= imageBytesCount || atlasIndex >= atlasBytesCount)
            assert(false && "Index out of Range");

        // copy pixel to atlasData
        atlasDataAsPngData[atlasIndex] = imageDataAsPngData[imageIndex];
    }
}
