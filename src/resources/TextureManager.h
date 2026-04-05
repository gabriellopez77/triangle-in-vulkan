#pragma once


// fwd
namespace rk {
    class Texture;
}

namespace resources::textueManager {
    extern void start();
    extern const rk::Texture* get(const char* name);
}