#include <framework.h>

#include <cassert>
#include <shlwapi.h>
#include <atlconv.h>

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
	if (FAILED (CoInitialize (0)))
		return -1;

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

	Destroy ();

	CoUninitialize ();

	return (int)msg.wParam;
}

bool g_keyDowned[256] = {};
DirectX::XMINT2 g_mousePosition;
bool g_mouseButtonDowned[3] = {};

bool IsKeyDown (BYTE vk) { return g_keyDowned[vk]; }
DirectX::XMINT2 GetMousePosition () { return g_mousePosition; }
bool IsMouseButtonDown (int button) { return g_mouseButtonDowned[button]; }

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN: g_keyDowned[wParam] = true; break;
	case WM_KEYUP: g_keyDowned[wParam] = false; break;

	case WM_MOUSEMOVE:
		g_mousePosition = DirectX::XMINT2 (LOWORD (lParam), HIWORD (lParam));
		break;

	case WM_LBUTTONDOWN: g_mouseButtonDowned[0] = true; break;
	case WM_LBUTTONUP: g_mouseButtonDowned[0] = false; break;
	case WM_RBUTTONDOWN: g_mouseButtonDowned[1] = true; break;
	case WM_RBUTTONUP: g_mouseButtonDowned[1] = false; break;
	case WM_MBUTTONDOWN: g_mouseButtonDowned[2] = true; break;
	case WM_MBUTTONUP: g_mouseButtonDowned[2] = false; break;

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

	DWORD createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG) || !defined (NDEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (IsWindows8OrGreater ())
	{
		CComPtr<ID3D11Device> d3dDevice;
		CComPtr<ID3D11DeviceContext> immediateContext;
		if (FAILED (D3D11CreateDevice (nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL,
			createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION,
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
			createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
			swapChain, device, nullptr, immediate)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT ReadAndCompile (LPCTSTR filename, LPCSTR profile, LPCSTR main, void* buffer, UINT maxLength, UINT* bufferLength)
{
	uint8_t innerBuffer[8192];
	UINT readLength;
	if (FAILED (ReadAllData (filename, innerBuffer, 8192, &readLength)))
		return E_FAIL;

	CComPtr<ID3DBlob> blob, errMsg;
	if (FAILED (D3DCompile (innerBuffer, readLength, nullptr, nullptr, nullptr,
		main, profile, D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &blob, &errMsg)))
	{
		OutputDebugStringA ((LPCSTR)(errMsg->GetBufferPointer ()));
		return E_FAIL;
	}

	memcpy (buffer, blob->GetBufferPointer (), min (maxLength, blob->GetBufferSize ()));
	*bufferLength = blob->GetBufferSize ();

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

HRESULT LoadTexture2D (ID3D11Device* d3dDevice, LPCTSTR filename, ID3D11Texture2D** texture)
{
#ifndef UNICODE
	USES_CONVERSION;
#endif

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

	LPCWSTR convedFilename;
#ifndef UNICODE
	convedFilename = A2W (readFile);
#else
	convedFilename = readFile;
#endif

	CComPtr<IWICImagingFactory> wicFactory;
	if (FAILED (CoCreateInstance (CLSID_WICImagingFactory, nullptr, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)& wicFactory)))
		return E_FAIL;

	CComPtr<IWICBitmapDecoder> wicDecoder;
	if (FAILED (wicFactory->CreateDecoderFromFilename (convedFilename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder)))
		return E_FAIL;

	CComPtr<IWICBitmapFrameDecode> frameDecode;
	if (FAILED (wicDecoder->GetFrame (0, &frameDecode)))
		return E_FAIL;

	CComPtr<IWICFormatConverter> formatConverter;
	if (FAILED (wicFactory->CreateFormatConverter (&formatConverter)))
		return E_FAIL;

	if (FAILED (formatConverter->Initialize (frameDecode, GUID_WICPixelFormat32bppBGRA,
		WICBitmapDitherTypeNone, nullptr, 1, WICBitmapPaletteTypeCustom)))
		return E_FAIL;

	UINT width, height;
	formatConverter->GetSize (&width, &height);
	UINT stride = width * 4, totalSize = stride * height;

	std::vector<BYTE> buffer (totalSize);
	if (FAILED (formatConverter->CopyPixels (nullptr, stride, totalSize, buffer.data ())))
		return E_FAIL;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA textureInitialData = {};
	textureInitialData.pSysMem = buffer.data ();
	textureInitialData.SysMemPitch = stride;
	textureInitialData.SysMemSlicePitch = totalSize;

	if (FAILED (d3dDevice->CreateTexture2D (&textureDesc, &textureInitialData, texture)))
		return E_FAIL;

	return S_OK;
}

HRESULT LoadTextureCube (ID3D11Device* d3dDevice, LPCTSTR px, LPCTSTR nx, LPCTSTR py, LPCTSTR ny, LPCTSTR pz, LPCTSTR nz, ID3D11Texture2D** texture)
{
#ifndef UNICODE
	USES_CONVERSION;
#endif
	std::vector<BYTE> cubeTextureBuffer;
	UINT width = 0, height = 0;
	UINT stride = 0, totalSize = 0;

	CComPtr<IWICImagingFactory> wicFactory;
	if (FAILED (CoCreateInstance (CLSID_WICImagingFactory, nullptr, CLSCTX_ALL, __uuidof(IWICImagingFactory), (void**)& wicFactory)))
		return E_FAIL;

	LPCTSTR filenames[6] = { px, nx, py, ny, pz, nz };
	for (LPCTSTR filename : filenames)
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

		LPCWSTR convedFilename;
#ifndef UNICODE
		convedFilename = A2W (readFile);
#else
		convedFilename = readFile;
#endif

		CComPtr<IWICBitmapDecoder> wicDecoder;
		if (FAILED (wicFactory->CreateDecoderFromFilename (convedFilename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder)))
			return E_FAIL;

		CComPtr<IWICBitmapFrameDecode> frameDecode;
		if (FAILED (wicDecoder->GetFrame (0, &frameDecode)))
			return E_FAIL;

		CComPtr<IWICFormatConverter> formatConverter;
		if (FAILED (wicFactory->CreateFormatConverter (&formatConverter)))
			return E_FAIL;

		if (FAILED (formatConverter->Initialize (frameDecode, GUID_WICPixelFormat32bppBGRA,
			WICBitmapDitherTypeNone, nullptr, 1, WICBitmapPaletteTypeCustom)))
			return E_FAIL;

		UINT twidth, theight;
		formatConverter->GetSize (&twidth, &theight);
		if (width != 0 && height != 0)
		{
			if (width != twidth || height != theight)
				return E_FAIL;
		}
		else
		{
			width = twidth;
			height = theight;

			stride = width * 4;
			totalSize = stride * height;
		}
		UINT offset = cubeTextureBuffer.size ();

		cubeTextureBuffer.resize (cubeTextureBuffer.size () + totalSize);

		if (FAILED (formatConverter->CopyPixels (nullptr, stride, totalSize, cubeTextureBuffer.data () + offset)))
			return E_FAIL;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.ArraySize = 6;
	textureDesc.MipLevels = 1;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SUBRESOURCE_DATA textureInitialData [6] = {};

	for (int i = 0; i < 6; ++i)
	{
		textureInitialData[i].pSysMem = cubeTextureBuffer.data () + (totalSize * i);
		textureInitialData[i].SysMemPitch = stride;
		textureInitialData[i].SysMemSlicePitch = totalSize;
	}

	if (FAILED (d3dDevice->CreateTexture2D (&textureDesc, textureInitialData, texture)))
		return E_FAIL;

	return S_OK;
}

DirectX::XMFLOAT3 GetNormalVector (const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT3& c)
{
	DirectX::XMVECTOR va, vb, vc;
	va = DirectX::XMLoadFloat3 (&a);
	vb = DirectX::XMLoadFloat3 (&b);
	vc = DirectX::XMLoadFloat3 (&c);

	DirectX::XMVECTOR temp1 = DirectX::XMVectorSubtract (vb, va)
		, temp2 = DirectX::XMVectorSubtract (vc, va);
	DirectX::XMVECTOR temp3 = DirectX::XMVector3Normalize (DirectX::XMVector3Cross (temp1, temp2));

	DirectX::XMFLOAT3 ret;
	DirectX::XMStoreFloat3 (&ret, temp3);

	return ret;
}

struct FRAMEWORK_VERTEX
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texcoord;
	DirectX::XMFLOAT4 color;
};

UINT FrameworkVertexStride () { return sizeof (FRAMEWORK_VERTEX); }

HRESULT CreateTriangle (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices)
{
	static FRAMEWORK_VERTEX vertexList[] = {
		{ {-1, -1, 0}, {0, 0, 1}, {0, 1}, {1, 1, 1, 1} },
		{ {+0, +1, 0}, {0, 0, 1}, {0.5f, 0}, {1, 1, 1, 1} },
		{ {+1, -1, 0}, {0, 0, 1}, {1, 1}, {1, 1, 1, 1} },
	};

	if (d3dDevice == nullptr || buffer == nullptr || vertices == nullptr)
		return E_INVALIDARG;

	*vertices = _countof (vertexList);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof (vertexList);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertexList;
	initialData.SysMemPitch = sizeof (vertexList);

	return d3dDevice->CreateBuffer (&bufferDesc, &initialData, buffer);
}

HRESULT CreateRectangle (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices)
{
	static FRAMEWORK_VERTEX vertexList[] = {
		{ {-1, +1, 0}, {0, 0, 1}, {0, 0}, {1, 1, 1, 1} },
		{ {-1, -1, 0}, {0, 0, 1}, {0, 1}, {1, 1, 1, 1} },
		{ {+1, +1, 0}, {0, 0, 1}, {1, 0}, {1, 1, 1, 1} },

		{ {-1, -1, 0}, {0, 0, 1}, {0, 1}, {1, 1, 1, 1} },
		{ {+1, -1, 0}, {0, 0, 1}, {1, 1}, {1, 1, 1, 1} },
		{ {+1, +1, 0}, {0, 0, 1}, {1, 0}, {1, 1, 1, 1} },
	};

	if (d3dDevice == nullptr || buffer == nullptr || vertices == nullptr)
		return E_INVALIDARG;

	*vertices = _countof (vertexList);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof (vertexList);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertexList;
	initialData.SysMemPitch = sizeof (vertexList);

	return d3dDevice->CreateBuffer (&bufferDesc, &initialData, buffer);
}

HRESULT CreateBox (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices)
{
	static FRAMEWORK_VERTEX vertexList[] = {
		{ { -1, -1, -1 }, {0, 0, -1}, { 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {0, 0, -1}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, -1, -1 }, {0, 0, -1}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {0, 0, -1}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {0, 0, -1}, { 0, 0 }, { 1, 1, 1, 1 } },
		{ { -1, +1, -1 }, {0, 0, -1}, { 0, 1 }, { 1, 1, 1, 1 } },

		{ { -1, -1, +1 }, {0, 0, +1}, { 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {0, 0, +1}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {0, 0, +1}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {0, 0, +1}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { -1, +1, +1 }, {0, 0, +1}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, +1 }, {0, 0, +1}, { 0, 0 }, { 1, 1, 1, 1 } },

		{ { -1, +1, +1 }, {-1, 0, 0}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { -1, +1, -1 }, {-1, 0, 0}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {-1, 0, 0}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {-1, 0, 0}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, +1 }, {-1, 0, 0}, { 0, 0 }, { 1, 1, 1, 1 } },
		{ { -1, +1, +1 }, {-1, 0, 0}, { 1, 0 }, { 1, 1, 1, 1 } },

		{ { +1, +1, +1 }, {+1, 0, 0}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, -1 }, {+1, 0, 0}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {+1, 0, 0}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, -1, -1 }, {+1, 0, 0}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {+1, 0, 0}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {+1, 0, 0}, { 0, 0 }, { 1, 1, 1, 1 } },

		{ { -1, -1, -1 }, {0, -1, 0}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { +1, -1, -1 }, {0, -1, 0}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {0, -1, 0}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {0, -1, 0}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { -1, -1, +1 }, {0, -1, 0}, { 0, 0 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {0, -1, 0}, { 0, 1 }, { 1, 1, 1, 1 } },

		{ { -1, +1, -1 }, {0, +1, 0}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {0, +1, 0}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {0, +1, 0}, { 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {0, +1, 0}, { 1, 0 }, { 1, 1, 1, 1 } },
		{ { -1, +1, -1 }, {0, +1, 0}, { 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, +1, +1 }, {0, +1, 0}, { 0, 0 }, { 1, 1, 1, 1 } },
	};

	if (d3dDevice == nullptr || buffer == nullptr || vertices == nullptr)
		return E_INVALIDARG;

	*vertices = _countof (vertexList);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof (vertexList);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertexList;
	initialData.SysMemPitch = sizeof (vertexList);

	return d3dDevice->CreateBuffer (&bufferDesc, &initialData, buffer);
}

HRESULT CreateSphere (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices)
{
	if (d3dDevice == nullptr || buffer == nullptr || vertices == nullptr)
		return E_INVALIDARG;

	std::vector<FRAMEWORK_VERTEX> vertexList;
	constexpr unsigned rings = 40;
	constexpr unsigned sectors = 40;

	float const R = 1.0f / (float)(rings - 1);
	float const S = 1.0f / (float)(sectors - 1);
	unsigned r, s;

	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT2> texcoords;
	for (r = 0; r < rings; r++) {
		for (s = 0; s < sectors; s++) {
			float const y = sin ((-3.141592f / 2) + 3.141592f * r * R);
			float const x = cos (2 * 3.141592f * s * S) * sin (3.141592f * r * R);
			float const z = sin (2 * 3.141592f * s * S) * sin (3.141592f * r * R);
			positions.push_back (DirectX::XMFLOAT3 (x, y, z));
			texcoords.push_back (DirectX::XMFLOAT2 (s * S, r * R));
		}
	}

	std::vector<unsigned> indices;
	std::vector<DirectX::XMFLOAT3> normals;
	for (r = 0; r < rings - 1; r++) {
		for (s = 0; s < sectors - 1; s++) {
			int curRow = r * sectors;
			int nextRow = (r + 1) * sectors;

			indices.push_back (curRow + s);
			indices.push_back (nextRow + s);
			indices.push_back (nextRow + (s + 1));

			DirectX::XMFLOAT3 normal1 = GetNormalVector (positions[curRow + s], positions[nextRow + s], positions[nextRow + (s + 1)]);
			normals.push_back (normal1);
			normals.push_back (normal1);
			normals.push_back (normal1);

			indices.push_back (curRow + s);
			indices.push_back (nextRow + (s + 1));
			indices.push_back (curRow + (s + 1));

			DirectX::XMFLOAT3 normal2 = GetNormalVector (positions[curRow + s], positions[nextRow + (s + 1)], positions[curRow + (s + 1)]);
			normals.push_back (normal2);
			normals.push_back (normal2);
			normals.push_back (normal2);
		}
	}

	vertexList.resize (indices.size ());
	int j = 0;
	for (auto i = indices.begin (); i != indices.end (); ++i)
	{
		FRAMEWORK_VERTEX vtx;
		vtx.position = positions[*i];
		vtx.normal = normals[j];
		vtx.texcoord = texcoords[*i];
		vtx.color = DirectX::XMFLOAT4 (1, 1, 1, 1);
		vertexList[j++] = vtx;
	}

	*vertices = vertexList.size ();

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = vertexList.size () * sizeof (FRAMEWORK_VERTEX);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertexList.data ();
	initialData.SysMemPitch = sizeof (vertexList);

	return d3dDevice->CreateBuffer (&bufferDesc, &initialData, buffer);
}

FILE* fopen_fw (LPCTSTR filename, char* mode)
{
#ifdef UNICODE
	USES_CONVERSION;
#endif

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
				return nullptr;
		}
	}

	LPCSTR convedFilename;
#ifdef UNICODE
	convedFilename = W2A (readFile);
#else
	convedFilename = readFile;
#endif

	return fopen (readFile, mode);
}

