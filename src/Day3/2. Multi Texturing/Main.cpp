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

ID3D11Texture2D* texture1, * texture2;
ID3D11ShaderResourceView* shaderResourceView1, * shaderResourceView2;

D3D11_VIEWPORT viewport = {};

struct VERTEX
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texcoord;
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

	VERTEX vertices[] =
	{
		{ { -0.5f, -0.5f, 0 }, { 0, 1 } },
		{ { +0.5f, +0.5f, 0 }, { 1, 0 } },
		{ { +0.5f, -0.5f, 0 }, { 1, 1 } },
		{ { +0.5f, +0.5f, 0 }, { 1, 0 } },
		{ { -0.5f, -0.5f, 0 }, { 0, 1 } },
		{ { -0.5f, +0.5f, 0 }, { 0, 0 } },
	};

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof (vertices);
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertices;
	initialData.SysMemPitch = sizeof (vertices);

	if (FAILED (d3dDevice->CreateBuffer (&vertexBufferDesc, &initialData, &vertexBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day3\\2\\TextureVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day3\\2\\TexturePixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader)))
		return E_FAIL;

	if (FAILED (CreateInputLayoutFromVertexShader (d3dDevice, vertexShaderData, vertexShaderDataLength, &inputLayout)))
		return E_FAIL;

	if (FAILED (LoadTexture2D (d3dDevice, TEXT ("Day3\\2\\Sample1.png"), &texture1)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateShaderResourceView (texture1, nullptr, &shaderResourceView1)))
		return E_FAIL;

	if (FAILED (LoadTexture2D (d3dDevice, TEXT ("Day3\\2\\Sample2.jpg"), &texture2)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateShaderResourceView (texture2, nullptr, &shaderResourceView2)))
		return E_FAIL;

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (shaderResourceView2);
	SAFE_RELEASE (shaderResourceView1);
	SAFE_RELEASE (texture2);
	SAFE_RELEASE (texture1);

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
	immediateContext->PSSetShaderResources (0, 1, &shaderResourceView1);
	immediateContext->PSSetShaderResources (1, 1, &shaderResourceView2);

	immediateContext->Draw (6, 0);

	dxgiSwapChain->Present (0, 0);
}