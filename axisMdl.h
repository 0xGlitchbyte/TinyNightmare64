/***************************************************************
                            axisMdl.h
                               
An axis model which was useful for getting calculations right.
Red represents X, Green represents Y, Blue represents Z.
***************************************************************/

Vtx vtx_axis[] = {
  {-2, -2, -2, 0, 0, 0, -73, -73, -73, 255},
  { 50, -2, -2, 0, 0, 0,  73, -73, -73, 255},
  { 50,  2, -2, 0, 0, 0,  73,  73, -73, 255},
  {-2,  2, -2, 0, 0, 0, -73,  73, -73, 255},
  {-2, -2,  2, 0, 0, 0, -73, -73,  73, 255},
  { 50, -2,  2, 0, 0, 0,  73, -73,  73, 255},
  { 50,  2,  2, 0, 0, 0,  73,  73,  73, 255},
  {-2,  2,  2, 0, 0, 0, -73,  73,  73, 255},
  {-2, -2, -2, 0, 0, 0, -73, -73, -73, 255},
};

Gfx gfx_axis[] = {
    gsDPSetPrimColor(0, 0, 0, 255, 0, 255),
    gsDPPipeSync(),
    gsSPVertex(vtx_axis+8, 8, 0),
    gsSP2Triangles(4, 6, 7, 0, 4, 5, 6, 0),
    gsSP2Triangles(6, 5, 2, 1, 2, 5, 1, 1),
    gsSP2Triangles(1, 3, 2, 0, 0, 3, 1, 2),
    gsSP2Triangles(3, 0, 7, 2, 0, 4, 7, 2),
    gsSP2Triangles(2, 7, 6, 0, 2, 3, 7, 0),
    gsSP2Triangles(0, 5, 4, 0, 0, 1, 5, 0),
    gsSPEndDisplayList(),
};

