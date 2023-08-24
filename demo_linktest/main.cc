
__attribute__((visibility("default"))) void aaa();
__attribute__((visibility("default"))) void bbb();

int main() {
    aaa();
    bbb();
    return 0;
}