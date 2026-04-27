#include <Arduino.h>
#include "hw/bee.h" // 引入你的蜂鸣器头文件

extern "C"
{
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
}

// --- 1. 内存分配器：确保 Lua 使用 PSRAM ---
static void *l_alloc_esp32(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void)ud;
    (void)osize;
    if (nsize == 0)
    {
        free(ptr);
        return NULL;
    }
    else
    {
        // MALLOC_CAP_SPIRAM 确保优先或能够使用 8MB PSRAM
        void *p = heap_caps_realloc(ptr, nsize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        return p;
    }
}

// --- 2. 硬件接口封装 (C Wrapper) ---
// 包装蜂鸣器函数：Lua 调用方式 beep(频率, 毫秒)
static int l_play_beep(lua_State *L)
{
    int freq = (int)luaL_checkinteger(L, 1);     // 参数1
    int duration = (int)luaL_checkinteger(L, 2); // 参数2
    play_beep(freq, duration);                   // 调用你的 hw/bee.cpp
    return 0;                                    // 返回值个数为0
}

// --- 3. 自定义打印函数 (重定向 Lua print 到 Serial) ---
static int l_my_print(lua_State *L)
{
    int n = lua_gettop(L); // 获取参数个数
    for (int i = 1; i <= n; i++)
    {
        size_t len;
        const char *s = luaL_tolstring(L, i, &len); // 将参数转为字符串
        if (i > 1)
            Serial.print("\t");
        Serial.print(s);
        lua_pop(L, 1); // 弹出 tolstring 生成的副本
    }
    Serial.println();
    return 0;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- Lua 5.5 ESP32-S3 Start ---");

    // 初始化硬件
    bee_setup();

    // --- 4. 创建 Lua 状态机 (注入自定义分配器) ---
    lua_State *L = lua_newstate(l_alloc_esp32, NULL, 100);
    if (L == NULL)
    {
        Serial.println("Failed to create Lua state (Out of memory)");
        return;
    }

    // 打开基础库
    luaL_openlibs(L);

    // 注册自定义 print 函数 (覆盖 Lua 原生的 print)
    lua_register(L, "print", l_my_print);

    // 注册硬件控制函数
    lua_register(L, "beep", l_play_beep);

    // --- 5. 硬编码的 Lua 脚本 ---
    const char *lua_script =
        "print('Hello from Lua 5.5 on ESP32-S3!') "
        "print('Memory optimization: Check.') "
        "local f = 1000 "
        "for i = 1, 3 do "
        "  print('Beeping at ' .. f .. 'Hz') "
        "  beep(f, 200) "
        "  f = f + 500 "
        "end "
        "print('Lua script execution finished.') ";

    // --- 6. 运行脚本 ---
    int status = luaL_dostring(L, lua_script);
    if (status != LUA_OK)
    {
        const char *err = lua_tostring(L, -1);
        Serial.printf("Lua Error: %s\n", err);
    }

    // 释放虚拟机 (如果你不需要常驻)
    // lua_close(L);
}

void loop()
{
    // 保持运行
    delay(1000);
}