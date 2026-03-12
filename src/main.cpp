#include "Application.h"

#include <iostream>

i32 main() {
    Application app;
    
    app.initWindow(800, 600, "Vulkan App");
    app.initVulkan();
    app.run();
}