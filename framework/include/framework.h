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

extern HRESULT Initialize (HWND hWnd, UINT width, UINT height);
extern void Destroy ();
extern void Update (float dt);
extern void Render (float dt);

HRESULT CreateDevice11 (HWND hWnd, UINT width, UINT height,
	ID3D11Device** device, ID3D11DeviceContext** immediate, IDXGISwapChain** swapChain);

#define RAC_VERTEXSHADER									"vs_5_0"
#define RAC_PIXELSHADER										"ps_5_0"
#define RAC_GEOMETRYSHADER									"gs_5_0"
#define RAC_HULLSHADER										"hs_5_0"
#define RAC_DOMAINSHADER									"ds_5_0"
#define RAC_COMPUTESHADER									"cs_5_0"
HRESULT ReadAndCompile (LPCTSTR filename, LPCSTR profile, LPCSTR main, void* buffer, UINT maxLength);

HRESULT CreateInputLayoutFromVertexShader (ID3D11Device* d3dDevice, void* vs, UINT vsLength, ID3D11InputLayout** inputLayout);

#endif