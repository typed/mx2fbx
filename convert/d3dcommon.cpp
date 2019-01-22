#include "d3dcommon.h"

HWND g_hWnd = NULL;
LPDIRECT3D9 g_pD3d = NULL;
LPDIRECT3DDEVICE9 g_pDevice = NULL;

void InitD3D()
{
	//g_pD3d = Direct3DCreate9(D3D_SDK_VERSION);
	//HRESULT ret = g_pD3d->CreateDevice(
}

void ReleaseD3D()
{
	REL(g_pDevice);
	REL(g_pD3d);
	g_hWnd = NULL;
}

LPDIRECT3DDEVICE9 GetDevice()
{
	return g_pDevice;
}