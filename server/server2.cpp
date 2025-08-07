#include"CServer.h"

int main(int argc, char** argv) {
	CServer server(10);

	server.setDomainAndPost("0.0.0.0", 25565);

	server.connect();

	system("pause");
	return 0;
}