// Generated by Arabiki64 V1.3
// By Buu342


// Custom combine mode to allow mixing primitive and vertex colors
#ifndef G_CC_PRIMLITE
    #define G_CC_PRIMLITE SHADE,0,PRIMITIVE,0,0,0,0,PRIMITIVE
#endif


Vtx vtx_ground[] = {
  {-500, -500, -2, 0, 0, 0, -73, -73, -73, 255},
  { 500, -500, -2, 0, 0, 0,  73, -73, -73, 255},
  { 500,  500, -2, 0, 0, 0,  73,  73, -73, 255},
  {-500,  500, -2, 0, 0, 0, -73,  73, -73, 255},
  {-500, -500,  2, 0, 0, 0, -73, -73,  73, 255},
  { 500, -500,  2, 0, 0, 0,  73, -73,  73, 255},
  { 500,  500,  2, 0, 0, 0,  73,  73,  73, 255},
  {-500,  500,  2, 0, 0, 0, -73,  73,  73, 255},
  {-500, -500, -2, 0, 0, 0, -73, -73, -73, 255},
};

Gfx gfx_ground[] = {
    gsDPSetPrimColor(0, 0, 4, 75, 28, 255),
    gsDPPipeSync(),
    gsSPVertex(vtx_ground, 8, 0),
    gsSP2Triangles(4, 6, 7, 0, 4, 5, 6, 0),
    gsSP2Triangles(6, 5, 2, 1, 2, 5, 1, 1),
    gsSP2Triangles(1, 3, 2, 0, 0, 3, 1, 2),
    gsSP2Triangles(3, 0, 7, 2, 0, 4, 7, 2),
    gsSP2Triangles(2, 7, 6, 0, 2, 3, 7, 0),
    gsSP2Triangles(0, 5, 4, 0, 0, 1, 5, 0),
    gsDPSetPrimColor(0, 0, 0, 255, 0, 255),
    gsDPPipeSync(),
    gsSPEndDisplayList(),
};
