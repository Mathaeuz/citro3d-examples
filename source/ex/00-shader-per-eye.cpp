#include <3ds.h>
#include <citro3d.h>
#include <string.h>
#include "vshader_shbin.h"
#include "vshader-negative_shbin.h"

namespace
{
#define CLEAR_COLOR 0x68B0D8FF

#define CONFIG_3D_SLIDERSTATE (*(volatile float *)0x1FF81080)

#define DISPLAY_TRANSFER_FLAGS                                                                     \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |               \
     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

    typedef struct
    {
        DVLB_s *vshader_dvlb;
        shaderProgram_s program;
        s8 uLoc_projection;
    } TShader;
    C3D_Mtx projection;

    TShader shaderPositive, shaderNegative;

    void loadShader(TShader *shader, u32 *data, u32 size)
    {
        // Load the vertex shader, create a shader program and bind it
        shader->vshader_dvlb = DVLB_ParseFile(data, size);
        shaderProgramInit(&shader->program);
        shaderProgramSetVsh(&shader->program, &shader->vshader_dvlb->DVLE[0]);
        C3D_BindProgram(&shader->program);
        // Get the location of the uniforms

        shader->uLoc_projection = shaderInstanceGetUniformLocation(shader->program.vertexShader, "projection");

        // Configure attributes for use with the vertex shader
        // Attribute format and element count are ignored in immediate mode
        C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
        AttrInfo_Init(attrInfo);
        AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
        AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 3); // v1=color
    }

    void sceneInit(void)
    {
        loadShader(&shaderPositive, (u32 *)vshader_shbin, vshader_shbin_size);
        loadShader(&shaderNegative, (u32 *)vshader_negative_shbin, vshader_negative_shbin_size);

        // Compute the projection matrix
        Mtx_OrthoTilt(&projection, 0.0, 400.0, 0.0, 240.0, 0.0, 1.0, true);

        // Configure the first fragment shading substage to just pass through the vertex color
        // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
        C3D_TexEnv *env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, (GPU_TEVSRC)0, (GPU_TEVSRC)0);
        C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    }

    void sceneRender(TShader *shader)
    {
        // Update the uniforms
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shader->uLoc_projection, &projection);

        // Draw the triangle directly
        C3D_ImmDrawBegin(GPU_TRIANGLES);
        C3D_ImmSendAttrib(200.0f, 200.0f, 0.5f, 0.0f); // v0=position
        C3D_ImmSendAttrib(1.0f, 0.0f, 0.0f, 1.0f);     // v1=color

        C3D_ImmSendAttrib(100.0f, 40.0f, 0.5f, 0.0f);
        C3D_ImmSendAttrib(0.0f, 1.0f, 0.0f, 1.0f);

        C3D_ImmSendAttrib(300.0f, 40.0f, 0.5f, 0.0f);
        C3D_ImmSendAttrib(0.0f, 0.0f, 1.0f, 1.0f);
        C3D_ImmDrawEnd();
    }

    void sceneExit(void)
    {
        // Free the shader program
        shaderProgramFree(&shaderPositive.program);
        DVLB_Free(shaderPositive.vshader_dvlb);

        shaderProgramFree(&shaderNegative.program);
        DVLB_Free(shaderNegative.vshader_dvlb);
    }
}

void exerciseShaderPerEye()
{
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize the render target
    C3D_RenderTarget *leftTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(leftTarget, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTarget *rightTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(rightTarget, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

    // Initialize the scene
    sceneInit();

    // Main loop
    while (aptMainLoop())
    {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
        {
            return; // break exercise
        }

        if (CONFIG_3D_SLIDERSTATE > 0.0f)
        {
            gfxSet3D(true);
        }
        else
        {
            gfxSet3D(false);
        }

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(leftTarget, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(leftTarget);
        C3D_BindProgram(&shaderPositive.program);
        sceneRender(&shaderPositive);
        C3D_RenderTargetClear(rightTarget, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(rightTarget);
        C3D_BindProgram(&shaderNegative.program);
        sceneRender(&shaderPositive);
        C3D_FrameEnd(0);
    }

    // Deinitialize the scene
    sceneExit();

    C3D_RenderTargetDelete(leftTarget);
    C3D_RenderTargetDelete(rightTarget);

    // Deinitialize graphics
    C3D_Fini();
    gfxSet3D(false);
}
