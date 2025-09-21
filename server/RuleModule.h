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
        // 1. 创建Lua状态机
        L = luaL_newstate();
        if (!L) {
            std::cerr << "Failed to create Lua state" << std::endl;
            return 1;
        }
        std::cout << "Lua state created successfully" << std::endl;

        // 加载标准库
        luaL_openlibs(L);
        std::cout << "Lua standard libraries loaded" << std::endl;

        // 3. 检查Lua脚本文件是否存在
        std::ifstream file("rule.lua");
        if (!file.good()) {
            std::cerr << "Lua script file does not exist or cannot be accessed" << std::endl;
            lua_close(L);
            L = nullptr;
            return 1;
        }
        file.close();

        // 4. 加载Lua脚本
        std::cout << "Loading Lua script..." << std::endl;
        int result = luaL_dofile(L, "D:\\C++learning\\HJMadventrue++\\server\\rule.lua");
        if (result != LUA_OK) {
            // 获取错误信息
            const char* errorMsg = lua_tostring(L, -1);
            if (errorMsg) {
                std::cerr << "Error loading Lua script: " << errorMsg << std::endl;
            }
            else {
                std::cerr << "Unknown error loading Lua script (error code: " << result << ")" << std::endl;
            }

            // 弹出错误信息
            lua_pop(L, 1);

            // 关闭Lua状态机
            lua_close(L);
            L = nullptr;

            return 1;
        }
        std::cout << "Lua script loaded successfully" << std::endl;

        // 4. 调用init函数
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
        lua_pushnumber(L, 1.0); // 设置胜利圈数为1

        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            const char* errorMsg = lua_tostring(L, -1);
            if (errorMsg) {
                std::cerr << "Error calling init: " << errorMsg << std::endl;
            }
            else {
                std::cerr << "Unknown error calling init" << std::endl;
            }

            // 弹出错误信息
            lua_pop(L, 1);

            // 关闭Lua状态机
            lua_close(L);
            L = nullptr;

            return 1;
        }
        return 0; 
	}

    //int updatePlayerProcess(int player_id,const httplib::Request& req, httplib::Response& res) {
    //    // 获取玩家的进度
    //    if (player_progress[player_id] != -1) {
    //        try {
    //            int new_progress = std::stoi(req.body);  // 可能抛出异常
    //
    //            
    //
    //            ////////////
    //            player_progress[player_id] = new_progress;  // 更新玩家进度
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
        // 获取玩家的进度
        if (player_progress[player_id] != -1) {
            try {
                int new_progress = std::stoi(req.body);  // 可能抛出异常

                // 添加调试信息
                std::cout << "Updating player " << player_id << " progress" << std::endl;

                // 检查Lua状态机是否有效
                if (!L) {
                    std::cerr << "Lua state is not initialized" << std::endl;
                    res.set_content("{\"error\":\"Server error\"}", "application/json");
                    return 1;
                }

                ////////////
                player_progress[player_id] = new_progress;  // 更新玩家进度
                /*if (new_progress > player_progress[progressArraySize - 1]) 
                player_progress[progressArraySize - 1] = new_progress;*/

                int winner = getLuaMessageToReturn(player_id, new_progress);
                if (winner != -1) {
                    player_progress[progressArraySize - 1] = winner;
                }
                std::cout << "赢家是: " << winner << std::endl;
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
        // 检查Lua状态机是否有效
        if (!L) {
            std::cerr << "Lua state is not initialized" << std::endl;
            return -1;
        }

        lua_getglobal(L, "updateProcess");
        // 获取updateProcess函数
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
        // 获取四个返回值：winnerId, isGameOver, isInNewRound, newProgress
    // 注意：返回值在栈中的顺序是倒序的，第一个返回值在栈底
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

        // 清理栈
        lua_pop(L, 4);

        // 处理进入新回合的情况
        if (isInNewRound) {
            player_progress[player_id] = newProgressValue;
        }

        // 返回赢家ID（如果没有赢家则为-1）
        return winnerId;
    }
    // 函数：从Lua栈中指定位置的表（包括元表）提取所有整数值到vector
    //void getIntVectorFromLuaTable( int index) {
    //    
    //
    //    // 2. 检查指定位置的值是否为表
    //    if (!lua_istable(L, index)) {
    //        std::cerr << "Error: Expected a table at stack index " << index << std::endl;
    //        return ;
    //    }
    //
    //    // 3. 将第一个键推入nil，开始遍历表
    //    lua_pushnil(L);
    //    
    //    int player_index = 0;
    //    while (lua_next(L, index) != 0) {
    //        // 键在-2位置，值在-1位置
    //
    //        // 4. 检查值的类型是否为整数
    //        if (lua_isinteger(L, -1)) {
    //            // 如果是整数，获取并添加到vector
    //            lua_Integer value = lua_tointeger(L, -1);
    //            player_progress[player_index] = value;
    //            player_index++;
    //        }
    //        // 5. 弹出值，保留键用于下一次迭代
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

