#pragma once
#include"../thirdparty/httplib.h"
#include"../thirdparty/json.hpp"
#include"Atlas.h"
#include "Path.h"
#include"util.h"
#include "Player.h"
#include"IClient.h"

#include <chrono>                  // 时间处理
#include <string>                  // 字符串操作
#include <vector>                  // 动态数组容器
#include <thread>                  // 多线程支持
#include <codecvt>                 // 字符编码转换（注：C++17后弃用）
#include <fstream>                 // 文件流操作
#include <sstream>                 // 字符串流操作



using json = nlohmann::json;

class CClient:public IClient
{
public:
    CClient(int size) : player_progress(size + 1, -1), players(size+1), progressArraySize(size + 1) {
        init(); 
    }
    ~CClient() = default;

    void login_to_server()override {
        check_server_connection(hwnd); // 先检查基础连接
        
        //调试
        std::cout << "address: " << str_address << "post: " << post << std::endl;

        client = new httplib::Client(str_address, post);
        client->set_keep_alive(true);

        httplib::Result result = client->Post("/login");

        if (!result) {
            // 添加详细的错误信息
            std::string error_msg = "HTTP错误: ";
            error_msg += httplib::to_string(result.error());
            error_msg += "\n尝试连接: "+ str_address + " : " + std::to_string(post);

            // 转换为宽字符串
            int wchars_num = MultiByteToWideChar(CP_UTF8, 0, error_msg.c_str(), -1, NULL, 0);
            wchar_t* wstr = new wchar_t[wchars_num];
            MultiByteToWideChar(CP_UTF8, 0, error_msg.c_str(), -1, wstr, wchars_num);

            //调试
            std::cout << "连接失败: " + error_msg << std::endl;

            MessageBox(hwnd, wstr, _T("连接失败"), MB_OK | MB_ICONERROR);
            delete[] wstr;
            exit(-1);
        }

        if (!result || result->status != 200) {
            //调试
            std::cout << "无法连接到服务器,拒绝加入" << std::endl;

            MessageBox(hwnd, _T("无法连接到服务器"), _T("拒绝加入"), MB_OK | MB_ICONERROR);
            exit(-1);
        }

        id_player = std::stoi(result->body);

        if (id_player < 0) {
            MessageBox(hwnd, _T("比赛已经开始啦"), _T("拒绝加入"), MB_OK | MB_ICONERROR);
            exit(-1);
        }
        player_progress[progressArraySize - 1] = 0;
        player_progress[id_player] = 0;
        progressBackup = 0;

        //调试
        std::cout << "服务器连接成功" << std::endl;

        //发送 POST 请求到 /query_text 路径，并获取服务器返回的文本内容
        str_text = client->Post("/query_text")->body;
        //使用 std::stringstream 将 str_text 作为输入流进行逐行读取。
        //每读取一行文本，存储到 str_line_list 中，并统计每行的字符数,累加到 num_total_char
        std::stringstream str_stream(str_text);
        std::string str_line;
        while (std::getline(str_stream, str_line)) {
            str_line_list.push_back(str_line);
            num_total_char += (int)str_line.length();
        }
        //调试
        std::cout <<"字符总数: " << num_total_char << std::endl;


        uploadProgressToServer();
    }
    void uploadProgressToServer()override {
        std::thread([&]() {
            try {
                while (true) {
                    using namespace std::chrono;
                    //为每一个服务器确定路由
                    std::string route = "/update_" + std::to_string(id_player);
                    std::string body = std::to_string(player_progress[id_player]);
                    //打印调试
                    std::cout << "Sending request to route: " << route << " with body: " << body << std::endl;
                   
                    httplib::Result result = client->Post(route, body, "text/plain");
                    //打印调试
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
            //如果服务器返回成功（状态码为 200），解析返回的进度值并更新相应玩家的进度
            updateProgressFromJson(result->body);
        }
    }
    void getProgressFromServer() {

    }
    void load_resources()override {
        
        
        // 加载字体资源
        AddFontResourceEx(_T("resources/IPix.ttf"), FR_PRIVATE, NULL);

        // 玩家1动画资源
        atlas_1P_idle_up.load(_T("resources/hajimi_idle_back_%d.png"), 4);
        atlas_1P_idle_down.load(_T("resources/hajimi_idle_front_%d.png"), 4);
        atlas_1P_idle_left.load(_T("resources/hajimi_idle_left_%d.png"), 4);
        atlas_1P_idle_right.load(_T("resources/hajimi_idle_right_%d.png"), 4);
        atlas_1P_run_up.load(_T("resources/hajimi_run_back_%d.png"), 4);
        atlas_1P_run_down.load(_T("resources/hajimi_run_front_%d.png"), 4);
        atlas_1P_run_left.load(_T("resources/hajimi_run_left_%d.png"), 4);
        atlas_1P_run_right.load(_T("resources/hajimi_run_right_%d.png"), 4);

        // 玩家2动画资源
        atlas_2P_idle_up.load(_T("resources/manbo_idle_back_%d.png"), 4);
        atlas_2P_idle_down.load(_T("resources/manbo_idle_front_%d.png"), 4);
        atlas_2P_idle_left.load(_T("resources/manbo_idle_left_%d.png"), 4);
        atlas_2P_idle_right.load(_T("resources/manbo_idle_right_%d.png"), 4);
        atlas_2P_run_up.load(_T("resources/manbo_run_back_%d.png"), 4);
        atlas_2P_run_down.load(_T("resources/manbo_run_front_%d.png"), 4);
        atlas_2P_run_left.load(_T("resources/manbo_run_left_%d.png"), 4);
        atlas_2P_run_right.load(_T("resources/manbo_run_right_%d.png"), 4);
        

        // 加载界面图片资源
        loadimage(&img_ui_1, _T("resources/ui_1.png"));
        loadimage(&img_ui_2, _T("resources/ui_2.png"));
        loadimage(&img_ui_3, _T("resources/ui_3.png"));
        loadimage(&img_ui_fight, _T("resources/ui_fight.png"));
        loadimage(&img_ui_textbox, _T("resources/ui_textbox.png"));
        loadimage(&img_background, _T("resources/background.png"));

        // 背景音乐
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
            MessageBox(hwnd, L"无法打开配置 config.cfg", L"启动失败", MB_OK | MB_ICONERROR);
            exit(-1);
        }
        // 创建一个字符串流，用于将文件内容读取到内存中
        std::stringstream str_stream;
        str_stream << file.rdbuf();
        str_address = str_stream.str();

        file.close();
    }
    void init()override {
        HWND hwnd = initgraph(1280, 720/*,EW_SHOWCONSOLE*/);
        SetWindowText(hwnd, _T("哈基米大冒险!"));
        settextstyle(28, 0, _T("IPix"));

        setbkmode(TRANSPARENT);

        load_resources();
        login_to_server();

        //初始化所有角色动画与位置
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

        // 初始化设置
        camera_ui.set_size({ 1280, 720 });
        camera_scene.set_size({ 1280, 720 });
        

        // 倒计时定时器设置（来自第2张图）
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
                    play_audio(_T("bgm"), true); // true表示循环播放
                    break;
                }
            });

