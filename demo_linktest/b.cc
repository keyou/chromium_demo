#include <iostream>

__attribute__((visibility("default"))) void aaa();

__attribute__((visibility("default"))) void bbb() {
    std::cout << "bbbb";
    aaa();
}