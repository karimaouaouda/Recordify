#ifndef RECORDIFY_CORE_APP_H
#define RECORDIFY_CORE_APP_H

#include <memory>

namespace recordify {
namespace core {

class Application {
public:
    Application();
    ~Application();
    
    int run();
    void initialize();
    void shutdown();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace core
} // namespace recordify

#endif // RECORDIFY_CORE_APP_H
