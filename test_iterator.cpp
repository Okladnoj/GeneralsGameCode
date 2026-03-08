#include <filesystem>
#include <iostream>
#include <string>

int main() {
    std::string path = "/Users/okji/dev/games/Command and Conquer - Generals/Command and Conquer Generals";
    auto searchExt = std::filesystem::path("*.big").extension();
    std::cout << "Extension parsing: " << searchExt.string() << std::endl;
    std::error_code ec;
    auto iter = std::filesystem::directory_iterator(path, ec);
    if (ec) {
        std::cout << "Error: " << ec.message() << std::endl;
        return 1;
    }
    int count = 0;
    while (iter != std::filesystem::directory_iterator()) {
        std::string ext = iter->path().extension().string();
        if (strcasecmp(ext.c_str(), searchExt.string().c_str()) == 0) {
            count++;
        }
        iter++;
    }
    std::cout << "Count: " << count << std::endl;
    return 0;
}
