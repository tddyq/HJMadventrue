#pragma once
#include"../thirdparty/httplib.h"
#include"../thirdparty/json.hpp"
#include"Atlas.h"
#include "Path.h"
#include"util.h"
#include "Player.h"
#include"IClient.h"

#include <chrono>                  // ʱ�䴦��
#include <string>                  // �ַ�������
#include <vector>                  // ��̬��������
#include <thread>                  // ���߳�֧��
#include <codecvt>                 // �ַ�����ת����ע��C++17�����ã�
#include <fstream>                 // �ļ�������
#include <sstream>                 // �ַ���������



using json = nlohmann::json;

class CClient:public IClient
{
public:
    CClient(int size) : player_progress(size + 1, -1), players(size+1), progressArraySize(size + 1) {
        init(); 
    }
    ~CClient() = default;

    void login_to_server()override {
        check_server_connection(hwnd); // �ȼ���������
        
        //����
        std::cout << "address: " << str_address << "post: " << post << std::endl;

        client = new httplib::Client(str_address, post);
        client->set_keep_alive(true);

        httplib::Result result = client->Post("/login");

        if (!result) {
            // �����ϸ�Ĵ�����Ϣ
            std::string error_msg = "HTTP����: ";
            error_msg += httplib::to_string(result.error());
            error_msg += "\n��������: "+ str_address + " : " + std::to_string(post);

            // ת��Ϊ���ַ���
            int wchars_num = MultiByteToWideChar(CP_UTF8, 0, error_msg.c_str(), -1, NULL, 0);
            wchar_t* wstr = new wchar_t[wchars_num];
            MultiByteToWideChar(CP_UTF8, 0, error_msg.c_str(), -1, wstr, wchars_num);

            //����
            std::cout << "����ʧ��: " + error_msg << std::endl;

            MessageBox(hwnd, wstr, _T("����ʧ��"), MB_OK | MB_ICONERROR);
            delete[] wstr;
            exit(-1);
        }

        if (!result || result->status != 200) {
            //����
            std::cout << "�޷����ӵ�������,�ܾ�����" << std::endl;

            MessageBox(hwnd, _T("�޷����ӵ�������"), _T("�ܾ�����"), MB_OK | MB_ICONERROR);
            exit(-1);
        }

        id_player = std::stoi(result->body);

        if (id_player < 0) {
            MessageBox(hwnd, _T("�����Ѿ���ʼ��"), _T("�ܾ�����"), MB_OK | MB_ICONERROR);
            exit(-1);
        }
        player_progress[progressArraySize - 1] = 0;
        player_progress[id_player] = 0;
        progressBackup = 0;

        //����
        std::cout << "���������ӳɹ�" << std::endl;

        //���� POST ���� /query_text ·��������ȡ���������ص��ı�����
        str_text = client->Post("/query_text")->body;
        //ʹ�� std::stringstream �� str_text ��Ϊ�������������ж�ȡ��
        //ÿ��ȡһ���ı����洢�� str_line_list �У���ͳ��ÿ�е��ַ���,�ۼӵ� num_total_char
        std::stringstream str_stream(str_text);
        std::string str_line;
        while (std::getline(str_stream, str_line)) {
            str_line_list.push_back(str_line);
            num_total_char += (int)str_line.length();
        }
        //����
        std::cout <<"�ַ�����: " << num_total_char << std::endl;


        uploadProgressToServer();
    }
    void uploadProgressToServer()override {
        std::thread([&]() {
            try {
                while (true) {
                    using namespace std::chrono;
                    //Ϊÿһ��������ȷ��·��
                    std::string route = "/update_" + std::to_string(id_player);
                    std::string body = std::to_string(player_progress[id_player]);
                    //��ӡ����
                    std::cout << "Sending request to route: " << route << " with body: " << body << std::endl;
                   
                    httplib::Result result = client->Post(route, body, "text/plain");
                    //��ӡ����
                    if (result) std::cout << "Received response: " << result->status << std::endl;
                    else std::cout << "Request failed, error: " << result.error() << std::endl;

                    getProgressFromServer(result);
                    std::this_thread::sleep_for(nanoseconds(1000000000 / 10));
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Exception: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unknown exception!" << std::endl;
            }
            }).detach();
    }
    void getProgressFromServer(httplib::Result& result) {
        if (result && result->status == 200) {
            //������������سɹ���״̬��Ϊ 200�����������صĽ���ֵ��������Ӧ��ҵĽ���
            updateProgressFromJson(result->body);
        }
    }
    void getProgressFromServer() {

    }
    void load_resources()override {
        
        
        // ����������Դ
        AddFontResourceEx(_T("resources/IPix.ttf"), FR_PRIVATE, NULL);

        // ���1������Դ
        atlas_1P_idle_up.load(_T("resources/hajimi_idle_back_%d.png"), 4);
        atlas_1P_idle_down.load(_T("resources/hajimi_idle_front_%d.png"), 4);
        atlas_1P_idle_left.load(_T("resources/hajimi_idle_left_%d.png"), 4);
        atlas_1P_idle_right.load(_T("resources/hajimi_idle_right_%d.png"), 4);
        atlas_1P_run_up.load(_T("resources/hajimi_run_back_%d.png"), 4);
        atlas_1P_run_down.load(_T("resources/hajimi_run_front_%d.png"), 4);
        atlas_1P_run_left.load(_T("resources/hajimi_run_left_%d.png"), 4);
        atlas_1P_run_right.load(_T("resources/hajimi_run_right_%d.png"), 4);

        // ���2������Դ
        atlas_2P_idle_up.load(_T("resources/manbo_idle_back_%d.png"), 4);
        atlas_2P_idle_down.load(_T("resources/manbo_idle_front_%d.png"), 4);
        atlas_2P_idle_left.load(_T("resources/manbo_idle_left_%d.png"), 4);
        atlas_2P_idle_right.load(_T("resources/manbo_idle_right_%d.png"), 4);
        atlas_2P_run_up.load(_T("resources/manbo_run_back_%d.png"), 4);
        atlas_2P_run_down.load(_T("resources/manbo_run_front_%d.png"), 4);
        atlas_2P_run_left.load(_T("resources/manbo_run_left_%d.png"), 4);
        atlas_2P_run_right.load(_T("resources/manbo_run_right_%d.png"), 4);
        

        // ���ؽ���ͼƬ��Դ
        loadimage(&img_ui_1, _T("resources/ui_1.png"));
        loadimage(&img_ui_2, _T("resources/ui_2.png"));
        loadimage(&img_ui_3, _T("resources/ui_3.png"));
        loadimage(&img_ui_fight, _T("resources/ui_fight.png"));
        loadimage(&img_ui_textbox, _T("resources/ui_textbox.png"));
        loadimage(&img_background, _T("resources/background.png"));

        // ��������
        load_audio(_T("resources/bgm.mp3"), _T("bgm"));
        load_audio(_T("resources/1p_win.mp3"), _T("1p_win"));
        load_audio(_T("resources/2p_win.mp3"), _T("2p_win"));
        load_audio(_T("resources/click_1.mp3"), _T("click_1"));
        load_audio(_T("resources/click_2.mp3"), _T("click_2"));
        load_audio(_T("resources/click_3.mp3"), _T("click_3"));
        load_audio(_T("resources/click_4.mp3"), _T("click_4"));
        load_audio(_T("resources/ui_1.mp3"), _T("ui_1"));
        load_audio(_T("resources/ui_2.mp3"), _T("ui_2"));
        load_audio(_T("resources/ui_3.mp3"), _T("ui_3"));
        load_audio(_T("resources/ui_fight.mp3"), _T("ui_fight"));

        std::ifstream file("config.cfg");

        if (!file.good()) {
            MessageBox(hwnd, L"�޷������� config.cfg", L"����ʧ��", MB_OK | MB_ICONERROR);
            exit(-1);
        }
        // ����һ���ַ����������ڽ��ļ����ݶ�ȡ���ڴ���
        std::stringstream str_stream;
        str_stream << file.rdbuf();
        str_address = str_stream.str();

        file.close();
    }
    void init()override {
        HWND hwnd = initgraph(1280, 720/*,EW_SHOWCONSOLE*/);
        SetWindowText(hwnd, _T("�����״�ð��!"));
        settextstyle(28, 0, _T("IPix"));

        setbkmode(TRANSPARENT);

        load_resources();
        login_to_server();

        //��ʼ�����н�ɫ������λ��
        for (int i = 0; i < progressArraySize - 1; i++) {
            if (i % 2 == 0) {
                players[i].setAtlas(&atlas_1P_idle_up, &atlas_1P_idle_down, &atlas_1P_idle_left, &atlas_1P_idle_right,
                    &atlas_1P_run_up, &atlas_1P_run_down, &atlas_1P_run_left, &atlas_1P_run_right);
            }
            else {
                players[i].setAtlas(&atlas_2P_idle_up, &atlas_2P_idle_down, &atlas_2P_idle_left, &atlas_2P_idle_right,
                    &atlas_2P_run_up, &atlas_2P_run_down, &atlas_2P_run_left, &atlas_2P_run_right);
            }
            players[i].set_position({842,842});
        }

        // ��ʼ������
        camera_ui.set_size({ 1280, 720 });
        camera_scene.set_size({ 1280, 720 });
        

        // ����ʱ��ʱ�����ã����Ե�2��ͼ��
        timer_countdown.set_one_shot(false);
        timer_countdown.set_wait_time(1.0f);
        timer_countdown.set_on_timeout([&]()
            {
                val_countdown--;
                switch (val_countdown)
                {
                case 3: play_audio(_T("ui_3")); break;
                case 2: play_audio(_T("ui_2")); break;
                case 1: play_audio(_T("ui_1")); break;
                case 0: play_audio(_T("ui_fight")); break;
                case -1:
                    stage = Stage::Racing;
                    play_audio(_T("bgm"), true); // true��ʾѭ������
                    break;
                }
            });

        post = 25565;
    }
    void mainLoop()override {
        using namespace std::chrono;
        // ֡�ʿ�������
        const std::chrono::nanoseconds frame_duration(1000000000 / 144);
        steady_clock::time_point last_tick = steady_clock::now();
        BeginBatchDraw();  // ��ʼ�������Ʋ���
        while(true){
            /////////////////�����������////////////////
            processPlayerInput();
            /////////////////������Ϸ����////////////////
            steady_clock::time_point frame_start = steady_clock::now();
            duration<float> delta = duration<float>(frame_start - last_tick);

            //����
            std::cout << "delta:" << delta.count() << std::endl;
            processGameUpdate(delta.count());

            ////////////////������Ϸ��Ⱦ/////////////////
            processGameRander();

            FlushBatchDraw();

            last_tick = frame_start;
            std::chrono::nanoseconds sleep_duration = frame_duration - (steady_clock::now() - frame_start);
            if (sleep_duration > nanoseconds(0))
                std::this_thread::sleep_for(sleep_duration);
        }

        
    }
private:
    void check_server_connection(HWND hwnd) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            MessageBox(hwnd, _T("WSAStartup ʧ��"), _T("�������"), MB_OK | MB_ICONERROR);
            exit(-1);
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            MessageBox(hwnd, _T("���� socket ʧ��"), _T("�������"), MB_OK | MB_ICONERROR);
            WSACleanup();
            exit(-1);
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(25565);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        // ���ó�ʱ
        DWORD timeout = 3000; // 3��
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        if (connect(sock, (sockaddr*)&addr, sizeof(addr))) {
            int error = WSAGetLastError();
            std::wstring msg = L"�޷����ӵ�������\n";
            msg += L"IP: 127.0.0.1\n";
            msg += L"�˿�: 25565\n\n";
            msg += L"�������: " + std::to_wstring(error) + L"\n";

            switch (error) {
            case WSAECONNREFUSED: msg += L"���ӱ��ܾ� (������δ����?)"; break;
            case WSAETIMEDOUT: msg += L"���ӳ�ʱ"; break;
            case WSAEADDRNOTAVAIL: msg += L"��ַ������"; break;
            default: msg += L"δ֪�������";
            }

            MessageBox(hwnd, msg.c_str(), _T("���Ӳ���ʧ��"), MB_OK | MB_ICONERROR);
            closesocket(sock);
            WSACleanup();
            exit(-1);
        }

        closesocket(sock);
        WSACleanup();
    }
    void setDomainAndPost(std::string domain, int post) {
        this->domain = domain;
        this->post = post;
    }
    void updateProgressFromJson(const std::string& body) {
        try {
            // ���� JSON �ַ���
            json jsonData = json::parse(body);

            // SON ��ʽ��һ������
            if (jsonData.is_array()) {
                std::vector<int>  newProgresses = jsonData.get<std::vector<int>>();
                
                if (newProgresses.size() == progressArraySize) {
                    player_progress = std::move(newProgresses);
                    //������ݳ���ʹ�÷��������ص�ǰ����С����ʵ������ָ�
                    if (player_progress[id_player] < progressBackup) player_progress[id_player] = progressBackup;

                }
            }
            else {
                std::cerr << "JSON data is not an array!" << std::endl;
            }
        }
        catch (const json::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }
    void processPlayerInput() {
        ////////////////�����������///////////////
        try {
            while (peekmessage(&msg)) {
                if (stage != Stage::Racing) continue;

                if (msg.message == WM_CHAR && idx_line < str_line_list.size()) {
                    const std::string& str_line = str_line_list[idx_line];
                    if (str_line[idx_char] == msg.ch) {

                        //����
                        std::cout << "������ȷ,����progress" << std::endl;

                        switch (rand() % 4) {
                        case 0: play_audio(_T("click_1")); break;
                        case 1: play_audio(_T("click_2")); break;
                        case 2: play_audio(_T("click_3")); break;
                        case 3: play_audio(_T("click_4")); break;
                        }

                        // ��ӡ��ǰ������Ϣ
                        std::cout << "Player progress before increment: " << player_progress[id_player] << std::endl;

                        player_progress[id_player]++;
                        progressBackup++;

                        // ��ӡ���º�Ľ���
                        std::cout << "Player progress after increment: " << player_progress[id_player] << std::endl;

                        idx_char++;
                        if (idx_char >= str_line.length()) {
                            idx_char = 0;
                            idx_line++;
                        }
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown exception!" << std::endl;
        }
    }
    void processGameUpdate(float delta) {
        try {
            // �������
            std::cout << "===== ��ʼ������Ϸ���� delta=" << delta << " =====" << std::endl;

            if (stage == Stage::Waiting) {
                
                for (int i = 0; i < progressArraySize - 1; i++) {
                    if (player_progress[i] != 0) {
                        
                        return;
                    }
                }
                stage = Stage::Ready;
                std::cout << "״̬���: Waiting -> Ready" << std::endl;
            }
            else
            {
                std::cout << "��ǰ״̬: " << ((stage == Stage::Ready) ? "Ready" : "����") << std::endl;

                if (stage == Stage::Ready)
                    timer_countdown.on_update(delta);

                // ������Ϸ��������
                std::cout << "���ȼ��: ���ݽ���=" << progressBackup <<" �ַ�����: "<<num_total_char<< std::endl;
                if (progressBackup >= num_total_char) {
                    
                    stop_audio(_T("bgm"));
                    play_audio((id_player == 1) ? _T("1p_win") : _T("2p_win"));
                    MessageBox(hwnd, _T("Ӯ����"), _T("��Ϸ����"), MB_OK | MB_ICONINFORMATION);
                    exit(0);
                }
                else if (player_progress[progressArraySize - 1] >= num_total_char) {
                    
                    stop_audio(_T("bgm"));
                    MessageBox(hwnd, _T("����"), _T("��Ϸ����"), MB_OK | MB_ICONINFORMATION);
                    exit(0);
                }

                // �������λ�ø���
                std::cout << "�������Ŀ��λ�� | �������=" << progressArraySize - 1 << std::endl;
                for (int i = 0; i < progressArraySize - 1; i++) {
                    float progress = (float)player_progress[i] / num_total_char;
                    players[i].set_target(path.get_position_at_progress(progress));
                    
                    
                    std::cout << "���" << i << " ����=" << player_progress[i]<< " ��һ��=" << progress << std::endl;
                    
                   
                }

                // ������Ҹ���
                std::cout << "ִ����Ҹ���" << std::endl;

                for (int i = 0; i < progressArraySize - 1; i++) {
                    players[i].on_update(delta);

                    std::cout << "���" << i << " λ�ø������" << std::endl;
                }

                // �������������
                Vector2 camPos = players[id_player].get_position();
                
                camera_scene.look_at(camPos);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "!!! �쳣����: " << e.what() << std::endl;
            
        }
        catch (...) {
            std::cerr << "!!! δ֪�����쳣����" << std::endl;
        }

        std::cout << "===== ������Ϸ���´��� =====" << std::endl;
    }
    void processGameRander() {
        std::cout << "[DEBUG] Entering processGameRander" << std::endl;

        try {
            std::cout << "stage:" << (stage == Stage::Racing) << std::endl;

            ///////////////��Ⱦ����/////////////
            setbkcolor(RGB(0, 0, 0));
            cleardevice();

            if (stage == Stage::Waiting) {
                std::cout << "[DEBUG] Rendering waiting stage" << std::endl;
                settextcolor(RGB(195, 195, 195));
                outtextxy(15, 675, _T("����������ʼ,�ȴ�������Ҽ���"));
                //����
                std::cout << "�ѻ��Ƶȴ�" << std::endl;
            }
            else {
                std::cout << "[DEBUG] Rendering racing stage" << std::endl;

                //���Ʊ���ͼ
                try {
                    static const Rect rect_bg = {
                        0,0,
                        img_background.getwidth(),
                        img_background.getheight()
                    };
                    putimage_ex(camera_scene, &img_background, &rect_bg);
                    std::cout << "[DEBUG] Background rendered successfully" << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "[ERROR] Background rendering failed: " << e.what() << std::endl;
                }

                //�������
                try {
                    for (int i = 0; i < progressArraySize - 1; i++) {
                        players[i].on_render(camera_scene);
                    }
                    std::cout << "[DEBUG] Players rendered successfully" << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "[ERROR] Player rendering failed: " << e.what() << std::endl;
                }

                //���Ƶ���ʱ
                try {
                    std::cout << "[DEBUG] Rendering countdown: " << val_countdown << std::endl;
                    switch (val_countdown) {
                    case 3: {
                        static const Rect rect_ui_3 = {
                            1280 / 2 - img_ui_3.getwidth() / 2,
                            720 / 2 - img_ui_3.getheight() / 2,
                            img_ui_3.getwidth(),img_ui_3.getheight()
                        };
                        putimage_ex(camera_ui, &img_ui_3, &rect_ui_3);
                    }
                          break;
                    case 2: {
                        static const Rect rect_ui_2 = {
                            1280 / 2 - img_ui_2.getwidth() / 2,
                            720 / 2 - img_ui_2.getheight() / 2,
                            img_ui_2.getwidth(),img_ui_3.getheight()
                        };
                        putimage_ex(camera_ui, &img_ui_2, &rect_ui_2);
                    }
                          break;

                    case 1: {
                        static const Rect rect_ui_1 = {
                            1280 / 2 - img_ui_1.getwidth() / 2,
                            720 / 2 - img_ui_1.getheight() / 2,
                            img_ui_1.getwidth(),img_ui_3.getheight()
                        };
                        putimage_ex(camera_ui, &img_ui_1, &rect_ui_1);
                    }
                          break;
                    case 0: {
                        static const Rect rect_ui_fight = {
                            1280 / 2 - img_ui_fight.getwidth() / 2,
                            720 / 2 - img_ui_fight.getheight() / 2,
                            img_ui_fight.getwidth(),img_ui_fight.getheight()
                        };
                        putimage_ex(camera_ui, &img_ui_1, &rect_ui_fight);
                    }
                          break;
                    default:
                        std::cout << "[WARN] Unexpected countdown value: " << val_countdown << std::endl;
                        break;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "[ERROR] Countdown rendering failed: " << e.what() << std::endl;
                }

                //���ƽ���
                if (stage == Stage::Racing) {
                    try {
                        std::cout << "[DEBUG] Rendering racing UI" << std::endl;
                        static const Rect rect_textbox = {
                            0,
                            720 - img_ui_textbox.getheight(),
                            img_ui_textbox.getwidth(),
                            img_ui_textbox.getheight()
                        };

                        // �ַ���ת�������׳��쳣
                        try {
                            static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
                            std::wstring wstr_line = convert.from_bytes(str_line_list[idx_line]);
                            std::wstring wstr_completed = convert.from_bytes(str_line_list[idx_line].substr(0, idx_char));

                            putimage_ex(camera_ui, &img_ui_textbox, &rect_textbox);

                            settextcolor(RGB(125, 125, 125));
                            outtextxy(185 + 2, rect_textbox.y + 65 + 2, wstr_line.c_str());

                            settextcolor(RGB(25, 25, 25));
                            outtextxy(185, rect_textbox.y + 65, wstr_line.c_str());

                            settextcolor(RGB(0, 149, 217));
                            outtextxy(185, rect_textbox.y + 65, wstr_completed.c_str());

                            std::cout << "[DEBUG] UI text rendered successfully" << std::endl;
                        }
                        catch (const std::exception& e) {
                            std::cerr << "[ERROR] String conversion failed: " << e.what() << std::endl;
                        }
                    }
                    catch (const std::exception& e) {
                        std::cerr << "[ERROR] UI rendering failed: " << e.what() << std::endl;
                    }
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[CRITICAL] Unhandled exception in processGameRander: "
                << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "[CRITICAL] Unknown exception in processGameRander" << std::endl;
        }

        std::cout << "[DEBUG] Exiting processGameRander" << std::endl;
    }
private:
    enum class Stage {
        Waiting,
        Ready,
        Racing
    };
    Path path = Path({
    {842,842},{1322,842},{1322,442},
    {2762,442},{2762,842},{3162,842},
    {3162,1722},{2122,1722},{2122,1562},
    {842,1562},{842,842}
        });

private:
    // ���1����ͼ��
    Atlas atlas_1P_idle_up;     //���1�������ö���ͼ��
    Atlas atlas_1P_idle_down;   //���1�������ö���ͼ��
    Atlas atlas_1P_idle_left;   //���1�������ö���ͼ��
    Atlas atlas_1P_idle_right;  //���1�������ö���ͼ��
    Atlas atlas_1P_run_up;      //���1���ϱ��ܶ���ͼ��
    Atlas atlas_1P_run_down;    //���1���±��ܶ���ͼ��
    Atlas atlas_1P_run_left;    //���1�����ܶ���ͼ��
    Atlas atlas_1P_run_right;   //���1���ұ��ܶ���ͼ��

    // ���2����ͼ��
    Atlas atlas_2P_idle_up;     //���2�������ö���ͼ��
    Atlas atlas_2P_idle_down;   //���2�������ö���ͼ��
    Atlas atlas_2P_idle_left;   //���2�������ö���ͼ��
    Atlas atlas_2P_idle_right;  //���2�������ö���ͼ��
    Atlas atlas_2P_run_up;      //���2���ϱ��ܶ���ͼ��
    Atlas atlas_2P_run_down;    //���2���±��ܶ���ͼ��
    Atlas atlas_2P_run_left;    //���2�����ܶ���ͼ��
    Atlas atlas_2P_run_right;   //���2���ұ��ܶ���ͼ��

    // ����ͼ����Դ
    IMAGE img_ui_1;           //�����ı�1
    IMAGE img_ui_2;           //�����ı�2
    IMAGE img_ui_3;           //�����ı�3
    IMAGE img_ui_fight;       //�����ı�FIGHT
    IMAGE img_ui_textbox;     //�����ı���
    IMAGE img_background;     //����ͼ
private:
    //��������
    int val_countdown = 4;               //���ܵ���ʱ
    Stage stage = Stage::Waiting;        //��ǰ��Ϸ״̬

    std::vector<int> player_progress;
    int progressArraySize;
    int progressBackup;

    int num_total_char = 0;              //ȫ���ַ���
    int id_player = 0;

    int idx_line = 0;                        //��ǰ�ı�������
    int idx_char = 0;                        //��ǰ���ı��ַ�����
    std::string str_text;                    //�ı�����
    std::vector<std::string> str_line_list;  //���ı��б�

private:
    //���������
    std::string str_address;              //��������ַ
    httplib::Client* client = nullptr;    //http�ͻ��˶���
    std::string domain;                   //����
    int post = 25565;                     //�˿�
private:
    HWND hwnd;                            //���ھ��
    std::vector<Player> players;

    ExMessage msg;
    Timer timer_countdown;
    Camera camera_ui, camera_scene;
};

