#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include "ex/03.h"
#include "imgui.h"
#include <imgui_impl_citro3d.h>
#include <imgui_impl_ctr.h>

namespace
{
    float colors[3];
    void renderGUI()
    {
        ImGui::NewFrame();
        ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoResize);
        ImGui::ColorPicker3("A:", colors, ImGuiColorEditFlags_DisplayHex);
        ImGui::End();
        ImGui::Render();
    }

    const auto CLEAR_COLOR = 0x00000000;
    const auto SCREEN_WIDTH = 400.0f;
    const auto SCREEN_HEIGHT = 480.0f;
    const auto TRANSFER_SCALING = GX_TRANSFER_SCALE_NO;
    const auto FB_SCALE = 1.0f;
    const auto FB_WIDTH = SCREEN_WIDTH * FB_SCALE;
    const auto FB_HEIGHT = SCREEN_HEIGHT * FB_SCALE;

    const auto DISPLAY_TRANSFER_FLAGS =
        GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) |
        GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
        GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
        GX_TRANSFER_SCALING(TRANSFER_SCALING);
}

void run03()
{
    C3D_Init(2 * C3D_DEFAULT_CMDBUF_SIZE);

    // create top screen render target
    auto topScreen = C3D_RenderTargetCreate(FB_HEIGHT * 0.5f, FB_WIDTH, GPU_RB_RGBA8,
                                      GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(topScreen, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    // create bottom screen render target
    auto bottomScreen = C3D_RenderTargetCreate(FB_HEIGHT * 0.5f, FB_WIDTH * 0.8f,
                                         GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(bottomScreen, GFX_BOTTOM, GFX_LEFT,
                              DISPLAY_TRANSFER_FLAGS);

    ImGui::CreateContext();
    ImGui::StyleColorsClassic();
    auto &io = ImGui::GetIO();
    auto &style = ImGui::GetStyle();
    style.ScaleAllSizes(0.5f);
    io.IniFilename = nullptr;

    ImGui_ImplCtr_Init();
    ImGui_ImplCitro3D_Init();

    while (aptMainLoop())
    {
        hidScanInput();
        if (hidKeysDown() & KEY_START)
        {
            break; // break exercise
        }

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_FrameDrawOn(topScreen);
        C3D_RenderTargetClear(topScreen, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(bottomScreen);
        C3D_RenderTargetClear(bottomScreen, C3D_CLEAR_ALL, CLEAR_COLOR, 0);

        // setup display metrics
        io.DisplaySize = ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT);
        io.DisplayFramebufferScale = ImVec2(FB_SCALE, FB_SCALE);

        ImGui_ImplCitro3D_NewFrame();
        ImGui_ImplCtr_NewFrame();
        renderGUI();
        ImGui_ImplCitro3D_RenderDrawData(ImGui::GetDrawData(), (void*)(topScreen),
                                         (void*)(bottomScreen));
        C3D_FrameEnd(0);
    }
    ImGui_ImplCitro3D_Shutdown();
    ImGui_ImplCtr_Shutdown();
    ImGui::DestroyContext();
    C3D_RenderTargetDelete(topScreen);
    C3D_RenderTargetDelete(bottomScreen);
    C3D_Fini();
}