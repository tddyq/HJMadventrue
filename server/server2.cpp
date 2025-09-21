#include"CServer.h"
#include"RuleModule.h"
int main(int argc, char** argv) {
	RuleModule ruleModule(2);
	CServer server(&ruleModule);

	server.setDomainAndPost("0.0.0.0", 25565);

	server.connect();

	system("pause");
	return 0;
}