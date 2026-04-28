#include <Arduino.h>
#include "hw/bee.h"
#include "hw/sd.h" // 确保包含你的 SD 初始化头文件

extern "C"
{
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
}

// 内存分配器（保持不变，利用 PSRAM）
static void *l_alloc_esp32(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void)ud;
    (void)osize;
    if (nsize == 0)
    {
        free(ptr);
        return NULL;
    }
    return heap_caps_realloc(ptr, nsize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
}

// 串口打印重定向
static int l_my_print(lua_State *L)
{
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++)
    {
        size_t len;
        const char *s = luaL_tolstring(L, i, &len);
        if (i > 1)
            Serial.print("\t");
        Serial.print(s);
        lua_pop(L, 1);
    }
    Serial.println();
    return 0;
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

void setup()
{
    Serial.begin(115200);
    delay(1000);
    // 1. 初始化硬件
    bee_setup();
    if (!init_SD())
    {
        Serial.println("无法继续，SD卡异常");
        return;
    }
    printSDInfo();
    // 2. 创建 Lua 状态机
    lua_State *L = lua_newstate(l_alloc_esp32, NULL, (unsigned int)esp_random());
    if (L == NULL)
        return;
    luaL_openlibs(L);
    lua_register(L, "print", l_my_print);
    // 这里建议也注册你的 beep 函数...
    // 注册硬件控制函数
    lua_register(L, "beep", l_play_beep);
    // 3. --- 配置 require 搜索路径 (关键步骤) ---
    // 告诉 Lua require("mymod") 时去 /sdcard/mymod.lua 找
    const char *set_path = "package.path = '/sdcard/?.lua;' .. package.path";
    luaL_dostring(L, set_path);

    // 4. --- 从 SD 卡加载并运行 main.lua ---
    // 注意：Lua 5.5 的 luaL_dofile 内部使用标准 C 的 fopen
    // ESP32 的 SD_MMC 默认挂载点如果是 "/sdcard"，则路径需匹配
    const char *main_file = "/sdcard/main.lua";
    Serial.printf("Loading %s ...\n", main_file);
    int status = luaL_dofile(L, main_file);

    if (status != LUA_OK)
    {
        const char *err = lua_tostring(L, -1);
        Serial.printf("❌ Lua 运行错误: %s\n", err);
    }
    else
    {
        Serial.println("✅ Lua 脚本执行完毕");
    }
    // 如果脚本是常驻的（比如有循环），不要关闭 L
    // lua_close(L);
}

void loop()
{
    delay(1000);
}