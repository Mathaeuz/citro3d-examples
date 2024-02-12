#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>
#include <string.h>
#include <math.h>
#include "textured_shbin.h"
#include "ex/02.h"
#include "box_t3x.h"

namespace
{
#define CLEAR_COLOR 0x000000FF

#define DISPLAY_TRANSFER_FLAGS                                                                     \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |               \
     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

    const float vertexes[144] = {
        +0.5f, +0.5f, +0.5f, 0, 1, 1,
        +0.5f, +0.5f, -0.5f, 1, 1, 1,
        +0.5f, -0.5f, +0.5f, 0, 0, 1,
        +0.5f, -0.5f, -0.5f, 1, 0, 1,
        -0.5f, +0.5f, +0.5f, 1, 1, 1,
        -0.5f, +0.5f, -0.5f, 0, 1, 1,
        -0.5f, -0.5f, +0.5f, 1, 0, 1,
        -0.5f, -0.5f, -0.5f, 0, 0, 1, // 7
        +0.5f, +0.5f, +0.5f, 0, 0, 0,
        +0.5f, +0.5f, -0.5f, 0, 1, 0,
        -0.5f, +0.5f, +0.5f, 1, 0, 0,
        -0.5f, +0.5f, -0.5f, 1, 1, 0,
        +0.5f, -0.5f, +0.5f, 0, 0, 0,
        +0.5f, -0.5f, -0.5f, 0, 1, 0,
        -0.5f, -0.5f, +0.5f, 1, 0, 0,
        -0.5f, -0.5f, -0.5f, 1, 1, 0, // 15
        +0.5f, +0.5f, +0.5f, 1, 1, 1,
        -0.5f, +0.5f, +0.5f, 0, 1, 1,
        +0.5f, -0.5f, +0.5f, 1, 0, 1,
        -0.5f, -0.5f, +0.5f, 0, 0, 1,
        +0.5f, +0.5f, -0.5f, 0, 1, 1,
        -0.5f, +0.5f, -0.5f, 1, 1, 1,
        +0.5f, -0.5f, -0.5f, 0, 0, 1,
        -0.5f, -0.5f, -0.5f, 1, 0, 1, // 23
    };
#define vertex_count (sizeof(vertexes) / sizeof(vertexes[0]))

    const char indexes[36] = {
        1, 0, 2,    //+X1
        1, 2, 3,    //+X2
        4, 5, 6,    //-X1
        6, 5, 7,    //-X2
        8, 9, 10,   //+Y1
        10, 9, 11,  //+Y2
        13, 12, 14, //-Y1
        13, 14, 15, //-Y2
        16, 17, 18, //+Z1
        18, 17, 19, //+Z2
        21, 20, 22, //-Z1
        21, 22, 23, //-Z2
    };
#define index_count (sizeof(indexes) / sizeof(indexes[0]))

    typedef struct
    {
        DVLB_s *vshader_dvlb;
        shaderProgram_s program;
        s8 uLoc_projection, uLoc_modelView, uLoc_wrap;
    } TShader;
    C3D_Mtx projection;
    C3D_Tex texture;

    TShader shader;
    static void *vbo_data;
    static void *ibo_data;
    static float zDist, yRot, xRot;
    static float uWrap, vWrap;

    void loadShader(TShader *shader, u32 *data, u32 size)
    {
        // Load the vertex shader, create a shader program and bind it
        shader->vshader_dvlb = DVLB_ParseFile(data, size);
        shaderProgramInit(&shader->program);
        shaderProgramSetVsh(&shader->program, &shader->vshader_dvlb->DVLE[0]);
        C3D_BindProgram(&shader->program);
        // Get the location of the uniforms

        shader->uLoc_projection = shaderInstanceGetUniformLocation(shader->program.vertexShader, "projection");
        shader->uLoc_modelView = shaderInstanceGetUniformLocation(shader->program.vertexShader, "modelView");
        shader->uLoc_wrap = shaderInstanceGetUniformLocation(shader->program.vertexShader, "wrap");
    }

