#pragma once
#include"../thirdparty/httplib.h"
#include"../thirdparty/json.hpp"
#include"Atlas.h"
#include "Path.h"
#include"util.h"
#include "Player.h"
#include"IClient.h"
#include"lua.hpp"                  //lua���

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
    CClient(int size) : player_progress(size + 1, -1), progressBackup(size+1,-1), players(size + 1), 
        progressArraySize(size + 1), prev_player_rects(size+1){
        
    }
    ~CClient() = default;

    void login_to_server()override {
        check_server_connection(hwnd); // �ȼ���������
        
        

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

            

            MessageBox(hwnd, wstr, _T("����ʧ��"), MB_OK | MB_ICONERROR);
            delete[] wstr;
            exit(-1);
        }

        if (!result || result->status != 200) {
            

            MessageBox(hwnd, _T("�޷����ӵ�������"), _T("�ܾ�����"), MB_OK | MB_ICONERROR);
            exit(-1);
        }

        id_player = std::stoi(result->body);

        if (id_player < 0) {
            MessageBox(hwnd, _T("�����Ѿ���ʼ��"), _T("�ܾ�����"), MB_OK | MB_ICONERROR);
            exit(-1);
        }
        player_progress[progressArraySize - 1] = -1;
        player_progress[id_player] = 0;
        progressBackup[progressArraySize - 1] = -1;
        progressBackup[id_player] = 0;

        

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
        


        uploadProgressToServer();
    }
    void uploadProgressToServer()override {
        std::thread([&]() {
            
            try {
                while (true) {
                    using namespace std::chrono;
                    //Ϊÿһ��������ȷ��·��
                    std::string route = "/update_" + std::to_string(id_player);
                    std::string body;
                    
                    body = std::to_string(progressBackup[id_player]);
                    
                   
                    httplib::Result result = client->Post(route, body, "text/plain");
                    

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
        

        // ����ʱ��ʱ������
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
        //�Զ����߶�ʱ������-����
        timer_step.set_one_shot(false);
        timer_step.set_wait_time(0.5f);
        timer_step.set_on_timeout([&]() {
            
            //static std::random_device seed;       // ֻ�ᴴ��һ��
            //static std::ranlux48 engine(seed());  // ֻ�ᴴ��һ��
            //static std::uniform_int_distribution<> distrib(1, 10);  // ֻ�ᴴ��һ��

            //int random = distrib(engine);         // ���������

            int random = getRandomFromLua();

            progressBackup[id_player] += random;

            idx_char += random;
            if (idx_line < str_line_list.size() && idx_char >= str_line_list[idx_line].length()) {
                idx_char = 0;
                idx_line++;
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

            
            processGameUpdate(delta.count());

            ////////////////������Ϸ��Ⱦ/////////////////
            processGameRender();

            FlushBatchDraw();

            last_tick = frame_start;
            std::chrono::nanoseconds sleep_duration = frame_duration - (steady_clock::now() - frame_start);
            if (sleep_duration > nanoseconds(0))
                std::this_thread::sleep_for(sleep_duration);
        }
        EndBatchDraw();
        
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
    int getRandomFromLua() {
        static lua_State* L = luaL_newstate();
        static bool isLuaOpen = false;

        if (!isLuaOpen) {
            isLuaOpen = true;
            luaL_openlibs(L);

            if (luaL_dofile(L, "script.lua") != LUA_OK) {
                std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
                return -1;
            }
        }

        lua_getglobal(L, "generateRandomStep");

        if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
            std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
            lua_close(L);
            return -1;
        }

        int random = lua_tonumber(L, -1);

        return random;
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

                    std::lock_guard<std::mutex> lock(progress_mutex);

                    player_progress = std::move(newProgresses);
                    //������ݳ���ʹ�÷��������ص�ǰ����С����ʵ������ָ�
                    /*if (player_progress[id_player] >= progressBackup[id_player]) {
                        progressBackup = player_progress;
                    }*/
                    progressBackup = player_progress;

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
                        std::cout << "Player progress before increment: " << progressBackup[id_player] << std::endl;

                        progressBackup[id_player]++;

                        // ��ӡ���º�Ľ���
                        std::cout << "Player progress after increment: " << progressBackup[id_player] << std::endl;

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
            

            if (stage == Stage::Waiting) {
                for (int i = 0; i < progressArraySize - 1; i++) {
                    if (progressBackup[i] < 0) {
                        
                        return;
                    }
                }
                stage = Stage::Ready;
            }
            else
            {
                if (stage == Stage::Ready) {
                    timer_countdown.on_update(delta);
                }
                else if (stage == Stage::Racing) {
                    timer_step.on_update(delta);//����-�Զ�����
                }
                //if (progressBackup[id_player] >= num_total_char) {
                if (progressBackup[progressArraySize - 1] != -1 && progressBackup[progressArraySize - 1] == id_player) {
                    
                    stop_audio(_T("bgm"));
                    play_audio((id_player == 1) ? _T("1p_win") : _T("2p_win"));
                    MessageBox(hwnd, _T("Ӯ����"), _T("��Ϸ����"), MB_OK | MB_ICONINFORMATION);
                    exit(0);
                }
                else if (progressBackup[progressArraySize - 1] != -1 && progressBackup[progressArraySize - 1] != id_player) {
                    //else if (progressBackup[progressArraySize - 1] >= num_total_char) {
                    stop_audio(_T("bgm"));
                    MessageBox(hwnd, _T("����"), _T("��Ϸ����"), MB_OK | MB_ICONINFORMATION);
                    exit(0);
                }

                // �������λ�ø���
                std::cout << "�������Ŀ��λ�� | �������=" << progressArraySize - 1 << std::endl;

                {
                    std::lock_guard<std::mutex> lock(progress_mutex);

                    for (int i = 0; i < progressArraySize - 1; i++) {
                        float progress = (float)progressBackup[i] / num_total_char;
                        players[i].set_target(path.get_position_at_progress(progress));

                    }
                }

                // ������Ҹ���
                std::cout << "ִ����Ҹ���" << std::endl;

                for (int i = 0; i < progressArraySize - 1; i++) {
                    players[i].on_update(delta);
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


    }
    void processGameRender() {
        ///////////////��Ⱦ����/////////////
        //setbkcolor(RGB(0, 0, 0));
        //cleardevice();

        if (stage == Stage::Waiting) {
            settextcolor(RGB(195, 195, 195));
            outtextxy(15, 675, _T("����������ʼ,�ȴ�������Ҽ���"));
        }
        else if (stage == Stage::Ready) {
            //���Ʊ���ͼ
            static const Rect rect_bg = {
                0, 0,
                img_background.getwidth(),
                img_background.getheight()
            };
            putimage_ex(camera_scene, &img_background, &rect_bg);

            //�������
            for (int i = 0; i < progressArraySize - 1; i++) {
                players[i].on_render(camera_scene);
            }

            //���Ƶ���ʱ
            switch (val_countdown) {
            case 3: {
                static const Rect rect_ui_3 = {
                    1280 / 2 - img_ui_3.getwidth() / 2,
                    720 / 2 - img_ui_3.getheight() / 2,
                    img_ui_3.getwidth(), img_ui_3.getheight()
                };
                putimage_ex(camera_ui, &img_ui_3, &rect_ui_3);
            }
                  break;
            case 2: {
                static const Rect rect_ui_2 = {
                    1280 / 2 - img_ui_2.getwidth() / 2,
                    720 / 2 - img_ui_2.getheight() / 2,
                    img_ui_2.getwidth(), img_ui_3.getheight()
                };
                putimage_ex(camera_ui, &img_ui_2, &rect_ui_2);
            }
                  break;

            case 1: {
                static const Rect rect_ui_1 = {
                    1280 / 2 - img_ui_1.getwidth() / 2,
                    720 / 2 - img_ui_1.getheight() / 2,
                    img_ui_1.getwidth(), img_ui_3.getheight()
                };
                putimage_ex(camera_ui, &img_ui_1, &rect_ui_1);
            }
                  break;
            case 0: {
                static const Rect rect_ui_fight = {
                    1280 / 2 - img_ui_fight.getwidth() / 2,
                    720 / 2 - img_ui_fight.getheight() / 2,
                    img_ui_fight.getwidth(), img_ui_fight.getheight()
                };
                putimage_ex(camera_ui, &img_ui_1, &rect_ui_fight);
            }
                  break;
            default:
                break;
            }

            //���ƽ���
            if (stage == Stage::Racing) {
                static const Rect rect_textbox = {
                    0,
                    720 - img_ui_textbox.getheight(),
                    img_ui_textbox.getwidth(),
                    img_ui_textbox.getheight()
                };

                // �ַ���ת��
                static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
                if (idx_line < str_line_list.size()) {
                    std::wstring wstr_line = convert.from_bytes(str_line_list[idx_line]);
                    std::wstring wstr_completed = convert.from_bytes(str_line_list[idx_line].substr(0, idx_char));

                    putimage_ex(camera_ui, &img_ui_textbox, &rect_textbox);

                    settextcolor(RGB(125, 125, 125));
                    outtextxy(185 + 2, rect_textbox.y + 65 + 2, wstr_line.c_str());

                    settextcolor(RGB(25, 25, 25));
                    outtextxy(185, rect_textbox.y + 65, wstr_line.c_str());

                    settextcolor(RGB(0, 149, 217));
                    outtextxy(185, rect_textbox.y + 65, wstr_completed.c_str());
                }
            }
        }
        else {
            dirtyRectRender();
        }
    }
    void dirtyRectRender() {
        std::cout << "===== ��ʼ�������Ⱦ =====" << std::endl;

        // ���������Ϊ��Ч״̬ (��Ȼ�߶�Ϊ0��ʾ��Ч)
        dirty_rect = { 0, 0, 0, 0 };
        std::cout << "���������: (0, 0, 0, 0)" << std::endl;

        // ��ȡ��ǰ�����λ�úͳߴ�
        Rect current_camera_rect = {
            static_cast<int>(camera_scene.get_position().x),
            static_cast<int>(camera_scene.get_position().y),
            static_cast<int>(camera_scene.get_size().x),
            static_cast<int>(camera_scene.get_size().y)
        };

        std::cout << "�����λ��: (" << current_camera_rect.x << ", " << current_camera_rect.y
            << ") �ߴ�: " << current_camera_rect.w << "x" << current_camera_rect.h << std::endl;

        // ���������Ƿ��ƶ�
        if (current_camera_rect != prev_camera_rect) {
            dirty_rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
            std::cout << "������ƶ�! ���������ĻΪ������: (0, 0, "
                << SCREEN_WIDTH << ", " << SCREEN_HEIGHT << ")" << std::endl;
        }

        // �������������
        prev_camera_rect = current_camera_rect;

        // ����������ң����仯
        std::cout << "��� " << (progressArraySize - 1) << " ����ҵı仯..." << std::endl;
        for (int i = 0; i < progressArraySize - 1; i++) {
            // ��ȡ�������Ļ�ϵ���Ⱦ����
            Rect player_rect = players[i].getRenderRect(camera_scene);

            std::cout << "��� " << i << " ��Ļλ��: (" << player_rect.x << ", " << player_rect.y
                << ") �ߴ�: " << player_rect.w << "x" << player_rect.h << std::endl;

            // �������Ƿ�����Ļ��
            if (player_rect.x + player_rect.w < 0 ||
                player_rect.x > SCREEN_WIDTH ||
                player_rect.y + player_rect.h < 0 ||
                player_rect.y > SCREEN_HEIGHT) {
                continue;
            }

            // �޸���ұ仯���
            bool position_changed = (prev_player_rects[i] != player_rect);
            bool state_changed = players[i].checkIsChange();

            // ������λ���Ƿ�仯�򶯻�״̬�Ƿ����
             /*player_moved = (prev_player_rects[i] != player_rect) || players[i].checkIsChange();
            std::cout << "��� " << i << " �仯���: "
                << "λ�ñ仯=" << (prev_player_rects[i] != player_rect)
                << ", ״̬�仯=" << players[i].checkIsChange()
                << ", �ܱ仯=" << player_moved << std::endl;*/

            if (position_changed || state_changed) {
                std::cout << "��� " << i << " ��Ҫ����!" << std::endl;

                if (dirty_rect.w == 0 || dirty_rect.h == 0) {
                    // ��������Ϊ�գ�ֱ������Ϊ�������
                    dirty_rect = player_rect;
                    std::cout << "���������Ϊ�������: (" << player_rect.x << ", " << player_rect.y
                        << ", " << player_rect.w << ", " << player_rect.h << ")" << std::endl;
                }
                else {
                    // ����ϲ�����������������
                    dirty_rect.combineRects(player_rect);
                    std::cout << "�ϲ�������������: �³ߴ� (" << dirty_rect.x << ", " << dirty_rect.y
                        << ", " << dirty_rect.w << ", " << dirty_rect.h << ")" << std::endl;
                }

                // �������ƶ��������һ֡��λ�ã������ͼ��
                /*if (prev_player_rects[i].w > 0 && prev_player_rects[i].h > 0) {
                    dirty_rect.combineRects(prev_player_rects[i]);
                    std::cout << "�����һ֡λ�������Ӱ: (" << prev_player_rects[i].x << ", " << prev_player_rects[i].y
                        << ", " << prev_player_rects[i].w << ", " << prev_player_rects[i].h << ")" << std::endl;
                }*/
                if (position_changed && prev_player_rects[i].isValid()) {
                    dirty_rect.combineRects(prev_player_rects[i]);
                }
            }

            // �������λ�û���
            prev_player_rects[i] = player_rect;
        }

        // �ı������򣨹̶�λ�ã�
        static const Rect textbox_rect = {
            0,
            SCREEN_HEIGHT - img_ui_textbox.getheight(),
            img_ui_textbox.getwidth(),
            img_ui_textbox.getheight()
        };

        std::cout << "�ı�������: (0, " << textbox_rect.y
            << ", " << textbox_rect.w << ", " << textbox_rect.h << ")" << std::endl;

        // �ı�������
        if (prev_idx_char != idx_char || prev_idx_line != idx_line) {

            dirty_rect.combineRects(textbox_rect);

            prev_idx_char = idx_char;
            prev_idx_line = idx_line;
        }

        /*if (dirty_rect.w == 0) {
            dirty_rect = textbox_rect;
        }
        else {
            dirty_rect.combineRects(textbox_rect);
        }*/

        // ȷ�����������Ļ��Χ��
        if (dirty_rect.x < 0) {
            dirty_rect.w += dirty_rect.x;
            dirty_rect.x = 0;
        }
        if (dirty_rect.y < 0) {
            dirty_rect.h += dirty_rect.y;
            dirty_rect.y = 0;
        }
        if (dirty_rect.x + dirty_rect.w > SCREEN_WIDTH) {
            dirty_rect.w = SCREEN_WIDTH - dirty_rect.x;
        }
        if (dirty_rect.y + dirty_rect.h > SCREEN_HEIGHT) {
            dirty_rect.h = SCREEN_HEIGHT - dirty_rect.y;
        }

        // ����Ч����������
        if (dirty_rect.w <= 0 || dirty_rect.h <= 0) return;

        // �����ü�����
        HRGN clip_region = CreateRectRgn(
            dirty_rect.x,
            dirty_rect.y,
            dirty_rect.x + dirty_rect.w,
            dirty_rect.y + dirty_rect.h
        );

        if (!clip_region) {
            std::cerr << "����: �޷������ü�����!" << std::endl;
        }

        // Ӧ�òü�����
        setcliprgn(clip_region);
        std::cout << "���òü�����: (" << dirty_rect.x << ", " << dirty_rect.y
            << ", " << dirty_rect.w << ", " << dirty_rect.h << ")" << std::endl;

        // ������������
        setbkcolor(RGB(0, 0, 0));
        clearrectangle(
            dirty_rect.x,
            dirty_rect.y,
            dirty_rect.x + dirty_rect.w,
            dirty_rect.y + dirty_rect.h
        );
        std::cout << "������������" << std::endl;

        /******************************************************************
         * �ؼ���ѧ���㣺����ͼԴ����
         *
         * ����ͼ��һ����ͼ��������������������ƶ�
         * ���������Ļ�ϵ�һ��������Ҫӳ�䵽����ͼ����Ӧ����
         *
         * ��ѧԭ��:
         * ����ͼԴλ�� = �����λ�� + �����λ��
         *
         * ����:
         * - �����λ�� (camera_scene.get_position()) ��ʾ����ͼ���ĸ�������ʾ����Ļ���Ͻ�
         * - �����λ�� (dirty_rect.x, dirty_rect.y) ���������Ļ���Ͻǵ�ƫ��
         * - ��ˣ�����ͼ�϶�Ӧ��λ���������λ�� + �����λ��
         *
         * ʾ��:
         *   �����λ�� = (100, 50)
         *   �����λ�� = (200, 100)
         *   �򱳾�ͼԴλ�� = (100+200, 50+100) = (300, 150)
         ******************************************************************/
        Rect bg_src = {
            static_cast<int>(camera_scene.get_position().x) + dirty_rect.x,
            static_cast<int>(camera_scene.get_position().y) + dirty_rect.y,
            dirty_rect.w,
            dirty_rect.h
        };

        std::cout << "����ͼԴ�������ǰ: (" << bg_src.x << ", " << bg_src.y
            << ", " << bg_src.w << ", " << bg_src.h << ")" << std::endl;



        // �߽紦�� - ȷ������������ͼ��Χ
        if (bg_src.x < 0) {
            int offset_x = -bg_src.x;
            bg_src.x = 0;
            bg_src.w -= offset_x;
            std::cout << "XԽ�����: ƫ��=" << offset_x
                << ", �³ߴ�: w=" << bg_src.w << std::endl;
        }

        if (bg_src.y < 0) {
            int offset_y = -bg_src.y;
            bg_src.y = 0;
            bg_src.h -= offset_y;
            std::cout << "YԽ�����: ƫ��=" << offset_y
                << ", �³ߴ�: h=" << bg_src.h << std::endl;
        }

        if (bg_src.x + bg_src.w > img_background.getwidth()) {
            bg_src.w = img_background.getwidth() - bg_src.x;
            std::cout << "X+���Խ�����: �¿��=" << bg_src.w << std::endl;
        }

        if (bg_src.y + bg_src.h > img_background.getheight()) {
            bg_src.h = img_background.getheight() - bg_src.y;
            std::cout << "Y+�߶�Խ�����: �¸߶�=" << bg_src.h << std::endl;
        }

        std::cout << "����ͼԴ��������: (" << bg_src.x << ", " << bg_src.y
            << ", " << bg_src.w << ", " << bg_src.h << ")" << std::endl;
        std::cout << "����ͼ�ߴ�: " << img_background.getwidth() << "x" << img_background.getheight() << std::endl;

        // ֻ������Ч�ı�������
        if (bg_src.w > 0 && bg_src.h > 0) {
            std::cout << "��Ⱦ��������..." << std::endl;
           
            putimage(
                dirty_rect.x, // Ŀ��X (��Ļ����)
                dirty_rect.y, // Ŀ��Y (��Ļ����)
                bg_src.w,     // ���ƿ��
                bg_src.h,     // ���Ƹ߶�
                &img_background,
                bg_src.x,     // ԴX (����ͼ����)
                bg_src.y      // ԴY (����ͼ����)
            );
        }
        else {
            std::cout << "��Ч��������������Ⱦ" << std::endl;
        }

        // ��Ⱦ���
        std::cout << "��Ⱦ���..." << std::endl;
        for (int i = 0; i < progressArraySize - 1; i++) {
            Rect player_rect = players[i].getRenderRect(camera_scene);

            // �������Ƿ����������
            bool intersects = dirty_rect.intersects(player_rect);
            std::cout << "��� " << i << " ���������: " << (intersects ? "��" : "��")
                << " �������: (" << player_rect.x << ", " << player_rect.y
                << ", " << player_rect.w << ", " << player_rect.h << ")" << std::endl;

            if (intersects) {
                players[i].on_render(camera_scene);
            }
        }

        // ��Ⱦ�ı���
        std::cout << "��Ⱦ�ı���..." << std::endl;
        bool textbox_intersects = dirty_rect.intersects(textbox_rect);
        std::cout << "�ı������������: " << (textbox_intersects ? "��" : "��") << std::endl;

        
        

        if (idx_line < str_line_list.size() && textbox_intersects) {
            static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
            std::wstring wstr_line = convert.from_bytes(str_line_list[idx_line]);
            std::wstring wstr_completed = convert.from_bytes(str_line_list[idx_line].substr(0, idx_char));

            putimage_ex(camera_ui, &img_ui_textbox, &textbox_rect);

            settextcolor(RGB(125, 125, 125));
            outtextxy(185 + 2, textbox_rect.y + 65 + 2, wstr_line.c_str());

            settextcolor(RGB(25, 25, 25));
            outtextxy(185, textbox_rect.y + 65, wstr_line.c_str());

            settextcolor(RGB(0, 149, 217));
            outtextxy(185, textbox_rect.y + 65, wstr_completed.c_str());
        }


        // ���òü�����
        setcliprgn(NULL);
        DeleteObject(clip_region);
        std::cout << "���òü������ͷ���Դ" << std::endl;

        std::cout << "===== �����������Ⱦ =====" << std::endl << std::endl;
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
    std::vector<int> progressBackup;
    int progressArraySize;


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
private:
    //��ʱ��-�Զ����ߵ���
    Timer timer_step;
    std::mutex progress_mutex;
private:
    Rect dirty_rect;                      // ��ǰ֡��Ҫ�ػ������ 
    Rect prev_camera_rect;                // ��һ֡�������λ��
    std::vector<Rect> prev_player_rects;  // ��һ֡������ҵ�λ��
    // UI״̬����
    int prev_idx_line = -1; // ��һ֡���ı�������
    int prev_idx_char = -1; // ��һ֡���ı��ַ�����
};