HRESULT CreateModelFromOBJFile (ID3D11Device* d3dDevice, LPCTSTR filename, ID3D11Buffer** buffer, UINT* vertices)
{
	if (d3dDevice == nullptr || buffer == nullptr || vertices == nullptr)
		return E_INVALIDARG;

	std::vector<unsigned> vIndices, uvIndices, nIndices;
	std::vector<DirectX::XMFLOAT3> tempVertices;
	std::vector<DirectX::XMFLOAT2> tempUVs;
	std::vector<DirectX::XMFLOAT3> tempNormals;

	FILE* fp = fopen_fw (filename, "rt");
	if (fp == nullptr)
		return E_FAIL;

	char lineHeader[256];
	char stupidBuffer[1024];

	enum { POSITION = 0, TEXCOORD = 1, NORMAL = 2 };
	int prop = POSITION;

	while (true)
	{
		int res = fscanf (fp, "%s", lineHeader);
		if (res == EOF) break;

		if (strcmp (lineHeader, "v") == 0)
		{
			DirectX::XMFLOAT3 position;
			fscanf (fp, "%f %f %f\n", &position.x, &position.y, &position.z);
			position.z = 0 - position.z;
			tempVertices.push_back (position);
		}
		else if (strcmp (lineHeader, "vt") == 0)
		{
			DirectX::XMFLOAT2 uv;
			fscanf (fp, "%f %f\n", &uv.x, &uv.y);
			uv.y = 1 - uv.y;
			tempUVs.push_back (uv);
			prop |= TEXCOORD;
		}
		else if (strcmp (lineHeader, "vn") == 0)
		{
			DirectX::XMFLOAT3 normal;
			fscanf (fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normal.z = 0 - normal.z;
			tempNormals.push_back (normal);
			prop |= NORMAL;
		}
		else if (strcmp (lineHeader, "f") == 0)
		{
			unsigned vi[3] = { 0, }, uvi[3] = { 0, }, ni[3] = { 0, };
			if (prop == (POSITION | NORMAL | TEXCOORD)) {
				int matches = fscanf (fp, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vi[0], &uvi[0], &ni[0],
					&vi[1], &uvi[1], &ni[1], &vi[2], &uvi[2], &ni[2]);
				if (matches != 9)
					return E_FAIL;
			}
			else if (prop == (POSITION | NORMAL))
			{
				int matches = fscanf (fp, "%d//%d %d//%d %d//%d\n", &vi[0], &ni[0],
					&vi[1], &ni[1], &vi[2], &ni[2]);
				if (matches != 6)
					return E_FAIL;
			}
			else if (prop == (POSITION | TEXCOORD))
			{
				int matches = fscanf (fp, "%d/%d %d/%d %d/%d\n", &vi[0], &uvi[0],
					&vi[1], &uvi[1], &vi[2], &uvi[2]);
				if (matches != 6)
					return E_FAIL;
			}
			else if (prop == POSITION)
			{
				int matches = fscanf (fp, "%d %d %d\n", &vi[0], &vi[1], &vi[2]);
				if (matches != 3)
					return E_FAIL;
			}

			vIndices.push_back (vi[1]);
			vIndices.push_back (vi[0]);
			vIndices.push_back (vi[2]);

			if (prop & TEXCOORD)
			{
				uvIndices.push_back (uvi[1]);
				uvIndices.push_back (uvi[0]);
				uvIndices.push_back (uvi[2]);
			}
			if (prop & NORMAL)
			{
				nIndices.push_back (ni[1]);
				nIndices.push_back (ni[0]);
				nIndices.push_back (ni[2]);
			}
		}
		else
			fgets (stupidBuffer, 1024, fp);
	}

	fclose (fp);

	std::vector<FRAMEWORK_VERTEX> vertexList;
	for (int i = 0; i < vIndices.size (); ++i)
	{
		FRAMEWORK_VERTEX vertex = {};
		vertex.color = DirectX::XMFLOAT4 (1, 1, 1, 1);

		unsigned vertexIndex = vIndices[i] - 1;
		DirectX::XMFLOAT3 position = tempVertices[vertexIndex];
		vertex.position = position;

		if (prop & NORMAL)
		{
			unsigned normalIndex = nIndices[i] - 1;
			DirectX::XMFLOAT3 normal = tempNormals[normalIndex];
			vertex.normal = normal;
		}

		if (prop & TEXCOORD)
		{
			unsigned uvIndex = uvIndices[i] - 1;
			DirectX::XMFLOAT2 uv = tempUVs[uvIndex];
			vertex.texcoord = uv;
		}

		vertexList.push_back (vertex);
	}

	*vertices = vertexList.size ();

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = vertexList.size () * sizeof (FRAMEWORK_VERTEX);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertexList.data ();
	initialData.SysMemPitch = sizeof (vertexList);

	return d3dDevice->CreateBuffer (&bufferDesc, &initialData, buffer);
}

struct FRAMEWORK_SKYBOX_VERTEX
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 texcoord;
	DirectX::XMFLOAT4 color;
};

