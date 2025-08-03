#include"CClient.h"
#include <direct.h>
int main(int argc, char** argv) {
    //调试
    char cwd[1024];
    _getcwd(cwd, sizeof(cwd));
    std::cout << "当前工作目录: " << cwd << std::endl;

    CClient client(2);
    client.mainLoop();
    return 0;
}