        post = 25565;
    }
    void mainLoop()override {
        using namespace std::chrono;
        // 帧率控制设置
        const std::chrono::nanoseconds frame_duration(1000000000 / 144);
        steady_clock::time_point last_tick = steady_clock::now();
        BeginBatchDraw();  // 开始批量绘制操作
        while(true){
            /////////////////处理玩家输入////////////////
            processPlayerInput();
            /////////////////处理游戏更新////////////////
            steady_clock::time_point frame_start = steady_clock::now();
            duration<float> delta = duration<float>(frame_start - last_tick);

            //调试
            std::cout << "delta:" << delta.count() << std::endl;
            processGameUpdate(delta.count());

            ////////////////处理游戏渲染/////////////////
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
            MessageBox(hwnd, _T("WSAStartup 失败"), _T("网络错误"), MB_OK | MB_ICONERROR);
            exit(-1);
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            MessageBox(hwnd, _T("创建 socket 失败"), _T("网络错误"), MB_OK | MB_ICONERROR);
            WSACleanup();
            exit(-1);
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(25565);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        // 设置超时
        DWORD timeout = 3000; // 3秒
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        if (connect(sock, (sockaddr*)&addr, sizeof(addr))) {
            int error = WSAGetLastError();
            std::wstring msg = L"无法连接到服务器\n";
            msg += L"IP: 127.0.0.1\n";
            msg += L"端口: 25565\n\n";
            msg += L"错误代码: " + std::to_wstring(error) + L"\n";

            switch (error) {
            case WSAECONNREFUSED: msg += L"连接被拒绝 (服务器未运行?)"; break;
            case WSAETIMEDOUT: msg += L"连接超时"; break;
            case WSAEADDRNOTAVAIL: msg += L"地址不可用"; break;
            default: msg += L"未知网络错误";
            }

            MessageBox(hwnd, msg.c_str(), _T("连接测试失败"), MB_OK | MB_ICONERROR);
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
            // 解析 JSON 字符串
            json jsonData = json::parse(body);

            // SON 格式是一个数组
            if (jsonData.is_array()) {
                std::vector<int>  newProgresses = jsonData.get<std::vector<int>>();
                
                if (newProgresses.size() == progressArraySize) {
                    player_progress = std::move(newProgresses);
                    //如果数据出错使得服务器返回当前进度小于真实进度则恢复
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
        ////////////////处理玩家输入///////////////
        try {
            while (peekmessage(&msg)) {
                if (stage != Stage::Racing) continue;

                if (msg.message == WM_CHAR && idx_line < str_line_list.size()) {
                    const std::string& str_line = str_line_list[idx_line];
                    if (str_line[idx_char] == msg.ch) {

                        //调试
                        std::cout << "输入正确,更新progress" << std::endl;

                        switch (rand() % 4) {
                        case 0: play_audio(_T("click_1")); break;
                        case 1: play_audio(_T("click_2")); break;
                        case 2: play_audio(_T("click_3")); break;
                        case 3: play_audio(_T("click_4")); break;
                        }

                        // 打印当前进度信息
                        std::cout << "Player progress before increment: " << player_progress[id_player] << std::endl;

                        player_progress[id_player]++;
                        progressBackup++;

                        // 打印更新后的进度
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
            // 调试入口
            std::cout << "===== 开始处理游戏更新 delta=" << delta << " =====" << std::endl;

            if (stage == Stage::Waiting) {
                
                for (int i = 0; i < progressArraySize - 1; i++) {
                    if (player_progress[i] != 0) {
                        
                        return;
                    }
                }
                stage = Stage::Ready;
                std::cout << "状态变更: Waiting -> Ready" << std::endl;
            }
            else
            {
                std::cout << "当前状态: " << ((stage == Stage::Ready) ? "Ready" : "其他") << std::endl;

                if (stage == Stage::Ready)
                    timer_countdown.on_update(delta);

                // 调试游戏结束条件
                std::cout << "进度检查: 备份进度=" << progressBackup <<" 字符总数: "<<num_total_char<< std::endl;
                if (progressBackup >= num_total_char) {
                    
                    stop_audio(_T("bgm"));
                    play_audio((id_player == 1) ? _T("1p_win") : _T("2p_win"));
                    MessageBox(hwnd, _T("赢麻麻"), _T("游戏结束"), MB_OK | MB_ICONINFORMATION);
                    exit(0);
                }
                else if (player_progress[progressArraySize - 1] >= num_total_char) {
                    
                    stop_audio(_T("bgm"));
                    MessageBox(hwnd, _T("输光光"), _T("游戏结束"), MB_OK | MB_ICONINFORMATION);
                    exit(0);
                }

                // 调试玩家位置更新
                std::cout << "更新玩家目标位置 | 玩家数量=" << progressArraySize - 1 << std::endl;
                for (int i = 0; i < progressArraySize - 1; i++) {
                    float progress = (float)player_progress[i] / num_total_char;
                    players[i].set_target(path.get_position_at_progress(progress));
                    
                    
                    std::cout << "玩家" << i << " 进度=" << player_progress[i]<< " 归一化=" << progress << std::endl;
                    
                   
                }

                // 调试玩家更新
                std::cout << "执行玩家更新" << std::endl;

                for (int i = 0; i < progressArraySize - 1; i++) {
                    players[i].on_update(delta);

                    std::cout << "玩家" << i << " 位置更新完成" << std::endl;
                }

                // 调试摄像机更新
                Vector2 camPos = players[id_player].get_position();
                
                camera_scene.look_at(camPos);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "!!! 异常捕获: " << e.what() << std::endl;
            
        }
        catch (...) {
            std::cerr << "!!! 未知类型异常捕获" << std::endl;
        }

        std::cout << "===== 结束游戏更新处理 =====" << std::endl;
    }
    void processGameRander() {
        std::cout << "[DEBUG] Entering processGameRander" << std::endl;

        try {
            std::cout << "stage:" << (stage == Stage::Racing) << std::endl;

            ///////////////渲染部分/////////////
            setbkcolor(RGB(0, 0, 0));
            cleardevice();

            if (stage == Stage::Waiting) {
                std::cout << "[DEBUG] Rendering waiting stage" << std::endl;
                settextcolor(RGB(195, 195, 195));
                outtextxy(15, 675, _T("比赛即将开始,等待其他玩家加入"));
                //调试
                std::cout << "已绘制等待" << std::endl;
            }
            else {
                std::cout << "[DEBUG] Rendering racing stage" << std::endl;

                //绘制背景图
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

                //绘制玩家
                try {
                    for (int i = 0; i < progressArraySize - 1; i++) {
                        players[i].on_render(camera_scene);
                    }
                    std::cout << "[DEBUG] Players rendered successfully" << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "[ERROR] Player rendering failed: " << e.what() << std::endl;
                }

                //绘制倒计时
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

                //绘制界面
                if (stage == Stage::Racing) {
                    try {
                        std::cout << "[DEBUG] Rendering racing UI" << std::endl;
                        static const Rect rect_textbox = {
                            0,
                            720 - img_ui_textbox.getheight(),
                            img_ui_textbox.getwidth(),
                            img_ui_textbox.getheight()
                        };

                        // 字符串转换可能抛出异常
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
    // 玩家1动画图集
    Atlas atlas_1P_idle_up;     //玩家1向上闲置动画图集
    Atlas atlas_1P_idle_down;   //玩家1向下闲置动画图集
    Atlas atlas_1P_idle_left;   //玩家1向左闲置动画图集
    Atlas atlas_1P_idle_right;  //玩家1向右闲置动画图集
    Atlas atlas_1P_run_up;      //玩家1向上奔跑动画图集
    Atlas atlas_1P_run_down;    //玩家1向下奔跑动画图集
    Atlas atlas_1P_run_left;    //玩家1向左奔跑动画图集
    Atlas atlas_1P_run_right;   //玩家1向右奔跑动画图集

    // 玩家2动画图集
    Atlas atlas_2P_idle_up;     //玩家2向上闲置动画图集
    Atlas atlas_2P_idle_down;   //玩家2向下闲置动画图集
    Atlas atlas_2P_idle_left;   //玩家2向左闲置动画图集
    Atlas atlas_2P_idle_right;  //玩家2向右闲置动画图集
    Atlas atlas_2P_run_up;      //玩家2向上奔跑动画图集
    Atlas atlas_2P_run_down;    //玩家2向下奔跑动画图集
    Atlas atlas_2P_run_left;    //玩家2向左奔跑动画图集
    Atlas atlas_2P_run_right;   //玩家2向右奔跑动画图集

    // 界面图像资源
    IMAGE img_ui_1;           //界面文本1
    IMAGE img_ui_2;           //界面文本2
    IMAGE img_ui_3;           //界面文本3
    IMAGE img_ui_fight;       //界面文本FIGHT
    IMAGE img_ui_textbox;     //界面文本框
    IMAGE img_background;     //背景图
private:
    //具体数据
    int val_countdown = 4;               //起跑倒计时
    Stage stage = Stage::Waiting;        //当前游戏状态

    std::vector<int> player_progress;
    int progressArraySize;
    int progressBackup;

    int num_total_char = 0;              //全部字符数
    int id_player = 0;

    int idx_line = 0;                        //当前文本行索引
    int idx_char = 0;                        //当前行文本字符索引
    std::string str_text;                    //文本内容
    std::vector<std::string> str_line_list;  //行文本列表

private:
    //服务器相关
    std::string str_address;              //服务器地址
    httplib::Client* client = nullptr;    //http客户端对象
    std::string domain;                   //域名
    int post = 25565;                     //端口
private:
    HWND hwnd;                            //窗口句柄
    std::vector<Player> players;

    ExMessage msg;
    Timer timer_countdown;
    Camera camera_ui, camera_scene;
};