    void sceneInit(void)
    {
        zDist = -3.0f;
        xRot = 30.0f;
        yRot = 45.0f;
        uWrap = 0.5f;
        vWrap = 0.0f;
        loadShader(&shader, (u32 *)textured_shbin, textured_shbin_size);

        // Configure attributes for use with the vertex shader
        // Attribute format and element count are ignored in immediate mode
        C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
        AttrInfo_Init(attrInfo);
        AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
        AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 3); // v1=uv (xy or st) and wrap rate(z or p)

        // Configure texture
        Tex3DS_TextureImport(box_t3x, box_t3x_size, &texture, NULL, false);

        C3D_TexSetFilter(&texture, GPU_NEAREST, GPU_NEAREST);
        C3D_TexSetWrap(&texture, GPU_REPEAT, GPU_REPEAT);
        C3D_TexBind(0, &texture);

        // Create the VBO (vertex buffer object)
        vbo_data = linearAlloc(sizeof(vertexes));
        memcpy(vbo_data, vertexes, sizeof(vertexes));
        ibo_data = linearAlloc(sizeof(indexes));
        memcpy(ibo_data, indexes, sizeof(indexes));

        // Configure buffers
        C3D_BufInfo *bufInfo = C3D_GetBufInfo();
        BufInfo_Init(bufInfo);
        BufInfo_Add(bufInfo, vbo_data, sizeof(float[6]), 2, 0x10);

        // Configure the first fragment shading substage to just pass through the vertex color
        // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
        C3D_TexEnv *env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, (GPU_TEVSRC)0, (GPU_TEVSRC)0);
        C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

        C3D_CullFace(GPU_CULL_BACK_CCW);
        C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);
    }

    void sceneRender(TShader *shader, float iod)
    {
        // Compute the projection matrix
        Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(60.0f), C3D_AspectRatioTop, 0.01f, 100.0f, iod, 2.0f, false);

        // Calculate the modelView matrix
        C3D_Mtx modelView;
        Mtx_Identity(&modelView);
        Mtx_Translate(&modelView, 0, 0, zDist, true);
        Mtx_RotateX(&modelView, C3D_AngleFromDegrees(xRot), true);
        Mtx_RotateY(&modelView, C3D_AngleFromDegrees(yRot), true);

        // Update the uniforms
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shader->uLoc_projection, &projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shader->uLoc_modelView, &modelView);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, shader->uLoc_wrap, uWrap, vWrap, 0, 0);
        C3D_DrawElements(GPU_TRIANGLES, index_count, C3D_UNSIGNED_BYTE, ibo_data);
    }

    void sceneExit(void)
    {
        C3D_TexDelete(&texture);

        // Free the shader program
        shaderProgramFree(&shader.program);
        DVLB_Free(shader.vshader_dvlb);
    }
}

void run02()
{
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize the render target
    C3D_RenderTarget *leftTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(leftTarget, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTarget *rightTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(rightTarget, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

    // Initialize the scene
    sceneInit();
    C3D_BindProgram(&shader.program);
    // Main loop
    while (aptMainLoop())
    {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        auto iod = osGet3DSliderState() / 3;
        gfxSet3D(iod > 0);

        if (kDown & KEY_START)
        {
            return; // break exercise
        }
        if (kHeld & KEY_LEFT)
        {
            yRot -= 2;
        }
        if (kHeld & KEY_RIGHT)
        {
            yRot += 2;
        }
        if (kHeld & KEY_Y)
        {
            xRot -= 2;
        }
        if (kHeld & KEY_X)
        {
            xRot += 2;
        }
        if (kHeld & KEY_DOWN)
        {
            zDist -= .1;
        }
        if (kHeld & KEY_UP)
        {
            zDist += .1;
        }
        uWrap = fmodf(uWrap + 0.025f, 1.0f);

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(leftTarget, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(leftTarget);
        sceneRender(&shader, -iod);
        C3D_RenderTargetClear(rightTarget, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(rightTarget);
        sceneRender(&shader, iod);
        C3D_FrameEnd(0);
    }

    gfxSet3D(false);
    // Deinitialize the scene
    sceneExit();

    C3D_RenderTargetDelete(leftTarget);
    C3D_RenderTargetDelete(rightTarget);

    // Deinitialize graphics
    C3D_Fini();
}