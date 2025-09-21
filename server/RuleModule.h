#pragma once
#include "../thirdparty/httplib.h"
#include <vector>
#include"iostream"
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "../thirdparty/json.hpp"

#pragma comment(lib, "user32.lib")

using json = nlohmann::json;
class RuleModule
{
public:
    RuleModule(int size) : player_progress(size + 1, -1),
        progressArraySize(size + 1) {
        player_progress[progressArraySize - 1] = -1;

        init();
    }
    ~RuleModule() {
        if (L) {
            lua_close(L);
            L = nullptr;
        }
    }

	int init() {
        // 1. ����Lua״̬��
        L = luaL_newstate();
        if (!L) {
            std::cerr << "Failed to create Lua state" << std::endl;
            return 1;
        }
        std::cout << "Lua state created successfully" << std::endl;

        // ���ر�׼��
        luaL_openlibs(L);
        std::cout << "Lua standard libraries loaded" << std::endl;

        // 3. ���Lua�ű��ļ��Ƿ����
        std::ifstream file("rule.lua");
        if (!file.good()) {
            std::cerr << "Lua script file does not exist or cannot be accessed" << std::endl;
            lua_close(L);
            L = nullptr;
            return 1;
        }
        file.close();

        // 4. ����Lua�ű�
        std::cout << "Loading Lua script..." << std::endl;
        int result = luaL_dofile(L, "D:\\C++learning\\HJMadventrue++\\server\\rule.lua");
        if (result != LUA_OK) {
            // ��ȡ������Ϣ
            const char* errorMsg = lua_tostring(L, -1);
            if (errorMsg) {
                std::cerr << "Error loading Lua script: " << errorMsg << std::endl;
            }
            else {
                std::cerr << "Unknown error loading Lua script (error code: " << result << ")" << std::endl;
            }

            // ����������Ϣ
            lua_pop(L, 1);

            // �ر�Lua״̬��
            lua_close(L);
            L = nullptr;

            return 1;
        }
        std::cout << "Lua script loaded successfully" << std::endl;

        // 4. ����init����
        std::cout << "Calling Lua init function..." << std::endl;
        

        lua_getglobal(L, "init");
        if (!lua_isfunction(L, -1)) {
            std::cerr << "init function not found" << std::endl;
            lua_pop(L, 1);
            lua_close(L);
            L = nullptr;
            return 1;
        }
        lua_pushnumber(L, static_cast<lua_Number>(progressArraySize));
        lua_pushnumber(L, 1.0); // ����ʤ��Ȧ��Ϊ1

        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            const char* errorMsg = lua_tostring(L, -1);
            if (errorMsg) {
                std::cerr << "Error calling init: " << errorMsg << std::endl;
            }
            else {
                std::cerr << "Unknown error calling init" << std::endl;
            }

            // ����������Ϣ
            lua_pop(L, 1);

            // �ر�Lua״̬��
            lua_close(L);
            L = nullptr;

            return 1;
        }
        return 0; 
	}

    //int updatePlayerProcess(int player_id,const httplib::Request& req, httplib::Response& res) {
    //    // ��ȡ��ҵĽ���
    //    if (player_progress[player_id] != -1) {
    //        try {
    //            int new_progress = std::stoi(req.body);  // �����׳��쳣
    //
    //            
    //
    //            ////////////
    //            player_progress[player_id] = new_progress;  // ������ҽ���
    //            if (new_progress > player_progress[progressArraySize - 1]) 
    //                player_progress[progressArraySize - 1] = new_progress;
    //            ///////////////
    //            try {
    //                json player_progress_js = player_progress;
    //
    //                std::string response_content = player_progress_js.dump();
    //                res.set_content(response_content, "text/plain");
    //
    //                std::cout << "[UPDATE_SUCCESS] Player " << player_id
    //                    << " updated. Response: " << response_content << std::endl;
    //            }
    //            catch (const std::exception& e) {
    //                std::cerr << "[UPDATE_ERROR] JSON serialization failed: " << e.what() << std::endl;
    //                res.set_content("{\"error\":\"Serialization failed\"}", "application/json");
    //            }
    //        }
    //        catch (const std::invalid_argument& e) {
    //            std::cerr << "[UPDATE_ERROR] Invalid progress value: " << req.body
    //                << " - " << e.what() << std::endl;
    //            res.set_content("{\"error\":\"Invalid progress format\"}", "application/json");
    //        }
    //        catch (const std::out_of_range& e) {
    //            std::cerr << "[UPDATE_ERROR] Progress value out of range: " << req.body
    //                << " - " << e.what() << std::endl;
    //            res.set_content("{\"error\":\"Progress value too large\"}", "application/json");
    //        }
    //    }
    //    else {
    //        res.set_content("Player not found", "text/plain");
    //
    //        std::cout << "[UPDATE_WARN] Player " << player_id << " not found" << std::endl;
    //    }
    //    return 0;
    //}

    int updatePlayerProcess(int player_id, const httplib::Request& req, httplib::Response& res) {
        // ��ȡ��ҵĽ���
        if (player_progress[player_id] != -1) {
            try {
                int new_progress = std::stoi(req.body);  // �����׳��쳣

                // ��ӵ�����Ϣ
                std::cout << "Updating player " << player_id << " progress" << std::endl;

                // ���Lua״̬���Ƿ���Ч
                if (!L) {
                    std::cerr << "Lua state is not initialized" << std::endl;
                    res.set_content("{\"error\":\"Server error\"}", "application/json");
                    return 1;
                }

                ////////////
                player_progress[player_id] = new_progress;  // ������ҽ���
                /*if (new_progress > player_progress[progressArraySize - 1]) 
                player_progress[progressArraySize - 1] = new_progress;*/

                int winner = getLuaMessageToReturn(player_id, new_progress);
                if (winner != -1) {
                    player_progress[progressArraySize - 1] = winner;
                }
                std::cout << "Ӯ����: " << winner << std::endl;
                ///////////////
                try {
                    json player_progress_js = player_progress;

                    std::string response_content = player_progress_js.dump();
                    res.set_content(response_content, "text/plain");

                    std::cout << "[UPDATE_SUCCESS] Player " << player_id
                        << " updated. Response: " << response_content << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "[UPDATE_ERROR] JSON serialization failed: " << e.what() << std::endl;
                    res.set_content("{\"error\":\"Serialization failed\"}", "application/json");
                }
            }
            catch (const std::invalid_argument& e) {
                std::cerr << "[UPDATE_ERROR] Invalid progress value: " << req.body
                    << " - " << e.what() << std::endl;
                res.set_content("{\"error\":\"Invalid progress format\"}", "application/json");
            }
            catch (const std::out_of_range& e) {
                std::cerr << "[UPDATE_ERROR] Progress value out of range: " << req.body
                    << " - " << e.what() << std::endl;
                res.set_content("{\"error\":\"Progress value too large\"}", "application/json");
            }
        }
        else {
            res.set_content("Player not found", "text/plain");

            std::cout << "[UPDATE_WARN] Player " << player_id << " not found" << std::endl;
        }
        return 0;
    }


    int getLuaMessageToReturn(int player_id,int new_progress) {
        // ���Lua״̬���Ƿ���Ч
        if (!L) {
            std::cerr << "Lua state is not initialized" << std::endl;
            return -1;
        }

        lua_getglobal(L, "updateProcess");
        // ��ȡupdateProcess����
        lua_getglobal(L, "updateProcess");
        if (!lua_isfunction(L, -1)) {
            std::cerr << "updateProcess function not found" << std::endl;
            lua_pop(L, 1);
            return -1;
        }

        lua_pushnumber(L,(lua_Number)player_id);
        lua_pushnumber(L, (lua_Number)new_progress);
        
        if (lua_pcall(L, 2, 4, 0) != LUA_OK) {
            std::cerr << "error: " << lua_tostring(L, -1)<< std::endl;
            lua_pop(L, 1);
            return -1;
        }
        // ��ȡ�ĸ�����ֵ��winnerId, isGameOver, isInNewRound, newProgress
    // ע�⣺����ֵ��ջ�е�˳���ǵ���ģ���һ������ֵ��ջ��
        int winnerId = -1;
        bool isGameOver = false;
        bool isInNewRound = false;
        int newProgressValue = new_progress;

        if (lua_isnumber(L, -4)) {
            winnerId = static_cast<int>(lua_tonumber(L, -4));
        }

        if (lua_isboolean(L, -3)) {
            isGameOver = lua_toboolean(L, -3) != 0;
        }

        if (lua_isboolean(L, -2)) {
            isInNewRound = lua_toboolean(L, -2) != 0;
        }

        if (lua_isnumber(L, -1)) {
            newProgressValue = static_cast<int>(lua_tonumber(L, -1));
        }

        // ����ջ
        lua_pop(L, 4);

        // ��������»غϵ����
        if (isInNewRound) {
            player_progress[player_id] = newProgressValue;
        }

        // ����Ӯ��ID�����û��Ӯ����Ϊ-1��
        return winnerId;
    }
    // ��������Luaջ��ָ��λ�õı�����Ԫ����ȡ��������ֵ��vector
    //void getIntVectorFromLuaTable( int index) {
    //    
    //
    //    // 2. ���ָ��λ�õ�ֵ�Ƿ�Ϊ��
    //    if (!lua_istable(L, index)) {
    //        std::cerr << "Error: Expected a table at stack index " << index << std::endl;
    //        return ;
    //    }
    //
    //    // 3. ����һ��������nil����ʼ������
    //    lua_pushnil(L);
    //    
    //    int player_index = 0;
    //    while (lua_next(L, index) != 0) {
    //        // ����-2λ�ã�ֵ��-1λ��
    //
    //        // 4. ���ֵ�������Ƿ�Ϊ����
    //        if (lua_isinteger(L, -1)) {
    //            // �������������ȡ����ӵ�vector
    //            lua_Integer value = lua_tointeger(L, -1);
    //            player_progress[player_index] = value;
    //            player_index++;
    //        }
    //        // 5. ����ֵ��������������һ�ε���
    //        lua_pop(L, 1);
    //    }
    //
    //    return;
    //}

public:
    std::vector<int> player_progress;
    int progressArraySize;
private:
    lua_State* L;
	
};

