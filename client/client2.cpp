#include"CClient.h"
#include <direct.h>
int main(int argc, char** argv) {
    //ต๗สิ
    char cwd[1024];
    _getcwd(cwd, sizeof(cwd));


    CClient client(2);
    client.init();
    client.mainLoop();
    return 0;
}
