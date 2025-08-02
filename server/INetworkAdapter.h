#pragma once
#include<string>
#include<vector>
class INetworkAdapter
{
public:
     INetworkAdapter() = default;

     ~INetworkAdapter() = default;

    // 玩家登录
    virtual void login() = 0;

    // 连接服务器
    virtual bool connect(const std::string& address, int port) = 0;

    // 发送玩家进度
    virtual void sendProgress(int playerId, int progress) = 0;

    // 获取所有玩家进度
    virtual void getAllProgress() = 0;

    // 获取游戏文本
    virtual void getGameText(std::string str_text) = 0;
};

