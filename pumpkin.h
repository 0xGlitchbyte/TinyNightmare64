// Generated by Arabiki64 V1.3
// By Buu342


// Custom combine mode to allow mixing primitive and vertex colors
#ifndef G_CC_PRIMLITE
    #define G_CC_PRIMLITE SHADE,0,PRIMITIVE,0,0,0,0,PRIMITIVE
#endif


/*********************************
              Models
*********************************/

static Vtx vtx_pumpkin[] = {
    {1, 10, 26, 0, 0, 0, -5, 124, -27, 255}, /* 0 */
    {2, 2, 31, 0, 0, 0, -8, 109, 65, 255}, /* 1 */
    {6, 0, 31, 0, 0, 0, 123, 26, 17, 255}, /* 2 */
    {10, 3, 26, 0, 0, 0, 113, 46, -36, 255}, /* 3 */
    {4, -5, 31, 0, 0, 0, 59, -103, 46, 255}, /* 4 */
    {6, -8, 26, 0, 0, 0, 70, -96, -45, 255}, /* 5 */
    {0, -5, 31, 0, 0, 0, -52, -84, 80, 255}, /* 6 */
    {-5, -8, 26, 0, 0, 0, -79, -93, -36, 255}, /* 7 */
    {-2, 0, 31, 0, 0, 0, -86, 18, 92, 255}, /* 8 */
    {0, 1, 35, 0, 0, 0, -93, 42, 76, 255}, /* 9 */
    {4, 3, 35, 0, 0, 0, -10, 127, 5, 255}, /* 10 */
    {-9, 3, 26, 0, 0, 0, -116, 43, -27, 255}, /* 11 */
    {8, 1, 35, 0, 0, 0, 88, 31, -86, 255}, /* 12 */
    {6, -4, 35, 0, 0, 0, 68, -107, 7, 255}, /* 13 */
    {2, -4, 35, 0, 0, 0, -52, -82, 82, 255}, /* 14 */
    {4, 1, 37, 0, 0, 0, -44, 29, 116, 255}, /* 15 */
    {7, 3, 37, 0, 0, 0, 37, 110, 53, 255}, /* 16 */
    {10, 1, 37, 0, 0, 0, 116, 36, 35, 255}, /* 17 */
    {9, -2, 37, 0, 0, 0, 94, -69, 50, 255}, /* 18 */
    {5, -2, 37, 0, 0, 0, -18, -53, 114, 255}, /* 19 */
    {0, 24, -29, 0, 0, 0, 0, 66, -109, 255}, /* 20 */
    {27, 27, -11, 0, 0, 0, 85, 85, -40, 255}, /* 21 */
    {17, 17, -29, 0, 0, 0, 46, 46, -109, 255}, /* 22 */
    {0, 38, -11, 0, 0, 0, -1, 121, -39, 255}, /* 23 */
    {0, 0, -29, 0, 0, 0, 0, 0, -127, 255}, /* 24 */
    {-17, 17, -29, 0, 0, 0, -47, 47, -108, 255}, /* 25 */
    {38, 0, -11, 0, 0, 0, 121, 0, -40, 255}, /* 26 */
    {24, 0, -29, 0, 0, 0, 66, 0, -109, 255}, /* 27 */
    {-24, 0, -29, 0, 0, 0, -66, 1, -108, 255}, /* 28 */
    {27, -27, -11, 0, 0, 0, 85, -85, -40, 255}, /* 29 */
    {-25, 27, -11, 0, 0, 0, -84, 87, -39, 255}, /* 30 */
    {17, -17, -29, 0, 0, 0, 46, -46, -109, 255}, /* 31 */
    {-17, -17, -29, 0, 0, 0, -46, -46, -109, 255}, /* 32 */
    {0, -24, -29, 0, 0, 0, 0, -66, -109, 255}, /* 33 */
    {0, -38, -11, 0, 0, 0, 0, -121, -40, 255}, /* 34 */
    {-27, -27, -11, 0, 0, 0, -85, -85, -40, 255}, /* 35 */
    {-38, 0, -11, 0, 0, 0, -121, 2, -39, 255}, /* 36 */
    {-27, -27, 11, 0, 0, 0, -85, -85, 40, 255}, /* 37 */
    {-38, 0, 11, 0, 0, 0, -121, 3, 39, 255}, /* 38 */
    {-25, 27, 11, 0, 0, 0, -84, 87, 38, 255}, /* 39 */
    {0, 38, 11, 0, 0, 0, -1, 121, 39, 255}, /* 40 */
    {-17, 17, 29, 0, 0, 0, -44, 43, 111, 255}, /* 41 */
    {27, 27, 11, 0, 0, 0, 85, 85, 40, 255}, /* 42 */
    {0, 24, 29, 0, 0, 0, -1, 60, 112, 255}, /* 43 */
    {17, 17, 29, 0, 0, 0, 42, 42, 112, 255}, /* 44 */
    {0, 0, 26, 0, 0, 0, 0, 0, 127, 255}, /* 45 */
    {-24, 0, 29, 0, 0, 0, -60, 1, 112, 255}, /* 46 */
    {24, 0, 29, 0, 0, 0, 59, 0, 112, 255}, /* 47 */
    {-17, -17, 29, 0, 0, 0, -42, -42, 112, 255}, /* 48 */
    {0, -38, 11, 0, 0, 0, 0, -121, 40, 255}, /* 49 */
    {0, -24, 29, 0, 0, 0, 0, -59, 112, 255}, /* 50 */
    {27, -27, 11, 0, 0, 0, 85, -85, 40, 255}, /* 51 */
    {17, -17, 29, 0, 0, 0, 42, -42, 112, 255}, /* 52 */
    {0, 0, 26, 0, 0, 0, 0, 0, 127, 255}, /* 53 */
    {0, -24, 29, 0, 0, 0, 0, -59, 112, 255}, /* 54 */
    {27, -27, 11, 0, 0, 0, 85, -85, 40, 255}, /* 55 */
    {24, 0, 29, 0, 0, 0, 59, 0, 112, 255}, /* 56 */
    {38, 0, -11, 0, 0, 0, 121, 0, -40, 255}, /* 57 */
    {38, 0, 11, 0, 0, 0, 121, 0, 40, 255}, /* 58 */
    {27, 27, 11, 0, 0, 0, 85, 85, 40, 255}, /* 59 */
    {27, 27, -11, 0, 0, 0, 85, 85, -40, 255}, /* 60 */
};

