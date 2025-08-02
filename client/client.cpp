#include"../thirdparty/httplib.h"
#include "Path.h"                  // 自定义路径处理模块
#include "Player.h"                // 自定义播放器模块

// 标准库依赖
#include <chrono>                  // 时间处理
#include <string>                  // 字符串操作
#include <vector>                  // 动态数组容器
#include <thread>                  // 多线程支持
#include <codecvt>                 // 字符编码转换（注：C++17后弃用）
#include <fstream>                 // 文件流操作
#include <sstream>                 // 字符串流操作

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

enum class Stage {
	Waiting,
	Ready,
	Racing
};
int val_countdown = 4;               //起跑倒计时
Stage stage = Stage::Waiting;        //当前游戏状态

int id_player = 0;                   //玩家序号
std::atomic<int> progress_1 = -1;    //玩家1进度
std::atomic<int> progress_2 = -1;    //玩家2进度
int num_total_char = 0;              //全部字符数

Path path = Path({
	{842,842},{1322,842},{1322,442},
	{2762,442},{2762,842},{3162,842},
	{3162,1722},{2122,1722},{2122,1562},
	{842,1562},{842,842}
	});

int idx_line = 0;                        //当前文本行索引
int idx_char = 0;                        //当前行文本字符索引
std::string str_text;                    //文本内容
std::vector<std::string> str_line_list;  //行文本列表

std::string str_address;              //服务器地址
httplib::Client* client = nullptr;    //http客户端对象

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

void load_resources(HWND hwnd)
{
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
void login_to_server(HWND hwnd) {
    check_server_connection(hwnd); // 先检查基础连接

    client = new httplib::Client("127.0.0.1", 25565);
    client->set_keep_alive(true);

    httplib::Result result = client->Post("/login");
 
    if (!result) {
        // 添加详细的错误信息
        std::string error_msg = "HTTP错误: ";
        error_msg += httplib::to_string(result.error());
        error_msg += "\n尝试连接: 127.0.0.1:25565";

        // 转换为宽字符串
        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, error_msg.c_str(), -1, NULL, 0);
        wchar_t* wstr = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8, 0, error_msg.c_str(), -1, wstr, wchars_num);

        MessageBox(hwnd, wstr, _T("连接失败"), MB_OK | MB_ICONERROR);
        delete[] wstr;
        exit(-1);
    }


    if (!result || result->status != 200) {
        MessageBox(hwnd, _T("无法连接到服务器"), _T("拒绝加入"), MB_OK | MB_ICONERROR);
        exit(-1);
    }

    id_player = std::stoi(result->body);

    if (id_player <= 0) {
        MessageBox(hwnd, _T("比赛已经开始啦"), _T("拒绝加入"), MB_OK | MB_ICONERROR);
        exit(-1);
    }

    (id_player == 1) ? (progress_1 = 0) : (progress_2 = 0);
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

    std::thread([&]() {
        while (true) {
            using namespace std::chrono;

            std::string route = (id_player == 1) ? "/update_1" : "/update_2";
            //将当前玩家的进度值（progress_1 或 progress_2）转换为字符串，并作为请求的主体发送
            std::string body = std::to_string((id_player == 1) ? progress_1 : progress_2);
            httplib::Result result = client->Post(route, body, "text/plain");

            if (result && result->status == 200) {
                //如果服务器返回成功（状态码为 200），解析返回的进度值并更新相应玩家的进度
                int progress = std::stoi(result->body);
                (id_player == 1) ? (progress_2 = progress) : (progress_1 = progress);
            }
            //每 0.1 秒发送一次请求
            std::this_thread::sleep_for(nanoseconds(1000000000 / 10));
        }
        }).detach();
}

