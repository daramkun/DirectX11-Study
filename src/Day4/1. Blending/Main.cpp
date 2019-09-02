#include <framework.h>

#define SAFE_RELEASE(x)										if ((x)) (x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;

ID3D11Buffer* vertexBuffer1, * vertexBuffer2;
ID3D11Buffer* constantBuffer;
ID3D11InputLayout* inputLayout;
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

ID3D11BlendState* alphaBlend;
ID3D11BlendState* additiveBlend;

D3D11_VIEWPORT viewport = {};

struct VERTEX
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct TRANSFORM
{
	DirectX::XMMATRIX world;
};

void DrawWithBlend (const DirectX::XMFLOAT3& position, ID3D11BlendState* blendState)
{
	immediateContext->OMSetBlendState (blendState, nullptr, 0xffff);

	TRANSFORM transform;
	transform.world = DirectX::XMMatrixTranslation (position.x - 0.05f, position.y, position.z);
	immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (TRANSFORM), 0);

	UINT stride = sizeof (VERTEX), offset = 0;
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer1, &stride, &offset);
	immediateContext->Draw (3, 0);

	transform.world = DirectX::XMMatrixTranslation (position.x + 0.05f, position.y, position.z);
	immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (TRANSFORM), 0);

	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer2, &stride, &offset);
	immediateContext->Draw (3, 0);
}

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

	VERTEX vertices1[] =
	{
		{ { -0.15f, -0.15f, 0 }, { 1, 0, 1, 0.5f } },
		{ { +0.00f, +0.15f, 0 }, { 1, 0, 1, 0.5f } },
		{ { +0.15f, -0.15f, 0 }, { 1, 0, 1, 0.5f } },
	};
	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertices1;
	initialData.SysMemPitch = sizeof (vertices1);

	if (FAILED (d3dDevice->CreateBuffer (&vertexBufferDesc, &initialData, &vertexBuffer1)))
		return E_FAIL;

	VERTEX vertices2[] =
	{
		{ { -0.15f, -0.15f, 0 }, { 1, 1, 0, 0.5f } },
		{ { +0.00f, +0.15f, 0 }, { 1, 1, 0, 0.5f } },
		{ { +0.15f, -0.15f, 0 }, { 1, 1, 0, 0.5f } },
	};
	initialData.pSysMem = vertices2;
	initialData.SysMemPitch = sizeof (vertices2);

	if (FAILED (d3dDevice->CreateBuffer (&vertexBufferDesc, &initialData, &vertexBuffer2)))
		return E_FAIL;

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof (TRANSFORM);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &constantBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day4\\1\\TriangleVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day4\\1\\TrianglePixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader)))
		return E_FAIL;

	if (FAILED (CreateInputLayoutFromVertexShader (d3dDevice, vertexShaderData, vertexShaderDataLength, &inputLayout)))
		return E_FAIL;

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED (d3dDevice->CreateBlendState (&blendDesc, &alphaBlend)))
		return E_FAIL;

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED (d3dDevice->CreateBlendState (&blendDesc, &additiveBlend)))
		return E_FAIL;

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (additiveBlend);
	SAFE_RELEASE (alphaBlend);

	SAFE_RELEASE (pixelShader);
	SAFE_RELEASE (vertexShader);
	SAFE_RELEASE (inputLayout);
	SAFE_RELEASE (constantBuffer);
	SAFE_RELEASE (vertexBuffer2);
	SAFE_RELEASE (vertexBuffer1);

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
	immediateContext->IASetInputLayout (inputLayout);

	immediateContext->VSSetShader (vertexShader, nullptr, 0);
	immediateContext->VSSetConstantBuffers (0, 1, &constantBuffer);
	immediateContext->PSSetShader (pixelShader, nullptr, 0);

	// No Blend
	DrawWithBlend (DirectX::XMFLOAT3 (-0.5f, +0.5f, 0), nullptr);
	// Alpha Blend
	DrawWithBlend (DirectX::XMFLOAT3 (+0.5f, +0.5f, 0), alphaBlend);
	// Additive Blend
	DrawWithBlend (DirectX::XMFLOAT3 (-0.5f, -0.5f, 0), additiveBlend);

	dxgiSwapChain->Present (0, 0);
}