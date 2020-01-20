
#include "demo/demo_shell/app/demo_shell_content_main_delegate.h"
#include "content/public/app/content_main.h"

int main(int argc, const char** argv){
    content::DemoShellContentMainDelegate demo_shell_content_delegate;
    content::ContentMainParams content_main_params(&demo_shell_content_delegate);
    content_main_params.argc = argc;
    content_main_params.argv = argv;
    content::ContentMain(content_main_params);
}