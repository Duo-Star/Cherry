#include "hw/bee.h"
#include "hw/spk.h"
#include "hw/sd.h"
extern "C"
{
#include "lua/src/lua.h"
#include "lua/src/lauxlib.h"
}

// 包装 bee_test
static int l_hw_bee_test(lua_State *L)
{
    bee_test();
    return 0; // 无返回值
}

// 包装 play_beep(freq, ms)
static int l_hw_beep(lua_State *L)
{
    int freq = luaL_checkinteger(L, 1);
    int ms = luaL_checkinteger(L, 2);
    play_beep(freq, ms);
    return 0;
}

// 注册函数列表
static const struct luaL_Reg hwlib[] = {
    {"beep_test", l_hw_bee_test},
    {"beep", l_hw_beep},
    {NULL, NULL}};

// 库初始化函数
int luaopen_hw(lua_State *L)
{
    luaL_newlib(L, hwlib);
    return 1;
}