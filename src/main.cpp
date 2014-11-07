#include <iostream>

#include "ApplicationContainer.h"
#include "TestApplication.h"

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

int main(int argc, char *argv[])
{
    ApplicationContainer container(argc, argv);

    container.registerApplication(make_unique<Application>());

    try {
        return container.run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown exeption\n";
    }
    return 1;
}