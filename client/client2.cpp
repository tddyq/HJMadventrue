#include"CClient.h"
#include <direct.h>
int main(int argc, char** argv) {
    //����
    char cwd[1024];
    _getcwd(cwd, sizeof(cwd));
    std::cout << "��ǰ����Ŀ¼: " << cwd << std::endl;

    CClient client(2);
    client.mainLoop();
    return 0;
}
