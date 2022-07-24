#include <spdlog/spdlog.h>
#include "VulkanApp.h"

int main() {
    try {
        VulkanApp app;
        app.run();
    }
    catch (const std::exception& e) {
        spdlog::critical("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}