UINT FrameworkSkyboxVertexStride () { return sizeof (FRAMEWORK_SKYBOX_VERTEX); }

HRESULT CreateSkyboxBox (ID3D11Device* d3dDevice, ID3D11Buffer** buffer, UINT* vertices)
{
	static FRAMEWORK_SKYBOX_VERTEX vertexList[] = {
		{ { +1, -1, -1 }, {0, 0, -1}, { 1, 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {0, 0, -1}, { 1, 1, 0 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {0, 0, -1}, { 0, 0, 0 }, { 1, 1, 1, 1 } },
		{ { -1, +1, -1 }, {0, 0, -1}, { 0, 1, 0 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {0, 0, -1}, { 0, 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {0, 0, -1}, { 1, 1, 0 }, { 1, 1, 1, 1 } },

		{ { +1, +1, +1 }, {0, 0, +1}, { 1, 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {0, 0, +1}, { 1, 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, +1 }, {0, 0, +1}, { 0, 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, +1 }, {0, 0, +1}, { 0, 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, +1, +1 }, {0, 0, +1}, { 0, 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {0, 0, +1}, { 1, 1, 1 }, { 1, 1, 1, 1 } },

		{ { -1, +1, +1 }, {-1, 0, 0}, { 0, 1, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {-1, 0, 0}, { 0, 0, 0 }, { 1, 1, 1, 1 } },
		{ { -1, +1, -1 }, {-1, 0, 0}, { 0, 1, 0 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {-1, 0, 0}, { 0, 0, 0 }, { 1, 1, 1, 1 } },
		{ { -1, +1, +1 }, {-1, 0, 0}, { 0, 1, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, +1 }, {-1, 0, 0}, { 0, 0, 1 }, { 1, 1, 1, 1 } },

		{ { +1, +1, +1 }, {+1, 0, 0}, { 1, 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {+1, 0, 0}, { 1, 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, -1 }, {+1, 0, 0}, { 1, 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, -1 }, {+1, 0, 0}, { 1, 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {+1, 0, 0}, { 1, 0, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {+1, 0, 0}, { 1, 1, 1 }, { 1, 1, 1, 1 } },

		{ { -1, -1, -1 }, {0, -1, 0}, { 0, 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {0, -1, 0}, { 1, 0, 1 }, { 1, 1, 1, 1 } },
		{ { +1, -1, -1 }, {0, -1, 0}, { 1, 0, 0 }, { 1, 1, 1, 1 } },
		{ { +1, -1, +1 }, {0, -1, 0}, { 1, 0, 1 }, { 1, 1, 1, 1 } },
		{ { -1, -1, -1 }, {0, -1, 0}, { 0, 0, 0 }, { 1, 1, 1, 1 } },
		{ { -1, -1, +1 }, {0, -1, 0}, { 0, 0, 1 }, { 1, 1, 1, 1 } },

		{ { -1, +1, -1 }, {0, +1, 0}, { 0, 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, -1 }, {0, +1, 0}, { 1, 1, 0 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {0, +1, 0}, { 1, 1, 1 }, { 1, 1, 1, 1 } },
		{ { +1, +1, +1 }, {0, +1, 0}, { 1, 1, 1 }, { 1, 1, 1, 1 } },
		{ { -1, +1, +1 }, {0, +1, 0}, { 0, 1, 1 }, { 1, 1, 1, 1 } },
		{ { -1, +1, -1 }, {0, +1, 0}, { 0, 1, 0 }, { 1, 1, 1, 1 } },
	};

	if (d3dDevice == nullptr || buffer == nullptr || vertices == nullptr)
		return E_INVALIDARG;

	*vertices = _countof (vertexList);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof (vertexList);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertexList;
	initialData.SysMemPitch = sizeof (vertexList);

	return d3dDevice->CreateBuffer (&bufferDesc, &initialData, buffer);
}
