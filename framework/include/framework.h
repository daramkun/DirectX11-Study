#ifndef __DIRECTX11_STUDY_FRAMEWORK_H__
#define __DIRECTX11_STUDY_FRAMEWORK_H__

#include <Windows.h>
#include <VersionHelpers.h>

#include <atlbase.h>

#include <cstdint>

#if WDK_NTDDI_VERSION >= NTDDI_WIN10_RS3
#	include <dxgi1_6.h>
#endif
#if WDK_NTDDI_VERSION >= NTDDI_WIN10_RS1
#	include <dxgi1_5.h>
#endif
#if WDK_NTDDI_VERSION >= NTDDI_WIN10
#	include <d3d11_4.h>
#	include <d3d12.h>
#	include <dxgi1_4.h>
#endif
#if WDK_NTDDI_VERSION >= NTDDI_WINBLUE
#	include <d3d11_3.h>
#	include <dxgi1_3.h>
#endif
#if WDK_NTDDI_VERSION >= NTDDI_WIN8
#	include <d3d11_2.h>
#	include <dxgi1_2.h>
#endif
#if WDK_NTDDI_VERSION >= NTDDI_WIN7
#	include <d3d11.h>
#	include <dxgi.h>
#	include <d3dcompiler.h>
#	include <Xinput.h>
#	include <xaudio2.h>
#	include <x3daudio.h>

#	include <wincodec.h>

#	include <mfapi.h>
#	include <mfidl.h>
#	include <mftransform.h>
#	include <mfreadwrite.h>

#	include <DirectXMath.h>
#else
#	error "This framework only support Windows 7 or higher."
#endif

float GetTime ();
HRESULT ReadAllData (LPCTSTR filename, void* buffer, UINT maxLength, UINT* readLength);
HRESULT CreateStream (LPCTSTR filename, DWORD grfMode, IStream** stream);

extern HRESULT Initialize (HWND hWnd, UINT width, UINT height);
extern void Destroy ();
extern void Update (float dt);
extern void Render (float dt);

bool IsKeyDown (BYTE vk);
DirectX::XMINT2 GetMousePosition ();
bool IsMouseButtonDown (int button);

HRESULT CreateDevice11 (HWND hWnd, UINT width, UINT height,
	ID3D11Device** device, ID3D11DeviceContext** immediate, IDXGISwapChain** swapChain);

#define RAC_VERTEXSHADER									"vs_5_0"
#define RAC_PIXELSHADER										"ps_5_0"
#define RAC_GEOMETRYSHADER									"gs_5_0"
#define RAC_HULLSHADER										"hs_5_0"
#define RAC_DOMAINSHADER									"ds_5_0"
#define RAC_COMPUTESHADER									"cs_5_0"
HRESULT ReadAndCompile (LPCTSTR filename, LPCSTR profile, LPCSTR main, void* buffer, UINT maxLength, UINT* bufferLength);

HRESULT CreateInputLayoutFromVertexShader (ID3D11Device* d3dDevice, void* vs, UINT vsLength, ID3D11InputLayout** inputLayout);
HRESULT LoadTexture2D (ID3D11Device* d3dDevice, LPCTSTR filename, ID3D11Texture2D** texture);
HRESULT LoadTextureCube (ID3D11Device* d3dDevice, LPCTSTR px, LPCTSTR nx, LPCTSTR py, LPCTSTR ny, LPCTSTR pz, LPCTSTR nz, ID3D11Texture2D** texture);

DirectX::XMFLOAT3 GetNormalVector (const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT3& c);

UINT FrameworkVertexStride ();
HRESULT CreateTriangle (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices);
HRESULT CreateRectangle (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices);
HRESULT CreateBox (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices);
HRESULT CreateSphere (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices);
HRESULT CreateModelFromOBJFile (ID3D11Device* d3dDevice, LPCTSTR filename, ID3D11Buffer** buffer, UINT* vertices);

UINT FrameworkSkyboxVertexStride ();
HRESULT CreateSkyboxBox (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices);

class IDUR
{
public:
	virtual ~IDUR () {}

public:
	virtual HRESULT Initialize (HWND hWnd, UINT width, UINT height) = 0;

public:
	virtual void Update (float dt) = 0;
	virtual void Render (float dt) = 0;
};

#define REGISTER_IDUR(x)									x *g_idur;\
															HRESULT Initialize (HWND hWnd, UINT width, UINT height) { g_idur = new x (); return g_idur->Initialize (hWnd, width, height); }\
															void Destroy () { if(g_idur) delete g_idur; g_idur = nullptr; }\
															void Update (float dt) { g_idur->Update(dt); }\
															void Render (float dt) { g_idur->Render (dt); }

#endif