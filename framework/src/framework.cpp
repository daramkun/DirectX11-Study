#include <framework.h>

#include <cassert>
#include <shlwapi.h>

#include <string>
#include <sstream>

#include <vector>

#ifdef UNICODE
using String = std::wstring;
using StringStream = std::wstringstream;
#else
using String = std::string;
using StringStream = std::stringstream;
#endif

float GetTime ()
{
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency (&freq);
	QueryPerformanceCounter (&counter);
	return counter.QuadPart / (float)freq.QuadPart;
}

HRESULT ReadAllData (LPCTSTR filename, void* buffer, UINT maxLength, UINT* readLength)
{
	TCHAR readFile[512];
	if (PathFileExists (filename))
	{
		memcpy (readFile, filename, _tcslen (filename) * sizeof (TCHAR));
	}
	else
	{
		StringStream ss;
		ss << TEXT ("..\\res\\");
		ss << filename;
		GetFullPathName (ss.str ().c_str (), 512, readFile, nullptr);
		if (!PathFileExists (readFile))
		{
			ss = StringStream ();
			ss << TEXT ("..\\..\\..\\res\\");
			ss << filename;
			GetFullPathName (ss.str ().c_str (), 512, readFile, nullptr);
			if (!PathFileExists (readFile))
				return E_FAIL;
		}
	}

	HANDLE hFile = CreateFile (readFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	DWORD length = GetFileSize (hFile, nullptr);

	DWORD read;
	if (!ReadFile (hFile, buffer, min (maxLength, length), &read, nullptr))
		return E_FAIL;

	*readLength = read;

	return S_OK;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	WNDCLASS wndClass =
	{
		NULL, WndProc, 0, 0, GetModuleHandle (nullptr),
		LoadIcon (NULL, IDI_APPLICATION), LoadCursor (NULL, IDC_ARROW),
		NULL, nullptr, TEXT ("DirectX11Study")
	};
	assert (RegisterClass (&wndClass) != INVALID_ATOM);

	int clientWidth = 800, clientHeight = (int)(800 * (GetSystemMetrics (SM_CYSCREEN) / (float)GetSystemMetrics (SM_CXSCREEN)));
	RECT clientRect = { 0, 0, clientWidth, clientHeight };
	AdjustWindowRect (&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

	int width = clientRect.right - clientRect.left,
		height = clientRect.bottom - clientRect.top,
		x = GetSystemMetrics (SM_CXSCREEN) / 2 - width / 2,
		y = GetSystemMetrics (SM_CYSCREEN) / 2 - height / 2;

	HWND hWnd = CreateWindow (TEXT("DirectX11Study"), TEXT("DirectX 11 Study Window"),
		WS_OVERLAPPEDWINDOW, x, y, width, height,
		nullptr, nullptr, GetModuleHandle (nullptr), nullptr);

	assert (hWnd != nullptr);

	assert (SUCCEEDED (Initialize (hWnd, clientWidth, clientHeight)));

	ShowWindow (hWnd, SW_SHOW);

	MSG msg; bool running = true;
	float lastTime = GetTime (), currentTime;
	do
	{
		while (PeekMessage (&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				running = false;
				break;
			}

			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		currentTime = GetTime ();
		float deltaTime = currentTime - lastTime;
		Update (deltaTime);
		Render (deltaTime);
		lastTime = currentTime;

		Sleep (0);

	} while (running);

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage (0);
		break;

	default:
		return DefWindowProc (hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

HRESULT CreateDevice11 (HWND hWnd, UINT width, UINT height,
	ID3D11Device** device, ID3D11DeviceContext** immediate, IDXGISwapChain** swapChain)
{
	if (hWnd == NULL || width == 0 || height == 0
		|| device == nullptr || immediate == nullptr || swapChain == nullptr)
		return E_INVALIDARG;

	if (IsWindows8OrGreater ())
	{
		CComPtr<ID3D11Device> d3dDevice;
		CComPtr<ID3D11DeviceContext> immediateContext;
		if (FAILED (D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION,
			&d3dDevice, nullptr, &immediateContext)))
			return E_FAIL;

		CComPtr<IDXGIFactory1> dxgiFactory;
		if (FAILED (CreateDXGIFactory1 (__uuidof(IDXGIFactory1), (void**)& dxgiFactory)))
			return E_FAIL;

		CComPtr<IDXGIFactory2> dxgiFactory2;
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

		CComPtr<IDXGISwapChain1> swapChain1;
		if (FAILED (dxgiFactory2->CreateSwapChainForHwnd (d3dDevice, hWnd, &swapChainDesc, &fullscreenDesc, nullptr, &swapChain1)))
			return E_FAIL;

		if (FAILED (d3dDevice.QueryInterface<ID3D11Device> (device))) return E_FAIL;
		if (FAILED (immediateContext.QueryInterface<ID3D11DeviceContext> (immediate))) return E_FAIL;
		if (FAILED (swapChain1.QueryInterface<IDXGISwapChain> (swapChain))) return E_FAIL;
	}
	else
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.Windowed = true;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.SampleDesc.Count = 1;

		if (FAILED (D3D11CreateDeviceAndSwapChain (nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
			swapChain, device, nullptr, immediate)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT ReadAndCompile (LPCTSTR filename, LPCSTR profile, LPCSTR main, void* buffer, UINT maxLength)
{
	uint8_t innerBuffer[8192];
	UINT readLength;
	if (FAILED (ReadAllData (filename, innerBuffer, 8192, &readLength)))
		return E_FAIL;

	CComPtr<ID3DBlob> blob, errMsg;
	if (FAILED (D3DCompile (innerBuffer, readLength, nullptr, nullptr, nullptr, main, profile, 0, 0, &blob, &errMsg)))
	{
		OutputDebugStringA ((LPCSTR)(errMsg->GetBufferPointer ()));
		return E_FAIL;
	}

	memcpy (buffer, blob->GetBufferPointer (), min (maxLength, blob->GetBufferSize ()));

	return S_OK;
}

HRESULT CreateInputLayoutFromVertexShader (ID3D11Device* d3dDevice, void* vs, UINT vsLength, ID3D11InputLayout** inputLayout)
{
	CComPtr<ID3D11ShaderReflection> reflector;
	if (FAILED (D3DReflect (vs, vsLength, __uuidof(ID3D11ShaderReflection), (void**)& reflector)))
		return E_FAIL;

	D3D11_SHADER_DESC shaderDesc;
	if (FAILED (reflector->GetDesc (&shaderDesc)))
		return E_FAIL;

	UINT byteOffset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D11_SIGNATURE_PARAMETER_DESC desc;
		if (FAILED (reflector->GetInputParameterDesc (i, &desc)))
			return E_FAIL;

		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		reflector->GetInputParameterDesc (i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = byteOffset;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			byteOffset += 4;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			byteOffset += 8;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			byteOffset += 12;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			byteOffset += 16;
		}

		inputLayoutDesc.push_back (elementDesc);
	}

	if (FAILED (d3dDevice->CreateInputLayout (inputLayoutDesc.data (), inputLayoutDesc.size (), vs, vsLength, inputLayout)))
		return E_FAIL;

	return S_OK;
}