static Gfx gfx_pumpkin[] = {
    gsSPVertex(vtx_pumpkin+0, 20, 0),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_PRIMLITE, G_CC_PRIMLITE),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsSPClearGeometryMode(0xFFFFFFFF),
    gsSPSetGeometryMode(G_SHADE | G_ZBUFFER | G_CULL_BACK | G_SHADING_SMOOTH | G_LIGHTING),
    gsDPSetPrimColor(0, 0, 36, 55, 0, 255),
    gsDPPipeSync(),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    gsSP2Triangles(3, 2, 4, 0, 3, 4, 5, 0),
    gsSP2Triangles(5, 4, 6, 0, 5, 6, 7, 0),
    gsSP2Triangles(1, 8, 9, 0, 1, 9, 10, 0),
    gsSP2Triangles(7, 6, 8, 0, 7, 8, 11, 0),
    gsSP2Triangles(11, 8, 1, 0, 11, 1, 0, 0),
    gsSP2Triangles(3, 5, 7, 0, 11, 0, 7, 0),
    gsSP2Triangles(0, 3, 7, 0, 4, 2, 12, 0),
    gsSP2Triangles(4, 12, 13, 0, 8, 6, 14, 0),
    gsSP2Triangles(8, 14, 9, 0, 10, 9, 15, 0),
    gsSP2Triangles(10, 15, 16, 0, 13, 12, 17, 0),
    gsSP2Triangles(13, 17, 18, 0, 9, 14, 19, 0),
    gsSP2Triangles(9, 19, 15, 0, 2, 1, 10, 0),
    gsSP2Triangles(2, 10, 12, 0, 6, 4, 13, 0),
    gsSP2Triangles(6, 13, 14, 0, 16, 15, 19, 0),
    gsSP2Triangles(18, 17, 19, 0, 17, 16, 19, 0),
    gsSP2Triangles(14, 13, 18, 0, 14, 18, 19, 0),
    gsSP2Triangles(12, 10, 16, 0, 12, 16, 17, 0),

    gsSPVertex(vtx_pumpkin+20, 32, 0),
    gsDPSetPrimColor(0, 0, 120, 20, 0, 255),
    gsSP2Triangles(0, 1, 2, 0, 0, 3, 1, 0),
    gsSP2Triangles(4, 0, 2, 0, 5, 3, 0, 0),
    gsSP2Triangles(4, 5, 0, 0, 2, 1, 6, 0),
    gsSP2Triangles(4, 2, 7, 0, 2, 6, 7, 0),
    gsSP2Triangles(4, 8, 5, 0, 7, 6, 9, 0),
    gsSP2Triangles(8, 10, 5, 0, 5, 10, 3, 0),
    gsSP2Triangles(4, 7, 11, 0, 7, 9, 11, 0),
    gsSP2Triangles(4, 12, 8, 0, 4, 11, 13, 0),
    gsSP2Triangles(4, 13, 12, 0, 11, 14, 13, 0),
    gsSP2Triangles(11, 9, 14, 0, 13, 15, 12, 0),
    gsSP2Triangles(13, 14, 15, 0, 12, 16, 8, 0),
    gsSP2Triangles(12, 15, 16, 0, 8, 16, 10, 0),
    gsSP2Triangles(14, 17, 15, 0, 15, 18, 16, 0),
    gsSP2Triangles(15, 17, 18, 0, 16, 19, 10, 0),
    gsSP2Triangles(16, 18, 19, 0, 10, 19, 20, 0),
    gsSP2Triangles(10, 20, 3, 0, 18, 21, 19, 0),
    gsSP2Triangles(3, 20, 22, 0, 3, 22, 1, 0),
    gsSP2Triangles(19, 23, 20, 0, 19, 21, 23, 0),
    gsSP2Triangles(20, 23, 24, 0, 20, 24, 22, 0),
    gsSP2Triangles(21, 25, 23, 0, 23, 25, 24, 0),
    gsSP2Triangles(18, 26, 21, 0, 26, 25, 21, 0),
    gsSP2Triangles(17, 26, 18, 0, 22, 24, 27, 0),
    gsSP2Triangles(24, 25, 27, 0, 17, 28, 26, 0),
    gsSP2Triangles(28, 25, 26, 0, 29, 28, 17, 0),
    gsSP2Triangles(14, 29, 17, 0, 9, 29, 14, 0),
    gsSP2Triangles(30, 25, 28, 0, 29, 30, 28, 0),
    gsSP2Triangles(9, 31, 29, 0, 31, 30, 29, 0),
    gsSP1Triangle(6, 31, 9, 0),

    gsSPVertex(vtx_pumpkin+52, 9, 0),
    gsSP2Triangles(0, 1, 2, 0, 3, 0, 2, 0),
    gsSP2Triangles(4, 1, 0, 0, 5, 6, 3, 0),
    gsSP2Triangles(6, 0, 3, 0, 6, 4, 0, 0),
    gsSP2Triangles(7, 4, 6, 0, 8, 6, 5, 0),
    gsSP1Triangle(8, 7, 6, 0),
    gsSPEndDisplayList(),
};

