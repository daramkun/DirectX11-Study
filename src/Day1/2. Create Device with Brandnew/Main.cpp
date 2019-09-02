#include <framework.h>

#define SAFE_RELEASE(x)										if ((x)) (x)->Release ()

IDXGISwapChain1* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

HRESULT Initialize (HWND hWnd, UINT width, UINT height)
{
	if (FAILED (D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION,
		&d3dDevice, nullptr, &immediateContext)))
		return E_FAIL;

	IDXGIFactory1* dxgiFactory;
	if (FAILED (CreateDXGIFactory1 (__uuidof(IDXGIFactory1), (void**)& dxgiFactory)))
		return E_FAIL;

	IDXGIFactory2* dxgiFactory2;
	if (FAILED (dxgiFactory->QueryInterface (__uuidof(IDXGIFactory2), (void**)& dxgiFactory2)))
		return E_FAIL;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SampleDesc.Count = 1;
	if (IsWindows10OrGreater ())
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.BufferCount = 2;
	}
	else
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.BufferCount = 1;
	}

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
	fullscreenDesc.Windowed = true;

	if (FAILED (dxgiFactory2->CreateSwapChainForHwnd (d3dDevice, hWnd, &swapChainDesc, &fullscreenDesc, nullptr, &dxgiSwapChain)))
	{
		dxgiFactory2->Release ();
		dxgiFactory->Release ();
		return E_FAIL;
	}

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (immediateContext);
	SAFE_RELEASE (d3dDevice);
	SAFE_RELEASE (dxgiSwapChain);
}

void Update (float dt)
{

}

void Render (float dt)
{
	dxgiSwapChain->Present (0, 0);
}