#include "TextureManager.h"

#include <unordered_map>

#include "render/Texture.h"

#include "resources/TextureAtlas.h"
#include "render/VulkanEnums.h"


namespace resources::textueManager {
    std::unordered_map<std::string, rk::Texture*> textures;

    void start() {
        resources::TextureAtlas atlas{PROJECT_ROOT_PATH"/assets/textures"};

        auto tex = new rk::Texture();
        tex->create(atlas, rk::SamplerFilter::NEAREST, rk::SamplerMode::REPEAT);

        textures.emplace("sla", tex);
    }

    const rk::Texture* get(const char* name) {
        auto it = textures.find(name);

        if (it == textures.end())
            return nullptr;

        return it->second;
    }
}