int main(int argc, char** argv) {
	////////////////////////处理数据初始化///////////////////////
    using namespace std::chrono;
    
    HWND hwnd = initgraph(1280, 720/*,EW_SHOWCONSOLE*/);
    SetWindowText(hwnd, _T("哈基米大冒险!"));
    settextstyle(28, 0, _T("IPix"));

    setbkmode(TRANSPARENT);

    load_resources(hwnd);
    login_to_server(hwnd);

    // 变量声明部分（来自第1张图）
    ExMessage msg;
    Timer timer_countdown;
    Camera camera_ui, camera_scene;

    // 玩家初始化（来自第1张图）
    Player player_1(&atlas_1P_idle_up, &atlas_1P_idle_down, &atlas_1P_idle_left, &atlas_1P_idle_right,
        &atlas_1P_run_up, &atlas_1P_run_down, &atlas_1P_run_left, &atlas_1P_run_right);

    Player player_2(&atlas_2P_idle_up, &atlas_2P_idle_down, &atlas_2P_idle_left, &atlas_2P_idle_right,
        &atlas_2P_run_up, &atlas_2P_run_down, &atlas_2P_run_left, &atlas_2P_run_right);

    // 初始化设置（来自第1张图）
    camera_ui.set_size({ 1280, 720 });
    camera_scene.set_size({ 1280, 720 });
    player_1.set_position({ 842, 842 });
    player_2.set_position({ 842, 842 });

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

    // 帧率控制设置（来自第3张图）
    const nanoseconds frame_duration(1000000000 / 144);
    steady_clock::time_point last_tick = steady_clock::now();
    BeginBatchDraw();  // 开始批量绘制操作
    while (true) {
        ////////////////处理玩家输入///////////////
        while (peekmessage(&msg)) {
            if (stage != Stage::Racing)continue;

            if (msg.message == WM_CHAR && idx_line < str_line_list.size()) {
                const std::string& str_line = str_line_list[idx_line];
                if (str_line[idx_char] == msg.ch) {
                    
                    //调试
                    std::cout << "输入正确,更新progress" << std::endl;

                    switch (rand() % 4) {
                    case 0:play_audio(_T("click_1")); break;
                    case 1:play_audio(_T("click_2")); break;
                    case 2:play_audio(_T("click_3")); break;
                    case 3:play_audio(_T("click_4")); break;
                    }

                    (id_player == 1) ? progress_1++ : progress_2++;

                    idx_char++;
                    if (idx_char >= str_line.length()) {
                        idx_char = 0;
                        idx_line++;
                    }
                }
            }
        }
        /////////////////处理游戏更新////////////////
        steady_clock::time_point frame_start = steady_clock::now();
        duration<float> delta = duration<float>(frame_start - last_tick);
        
        //调试
        std::cout << "delta:" << delta.count() << std::endl;

        if (stage == Stage::Waiting) {
            //调试
            std:: cout << "等待中" << std::endl;
            
            if (progress_1 >= 0 && progress_2 >= 0)
                stage = Stage::Ready;
        }
        else
        {
            if (stage == Stage::Ready)
                timer_countdown.on_update(delta.count());

            if ((id_player == 1 && progress_1 >= num_total_char)
                || (id_player == 2 && progress_2 >= num_total_char)) {
                stop_audio(_T("bgm"));
                play_audio((id_player == 1) ? _T("1p_win") : _T("2p_win"));
                MessageBox(hwnd, _T("赢麻麻"), _T("游戏结束"), MB_OK | MB_ICONINFORMATION);
                exit(0);
            }
            else if ((id_player == 1 && progress_2 >= num_total_char)
                || (id_player == 2 && progress_1 >= num_total_char)) {
                stop_audio(_T("bgm"));
                MessageBox(hwnd, _T("输光光"), _T("游戏结束"), MB_OK | MB_ICONINFORMATION);
                exit(0);
            }

            player_1.set_target(path.get_position_at_progress((float)progress_1 / num_total_char));
            player_2.set_target(path.get_position_at_progress((float)progress_2 / num_total_char));

            player_1.on_update(delta.count());
            player_2.on_update(delta.count());

            camera_scene.look_at((id_player == 1)
                ? player_1.get_position() : player_2.get_position());


            //调试
            std::cout << "stage:" << (stage==Stage::Racing) << std::endl;

            ///////////////渲染部分/////////////
            setbkcolor(RGB(0, 0, 0));
            cleardevice();

            if (stage == Stage::Waiting) {
                settextcolor(RGB(195, 195, 195));
                outtextxy(15, 675, _T("比赛即将开始,等待其他玩家加入"));
                //调试
                std::cout << "已绘制等待" << std::endl;
            }
            else {
                //绘制背景图
                static const Rect rect_bg = {
                    0,0,
                    img_background.getwidth(),
                    img_background.getheight()
                };
                putimage_ex(camera_scene, &img_background, &rect_bg);

                //绘制玩家
                if (player_1.get_position().y > player_2.get_position().y) {
                    //调试
                    std::cout << "绘制玩家" << std::endl;

                    player_2.on_render(camera_scene);
                    player_1.on_render(camera_scene);
                }
                else {
                    player_1.on_render(camera_scene);
                    player_2.on_render(camera_scene);
                }
                //绘制倒计时
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
                default:break;
                }
                //绘制界面
                if (stage == Stage::Racing) {
                    static const Rect rect_textbox = {
                        0,
                        720 - img_ui_textbox.getheight(),
                        img_ui_textbox.getwidth(),
                        img_ui_textbox.getheight()
                    };
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
                }

            }
        }
       
        FlushBatchDraw();

        last_tick = frame_start;
        nanoseconds sleep_duration = frame_duration - (steady_clock::now() - frame_start);
        if (sleep_duration > nanoseconds(0))
            std::this_thread::sleep_for(sleep_duration);

    }
    EndBatchDraw();

	return 0;
}
