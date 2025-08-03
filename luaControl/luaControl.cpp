#include <iostream>
#include <lua.hpp>
#include <string>
#include <windows.h>

// �������
#define LUA_CHECK(L, status) \
    if (status != LUA_OK) { \
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl; \
        lua_pop(L, 1); \
        return false; \
    }

// ��ʼ��Lua����
bool init_lua(lua_State* L) {
    // ���ر�׼��
    luaL_openlibs(L);

    // �����Ŀ��Ŀ¼��package.path
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");

    std::string current_path = lua_tostring(L, -1);
    current_path += ";.\\?.lua;.\\scripts\\?.lua";

    lua_pushstring(L, current_path.c_str());
    lua_setfield(L, -3, "path");
    lua_pop(L, 2);  // ����package��ԭʼpathֵ

    return true;
}

// ִ��Lua�ű�
bool run_lua_script(lua_State* L, const char* script_name) {
    // ���ز����п��ƽű�
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
    // 1. ����Lua״̬��
    lua_State* L = luaL_newstate();
    if (!L) {
        std::cerr << "Failed to create Lua state" << std::endl;
        return 1;
    }

    // 2. ��ʼ��Lua����
    if (!init_lua(L)) {
        lua_close(L);
        return 1;
    }

    // 3. ִ�п��ƽű�
    std::cout << "Executing control script..." << std::endl;
    if (!run_lua_script(L, "control.lua")) {
        lua_close(L);
        return 1;
    }

    // 4. ������Դ
    lua_close(L);

    std::cout << "\nControl script completed. Press Enter to exit...";
    std::cin.get();

    return 0;
}