#include"../thirdparty/httplib.h"
#include <thread>
#include <mutex>

std::mutex mtx;  // 创建一个互斥锁

std::string str_text;            //文本内容

int progress_1 = -1;             //玩家1进度
int progress_2 = -1;             //玩家2进度	
int main(int argc, char** argv) {
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
	server.listen("0.0.0.0", 25565);
	return 0;
}

