#pragma once
#include "d3d9/d3dx9.h"

#define REL(p) if(p){p->Release();p=NULL;}

void InitD3D();
void ReleaseD3D();

LPDIRECT3DDEVICE9 GetDevice();