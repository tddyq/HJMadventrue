#pragma once
#include"HttplibNetworkAdapter.h"
class CServer
{
public:
	CServer(int size) :server(size) {
		getStrFromText();

		server.login();
		server.getGameText(str_text);
		
	}
	~CServer() = default;

	void setDomainAndPost(std::string domain, int post) {
		this->domain = domain;
		this->post = post;
	}
	
	void connect() {
		server.connect(domain, post);
	}
private:
	void getStrFromText() {
		std::ifstream file("text.txt");

		if (!file.good()) {
			MessageBox(nullptr, L"�޷����ı��ļ� text.txt", L"����ʧ��", MB_ICONERROR | MB_OK);

			exit(-1);
		}

		std::stringstream str_stream;
		str_stream << file.rdbuf();
		str_text = str_stream.str();

		file.close();
	}

private:
	HttplibNetworkAdapter server;
	std::string str_text;            //�ı�����
	std::string domain;              //����
	int post;                        //�˿�
};

