#include <Arduino.h>
extern "C"
{
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"
}

// 专门为 Lua 准备的内存分配器
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
        // 优先从内部 RAM 分配以获得最高性能
        // 如果内部内存不足，heap_caps_realloc 会自动尝试从 PSRAM 分配（取决于标志）
        // 这里我们强制使用 MALLOC_CAP_8BIT 确保兼容性
        void *p = heap_caps_realloc(ptr, nsize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (p == NULL && nsize > 0)
            return NULL;
        return p;
    }
}