#include <iostream>
#include <lua.hpp>
#include <string>
#include <windows.h>

// 错误处理宏
#define LUA_CHECK(L, status) \
    if (status != LUA_OK) { \
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl; \
        lua_pop(L, 1); \
        return false; \
    }

// 初始化Lua环境
bool init_lua(lua_State* L) {
    // 加载标准库
    luaL_openlibs(L);

    // 添加项目根目录到package.path
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");

    std::string current_path = lua_tostring(L, -1);
    current_path += ";.\\?.lua;.\\scripts\\?.lua";

    lua_pushstring(L, current_path.c_str());
    lua_setfield(L, -3, "path");
    lua_pop(L, 2);  // 弹出package和原始path值

    return true;
}

// 执行Lua脚本
bool run_lua_script(lua_State* L, const char* script_name) {
    // 加载并运行控制脚本
    int status = luaL_dofile(L, script_name);
    if (status != LUA_OK) {
        std::cerr << "Failed to run " << script_name << ": "
            << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }

    return true;
}

int main() {
    // 1. 创建Lua状态机
    lua_State* L = luaL_newstate();
    if (!L) {
        std::cerr << "Failed to create Lua state" << std::endl;
        return 1;
    }

    // 2. 初始化Lua环境
    if (!init_lua(L)) {
        lua_close(L);
        return 1;
    }

    // 3. 执行控制脚本
    std::cout << "Executing control script..." << std::endl;
    if (!run_lua_script(L, "control.lua")) {
        lua_close(L);
        return 1;
    }

    // 4. 清理资源
    lua_close(L);

    std::cout << "\nControl script completed. Press Enter to exit...";
    std::cin.get();

    return 0;
}