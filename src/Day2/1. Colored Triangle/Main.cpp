#include <framework.h>

#define SAFE_RELEASE(x)										if ((x)) (x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;

ID3D11Buffer* vertexBuffer;
ID3D11InputLayout* inputLayout;
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

D3D11_VIEWPORT viewport = {};

struct VERTEX
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

HRESULT Initialize (HWND hWnd, UINT width, UINT height)
{
	if (FAILED (CreateDevice11 (hWnd, width, height, &d3dDevice, &immediateContext, &dxgiSwapChain)))
		return E_FAIL;

	viewport.TopLeftX = viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	ID3D11Texture2D* backBuffer;
	if (FAILED (dxgiSwapChain->GetBuffer (0, __uuidof(ID3D11Texture2D), (void**)& backBuffer)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateRenderTargetView (backBuffer, nullptr, &renderTargetView)))
		return E_FAIL;

	backBuffer->Release ();

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof (VERTEX) * 3;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	VERTEX vertices[] =
	{
		{ { -0.5f, -0.5f, 0 }, { 1, 0, 0, 1 } },
		{ { +0.0f, +0.5f, 0 }, { 0, 1, 0, 1 } },
		{ { +0.5f, -0.5f, 0 }, { 0, 0, 1, 1 } },
	};
	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertices;
	initialData.SysMemPitch = sizeof (vertices);

	if (FAILED (d3dDevice->CreateBuffer (&vertexBufferDesc, &initialData, &vertexBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day2\\1\\TriangleVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day2\\1\\TrianglePixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader)))
		return E_FAIL;

	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	if (FAILED (d3dDevice->CreateInputLayout (inputLayoutDesc, 2, vertexShaderData, vertexShaderDataLength, &inputLayout)))
		return E_FAIL;

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (pixelShader);
	SAFE_RELEASE (vertexShader);
	SAFE_RELEASE (inputLayout);
	SAFE_RELEASE (vertexBuffer);

	SAFE_RELEASE (renderTargetView);

	SAFE_RELEASE (immediateContext);
	SAFE_RELEASE (d3dDevice);
	SAFE_RELEASE (dxgiSwapChain);
}

void Update (float dt)
{

}

void Render (float dt)
{
	float clearColor[] = { 0x64 / 255.0f, 0x95 / 255.0f, 0xed / 255.0f, 1 };
	immediateContext->ClearRenderTargetView (renderTargetView, clearColor);

	immediateContext->OMSetRenderTargets (1, &renderTargetView, nullptr);
	immediateContext->RSSetViewports (1, &viewport);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides = sizeof (VERTEX), offset = 0;
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer, &strides, &offset);
	immediateContext->IASetInputLayout (inputLayout);

	immediateContext->VSSetShader (vertexShader, nullptr, 0);
	immediateContext->PSSetShader (pixelShader, nullptr, 0);

	immediateContext->Draw (3, 0);

	dxgiSwapChain->Present (0, 0);
}