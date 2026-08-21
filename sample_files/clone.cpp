#include <iostream>
int main() {
    int count;
    std::cin >> count;
    for (int j = 0; j < count; ++j) {
        std::cout << j * j << std::endl;
    }
    return 0;
}
