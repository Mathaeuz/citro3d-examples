#pragma once
#include <stdio.h>
#include <3ds.h>

inline void debugPause(const char *message)
{
    return;
    printf("%s\n",message);
    while (aptMainLoop())
    {
		hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_A)
        {
            return;
        }
    }
}