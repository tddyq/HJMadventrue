#pragma once
#include<string>
#include<vector>
class INetworkAdapter
{
public:
     INetworkAdapter() = default;

     ~INetworkAdapter() = default;

    // ��ҵ�¼
    virtual void login() = 0;

    // ���ӷ�����
    virtual bool connect(const std::string& address, int port) = 0;

    // ������ҽ���
    virtual void sendProgress(int playerId, int progress) = 0;

    // ��ȡ������ҽ���
    virtual void getAllProgress() = 0;

    // ��ȡ��Ϸ�ı�
    virtual void getGameText(std::string str_text) = 0;
};

