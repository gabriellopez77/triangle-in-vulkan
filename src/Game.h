#pragma once

#include "render/DescriptorSet.h"
#include "render/GraphicsPipeline.h"
#include "render/VertexBuffer.h"
#include "world/player/Player.h"


// fwd
class Application;

class Game {
public:
    void start(Application* application);
    void update(float dt);
    void render();
    void resize(int width, int height);

    Player player;

private:
    rk::VertexBuffer vertexBuffer1;
    rk::VertexBuffer vertexBuffer2;
    rk::GraphicsPipeline pipeline;
    rk::DescriptorSet descriptorSet;

    Application* m_application = nullptr;

};