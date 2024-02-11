#include <3ds.h>
#include <citro3d.h>
#include <string.h>
#include "normal_shbin.h"
#include "ex/01.h"

namespace
{
#define CLEAR_COLOR 0x000000FF

#define DISPLAY_TRANSFER_FLAGS                                                                     \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |               \
     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

    const float vertexes[32] = {
        +0.5f, +0.5f, +0.5f, // 0
        +0.5f, +0.5f, -0.5f, // 1
        +0.5f, -0.5f, +0.5f, // 2
        +0.5f, -0.5f, -0.5f, // 3
        -0.5f, +0.5f, +0.5f, // 4
        -0.5f, +0.5f, -0.5f, // 5
        -0.5f, -0.5f, +0.5f, // 6
        -0.5f, -0.5f, -0.5f, // 7
    };
#define vertex_count (sizeof(vertexes) / sizeof(vertexes[0]))

    const char indexes[36] = {
        0, 1, 2, //+X1
        2, 1, 3, //+X2
        5, 4, 6, //-X1
        5, 6, 7, //-X2
        1, 0, 4, //+Y1
        1, 4, 5, //+Y2
        2, 3, 6, //-Y1
        6, 3, 7, //-Y2
        0, 2, 4, //+Z1
        4, 2, 6, //+Z2
        3, 1, 5, //-Z1
        3, 5, 7, //-Z2
    };
#define index_count (sizeof(indexes) / sizeof(indexes[0]))

    typedef struct
    {
        DVLB_s *vshader_dvlb;
        shaderProgram_s program;
        s8 uLoc_projection, uLoc_modelView;
    } TShader;
    C3D_Mtx projection;

    TShader shader;
    static void *vbo_data;
    static void *ibo_data;
    static float zDist, xRot, yRot;

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
    }

    void sceneInit(void)
    {
        zDist = 2.0f;
        yRot = 15.0f;
        xRot = 0.0f;
        loadShader(&shader, (u32 *)normal_shbin, normal_shbin_size);

        // Configure attributes for use with the vertex shader
        // Attribute format and element count are ignored in immediate mode
        C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
        AttrInfo_Init(attrInfo);
        AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position

        // Create the VBO (vertex buffer object)
        vbo_data = linearAlloc(sizeof(vertexes));
        memcpy(vbo_data, vertexes, sizeof(vertexes));
        ibo_data = linearAlloc(sizeof(indexes));
        memcpy(ibo_data, indexes, sizeof(indexes));

        // Configure buffers
        C3D_BufInfo *bufInfo = C3D_GetBufInfo();
        BufInfo_Init(bufInfo);
        BufInfo_Add(bufInfo, vbo_data, sizeof(float[3]), 1, 0x0);

        // Compute the projection matrix
        Mtx_Persp(&projection, C3D_AngleFromDegrees(60.0f), 1 / C3D_AspectRatioTop, 0.01f, 100.0f, true);

        // Configure the first fragment shading substage to just pass through the vertex color
        // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
        C3D_TexEnv *env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, (GPU_TEVSRC)0, (GPU_TEVSRC)0);
        C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    }

    void sceneRender(TShader *shader)
    {
        // Calculate the modelView matrix
        C3D_Mtx modelView;
        Mtx_Identity(&modelView);
        Mtx_Translate(&modelView, 0, 0, zDist, true);
        Mtx_RotateY(&modelView, C3D_AngleFromDegrees(yRot), true);
        Mtx_RotateX(&modelView, C3D_AngleFromDegrees(xRot), true);

        // Update the uniforms
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shader->uLoc_projection, &projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shader->uLoc_modelView, &modelView);

        C3D_CullFace(GPU_CULL_BACK_CCW);
        C3D_DrawElements(GPU_TRIANGLES, index_count, C3D_UNSIGNED_BYTE, ibo_data);
    }

    void sceneExit(void)
    {
        // Free the shader program
        shaderProgramFree(&shader.program);
        DVLB_Free(shader.vshader_dvlb);
    }
}

void run01()
{
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize the render target
    C3D_RenderTarget *leftTarget = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(leftTarget, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

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
        if (kDown & KEY_START)
        {
            return; // break exercise
        }
        if (kHeld & KEY_LEFT)
        {
            xRot -= 2;
        }
        if (kHeld & KEY_RIGHT)
        {
            xRot += 2;
        }
        if (kHeld & KEY_Y)
        {
            yRot -= 2;
        }
        if (kHeld & KEY_X)
        {
            yRot += 2;
        }
        if (kHeld & KEY_DOWN)
        {
            zDist -= .1;
        }
        if (kHeld & KEY_UP)
        {
            zDist += .1;
        }

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(leftTarget, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(leftTarget);
        sceneRender(&shader);
        C3D_FrameEnd(0);
    }

    // Deinitialize the scene
    sceneExit();

    C3D_RenderTargetDelete(leftTarget);

    // Deinitialize graphics
    C3D_Fini();
}