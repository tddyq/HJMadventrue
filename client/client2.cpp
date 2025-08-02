#include"CClient.h"
int main(int argc, char** argv) {
    CClient client(2);
    client.mainLoop();
    return 0;
}
