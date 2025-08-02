#include"../thirdparty/httplib.h"
#include <thread>
#include <mutex>
#include <vector>
std::mutex mtx;  // 创建一个互斥锁

std::string str_text;            //文本内容

int progress_1 = -1;             //玩家1进度
int progress_2 = -1;             //玩家2进度	
///////////////////扩展
std::vector<int> progress_player;

int main(int argc, char** argv) {
	if(argc != 2){
		MessageBox(nullptr, L"参与玩家数过少", L"启动失败", MB_ICONERROR | MB_OK);
		return -1;
	}

	std::ifstream file("text.txt");

	if (!file.good()) {
		MessageBox(nullptr, L"无法打开文本文件 text.txt", L"启动失败", MB_ICONERROR | MB_OK);

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
		(progress_1 >= 0) ? (progress_2 = 0) : (progress_1 = 0);       // 设置玩家进度
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

	std::cout << "正在启动服务器，端口: 25565" << std::endl;

	// 检查端口占用
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup 失败" << std::endl;
		return -1;
	}

	SOCKET test_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(25565);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(test_sock, (sockaddr*)&addr, sizeof(addr))){
		std::cerr << "端口 25565 已被占用! 错误代码: " << WSAGetLastError() << std::endl;
		MessageBoxA(nullptr, "端口 25565 已被占用", "服务器错误", MB_ICONERROR | MB_OK);
		closesocket(test_sock);
		WSACleanup();
		return -2;
	}
	closesocket(test_sock);
		WSACleanup();

		std::cout << "端口 25565 可用，开始监听..." << std::endl;

		try {
		server.listen("0.0.0.0", 25565);
		std::cout << "服务器已在 0.0.0.0:25565 上运行" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "服务器启动失败: " << e.what() << std::endl;
		MessageBoxA(nullptr, e.what(), "服务器错误", MB_ICONERROR | MB_OK);
		return -3;
	}

	system("pause");
	return 0;
}

