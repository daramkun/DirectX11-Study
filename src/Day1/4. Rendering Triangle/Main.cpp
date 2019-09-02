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
	vertexBufferDesc.ByteWidth = sizeof (DirectX::XMFLOAT3) * 3;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	DirectX::XMFLOAT3 vertices[] =
	{
		{ -0.5f, -0.5f, 0},
		{ +0.0f, +0.5f, 0},
		{ +0.5f, -0.5f, 0},
	};
	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertices;
	initialData.SysMemPitch = sizeof (vertices);
	
	if (FAILED (d3dDevice->CreateBuffer (&vertexBufferDesc, &initialData, &vertexBuffer)))
		return E_FAIL;

	char vertexShaderTextData[4096], pixelShaderTextData[4096];
	UINT vertexShaderTextLength, pixelShaderTextLength;
	if (FAILED (ReadAllData (TEXT ("Day1\\TriangleVertexShader.hlsl"), vertexShaderTextData, 4096, &vertexShaderTextLength)))
		return E_FAIL;
	if (FAILED (ReadAllData (TEXT ("Day1\\TrianglePixelShader.hlsl"), pixelShaderTextData, 4096, &pixelShaderTextLength)))
		return E_FAIL;

	ID3DBlob* vertexShaderBlob, * pixelShaderBlob;
	if (FAILED (D3DCompile (vertexShaderTextData, vertexShaderTextLength, nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, nullptr)))
		return E_FAIL;
	if (FAILED (D3DCompile (pixelShaderTextData, pixelShaderTextLength, nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, nullptr)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderBlob->GetBufferPointer (), vertexShaderBlob->GetBufferSize (), nullptr, &vertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderBlob->GetBufferPointer (), pixelShaderBlob->GetBufferSize (), nullptr, &pixelShader)))
		return E_FAIL;

	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	if (FAILED (d3dDevice->CreateInputLayout (inputLayoutDesc, 1, vertexShaderBlob->GetBufferPointer (), vertexShaderBlob->GetBufferSize (), &inputLayout)))
		return E_FAIL;

	pixelShaderBlob->Release ();
	vertexShaderBlob->Release ();

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
	UINT strides = sizeof (DirectX::XMFLOAT3), offset = 0;
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer, &strides, &offset);
	immediateContext->IASetInputLayout (inputLayout);

	immediateContext->VSSetShader (vertexShader, nullptr, 0);
	immediateContext->PSSetShader (pixelShader, nullptr, 0);

	immediateContext->Draw (3, 0);

	dxgiSwapChain->Present (0, 0);
}