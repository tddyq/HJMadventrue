#pragma once
#include "INetworkAdapter.h"
#include"../thirdparty/httplib.h"
#include <thread>
#include <mutex>
#include <vector>
#include<algorithm>
#include"../thirdparty/json.hpp"
#include "RuleModule.h"

using json = nlohmann::json;

class HttplibNetworkAdapter :public INetworkAdapter
{
public:

    /*HttplibNetworkAdapter(int size) : player_progress(size + 1, -1), 
           progressArraySize(size+1){
        player_progress[progressArraySize - 1] = 0;
    }*/
    HttplibNetworkAdapter(RuleModule* ruleModule) : ruleModule(ruleModule) {

    }
    ~HttplibNetworkAdapter() = default;

    // ���ӷ�����
    void login()override {
        std::cout << "[LOGIN] Setting up login endpoint" << std::endl;

        server.Post("/login", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                std::cout << "[LOGIN] Received login request" << std::endl;

                std::lock_guard<std::mutex> lock(*mtx);


                //�������id
                int player_id = -1;
                std::cout << "[LOGIN] Available slots: " << ruleModule->player_progress.size() - 1 << std::endl;

                try {
                    for (int i = 0; i < ruleModule->player_progress.size() - 1; i++) {
                        if (ruleModule->player_progress[i] == -1) {
                            player_id = i;
                            std::cout << "[LOGIN] Found free slot: " << player_id << std::endl;
                            break;
                        }
                    }

                    std::cout << "[LOGIN] Final player_id: " << player_id << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "[LOGIN_ERROR] Slot search failed: " << e.what() << std::endl;
                    res.set_content("-2", "text/plain");  // �ڲ�����
                    return;
                }

                if (player_id >= ruleModule->player_progress.size() - 1) {
                    std::cout << "[LOGIN_WARN] No available slots" << std::endl;
                    res.set_content("-1", "text/plain");  // ������������
                }
                else {
                    try {
                        
                        ruleModule->player_progress[player_id] = 0;  // ��ʼ����ҽ���
                        create_update_route(player_id);  // ��̬������ҽ��ȸ���·��
                        res.set_content(std::to_string(player_id), "text/plain");  // �������ID

                        std::cout << "[LOGIN] Assigning player ID: " << player_id << std::endl;
                        std::cout << "[LOGIN] Creating update route for player: " << player_id << std::endl;
                        std::cout << "[LOGIN_SUCCESS] Player " << player_id << " logged in" << std::endl;
                    }
                    catch (const std::exception& e) {
                        std::cerr << "[LOGIN_ERROR] Player setup failed: " << e.what() << std::endl;
                        res.set_content("-3", "text/plain");  // ��ʼ������
                        return;
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[LOGIN_CRITICAL] Unhandled exception: " << e.what() << std::endl;
                res.set_content("-4", "text/plain");  // ����������
            }
            catch (...) {
                std::cerr << "[LOGIN_CRITICAL] Unknown exception occurred" << std::endl;
                res.set_content("-5", "text/plain");  // δ֪����
            }
            });

        std::cout << "[LOGIN] Login endpoint setup complete" << std::endl;

    }

    // ���ӷ�����,�����˿�
    bool connect(const std::string& address, int port) {
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

        if (bind(test_sock, (sockaddr*)&addr, sizeof(addr))) {
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
    }

    // ������ҽ���
    void sendProgress(int playerId, int progress)override {

    }

    // ��ȡ������ҽ���
    void getAllProgress()override {

    }

    // ��ȡ��Ϸ�ı�
    void getGameText(std::string str_text)override {
        server.Post("/query_text", [&,str_text](const httplib::Request& req, httplib::Response& res) {

            res.set_content(str_text, "text/plain");
            });
    }
private:
    void create_update_route(int player_id) {
        std::string route = "/update_" + std::to_string(player_id);

        server.Post(route.c_str(), [this, player_id](const httplib::Request& req, httplib::Response& res) {
            try {
                std::cout << "[UPDATE] Received progress update for player " << player_id << std::endl;
                std::cout << "[UPDATE] Request body: " << req.body << std::endl;

                std::lock_guard<std::mutex> lock(*mtx);          

                try {

                    ruleModule->updatePlayerProcess(player_id, req, res);
                }
                catch (const std::out_of_range& e) {
                    std::cerr << "[UPDATE_ERROR] Invalid player ID: " << player_id
                        << " - " << e.what() << std::endl;
                    res.set_content("{\"error\":\"Invalid player ID\"}", "application/json");
                }
                
            }
            catch (const std::exception& e) {
                std::cerr << "[UPDATE_CRITICAL] Unhandled exception: " << e.what() << std::endl;
                res.set_content("{\"error\":\"Server error\"}", "application/json");
            }
            catch (...) {
                std::cerr << "[UPDATE_CRITICAL] Unknown exception occurred" << std::endl;
                res.set_content("{\"error\":\"Unknown server error\"}", "application/json");
            }
            });
    }
private:
    std::unique_ptr<std::mutex> mtx = std::make_unique<std::mutex>();  // ����һ��������
    httplib::Server server;
private:
    //std::vector<int> player_progress;
    //int progressArraySize;
private:
    RuleModule* ruleModule;
};
