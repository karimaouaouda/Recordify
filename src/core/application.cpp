#include "core/application.h"
#include <iostream>

namespace recordify {
namespace core {

class Application::Impl {
public:
    bool initialized = false;
};

Application::Application() : pImpl(std::make_unique<Impl>()) {
}

Application::~Application() = default;

void Application::initialize() {
    std::cout << "Initializing Recordify application..." << std::endl;
    pImpl->initialized = true;
}

void Application::shutdown() {
    std::cout << "Shutting down Recordify application..." << std::endl;
    pImpl->initialized = false;
}

int Application::run() {
    if (!pImpl->initialized) {
        initialize();
    }
    
    std::cout << "Recordify application running..." << std::endl;
    // Main application logic here
    
    shutdown();
    return 0;
}

} // namespace core
} // namespace recordify
