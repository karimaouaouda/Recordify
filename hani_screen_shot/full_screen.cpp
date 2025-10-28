#include <windows.h>
#include <iostream>

int main() {
    DEVMODE dm;
    ZeroMemory(&dm, sizeof(dm)); // Initialize the structure
    dm.dmSize = sizeof(dm);

    // Retrieve current display settings for the primary display
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
        std::cout << "Current Display Settings:" << std::endl;
        std::cout << "  Resolution: " << dm.dmPelsWidth << "x" << dm.dmPelsHeight << std::endl;
        std::cout << "  Bits per pixel: " << dm.dmBitsPerPel << std::endl;
        std::cout << "  Display frequency: " << dm.dmDisplayFrequency << " Hz" << std::endl;
        // You can access other members of the DEVMODE structure as needed
    } else {
        std::cerr << "Failed to retrieve display settings." << std::endl;
    }

    return 0;
}