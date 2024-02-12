/*#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_sw.h>
#include "sw-gui.h"

#include "debugPause.hpp"

namespace
{
    u16 guiWidth, guiHeight;
    C2D_Image image;
    imgui_sw::SwOptions sw_options;
    std::vector<uint32_t> pixel_buffer;
}

void guiInit(ImGuiIO *io, u16 width, u16 height)
{
    guiWidth = width;
    guiHeight = height;
    pixel_buffer = std::vector<uint32_t>(guiWidth * guiHeight, 0);
    sw_options = {};

    C3D_Tex *tex = (C3D_Tex *)malloc(sizeof(C3D_Tex));
    static const Tex3DS_SubTexture subt3x = {512, 256, 0.0f, 1.0f, 1.0f, 0.0f};

    image = (C2D_Image){tex, &subt3x};
    C3D_TexInit(image.tex, 512, 256, GPU_RGBA8);
    C3D_TexSetFilter(image.tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(image.tex, GPU_REPEAT, GPU_REPEAT);

    io->DisplaySize = ImVec2((float)width, (float)height);
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io->MouseDrawCursor = false;

    imgui_sw::bind_imgui_painting();
    imgui_sw::make_style_fast();
}

void guiIO(ImGuiIO *io, u32 inputButtons, touchPosition inputTouch)
{
    memset(io->NavInputs, 0, sizeof(io->NavInputs));
#define MAP_BUTTON(NAV, BUTTON)        \
    {                                  \
        if (inputButtons & BUTTON)        \
            io->NavInputs[NAV] = 1.0f; \
    }
    MAP_BUTTON(ImGuiNavInput_Activate, KEY_A);
    MAP_BUTTON(ImGuiNavInput_Cancel, KEY_B);
    MAP_BUTTON(ImGuiNavInput_Menu, KEY_Y);
    MAP_BUTTON(ImGuiNavInput_Input, KEY_X);
    MAP_BUTTON(ImGuiNavInput_DpadLeft, KEY_DLEFT);
    MAP_BUTTON(ImGuiNavInput_DpadRight, KEY_DRIGHT);
    MAP_BUTTON(ImGuiNavInput_DpadUp, KEY_DUP);
    MAP_BUTTON(ImGuiNavInput_DpadDown, KEY_DDOWN);
    MAP_BUTTON(ImGuiNavInput_FocusPrev, KEY_L);
    MAP_BUTTON(ImGuiNavInput_FocusNext, KEY_R);
// MAP_BUTTON(ImGuiNavInput_TweakSlow,   KEY_L);
// MAP_BUTTON(ImGuiNavInput_TweakFast,   KEY_R);
#undef MAP_BUTTON
    io->BackendFlags |= ImGuiBackendFlags_HasGamepad;
    if (inputTouch.px && inputTouch.py)
    {
        io->MouseDown[0] = true;
        io->MousePos = ImVec2(inputTouch.px, inputTouch.py);
    }
    else
        io->MouseDown[0] = false;
}

void guiDrawToC2D()
{
    debugPause("guiDrawToC2D");
    // fill gray (this could be any previous rendering)
    std::fill_n(pixel_buffer.data(), guiWidth * guiHeight, 0xFFFFFFFF);
    debugPause("post fill_n");

    // overlay the GUI
    paint_imgui(pixel_buffer.data(), guiWidth, guiHeight, sw_options);
    debugPause("post paint_imgui");

    for (u32 x = 0; x < guiWidth; x++)
    {
        for (u32 y = 0; y < guiHeight; y++)
        {
            u32 dstPos = ((((y >> 3) * (512 >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3))) * 4;
            u32 srcPos = (y * guiWidth + x) * 4;
            memcpy(&((u8 *)image.tex->data)[dstPos], &((u8 *)pixel_buffer.data())[srcPos], 4);
        }
    }
    debugPause("post memcpy forfor");

    C2D_DrawImageAt(image, 0.0f, 0.0f, 1.0f, NULL, 1.0f, 1.0f);
    debugPause("post C2D_DrawImageAt");
}

void guiExit()
{
    free(image.tex);
    imgui_sw::unbind_imgui_painting();
}*/