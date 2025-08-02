#include"../thirdparty/httplib.h"
#include <thread>
#include <mutex>
#include <vector>
std::mutex mtx;  // ����һ��������

std::string str_text;            //�ı�����

int progress_1 = -1;             //���1����
int progress_2 = -1;             //���2����	
///////////////////��չ
std::vector<int> progress_player;

int main(int argc, char** argv) {
	if(argc != 2){
		MessageBox(nullptr, L"�������������", L"����ʧ��", MB_ICONERROR | MB_OK);
		return -1;
	}

	std::ifstream file("text.txt");

	if (!file.good()) {
		MessageBox(nullptr, L"�޷����ı��ļ� text.txt", L"����ʧ��", MB_ICONERROR | MB_OK);

		return -1;
	}

	std::stringstream str_stream;
	str_stream << file.rdbuf();
	str_text = str_stream.str();

	file.close();

	httplib::Server server;

	

	server.Post("/login", [&](const httplib::Request& req, httplib::Response& res) {
		std::cout << "login" << std::endl;
		std::lock_guard<std::mutex> lock(mtx);  
		
		if (progress_1 >= 0 && progress_2 >= 0) {
			res.set_content("-1", "text/plain");
			return;
		}
		res.set_content(progress_1 >= 0 ? "2" : "1", "text/plain");
		(progress_1 >= 0) ? (progress_2 = 0) : (progress_1 = 0);       // ������ҽ���
		});
	server. Post("/query_text", [&](const httplib :: Request & req, httplib::Response& res) {

		res.set_content(str_text, "text/plain");
		});
	server.Post("/update_1", [&](const httplib::Request& req, httplib::Response& res) {
		std::lock_guard<std::mutex> lock(mtx);

		progress_1 = std::stoi(req.body);

		res.set_content(std::to_string(progress_2), "text/plain");

		});
	server.Post("/update_2", [&](const httplib::Request& req, httplib::Response& res) {
		std::lock_guard<std::mutex> lock(mtx);

		progress_2 = std::stoi(req.body);

		res.set_content(std::to_string(progress_1), "text/plain");

		});

	std::cout << "�����������������˿�: 25565" << std::endl;

	// ���˿�ռ��
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup ʧ��" << std::endl;
		return -1;
	}

	SOCKET test_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(25565);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(test_sock, (sockaddr*)&addr, sizeof(addr))){
		std::cerr << "�˿� 25565 �ѱ�ռ��! �������: " << WSAGetLastError() << std::endl;
		MessageBoxA(nullptr, "�˿� 25565 �ѱ�ռ��", "����������", MB_ICONERROR | MB_OK);
		closesocket(test_sock);
		WSACleanup();
		return -2;
	}
	closesocket(test_sock);
		WSACleanup();

		std::cout << "�˿� 25565 ���ã���ʼ����..." << std::endl;

		try {
		server.listen("0.0.0.0", 25565);
		std::cout << "���������� 0.0.0.0:25565 ������" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "����������ʧ��: " << e.what() << std::endl;
		MessageBoxA(nullptr, e.what(), "����������", MB_ICONERROR | MB_OK);
		return -3;
	}

	system("pause");
	return 0;
}

