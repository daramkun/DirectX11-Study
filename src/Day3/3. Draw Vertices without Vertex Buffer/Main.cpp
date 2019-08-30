#include <framework.h>

#define SAFE_RELEASE(x)										(x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;

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

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day3\\3\\TriangleVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day3\\3\\TrianglePixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader)))
		return E_FAIL;

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (pixelShader);
	SAFE_RELEASE (vertexShader);

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
	immediateContext->IASetVertexBuffers (0, 0, nullptr, nullptr, nullptr);
	immediateContext->IASetInputLayout (nullptr);

	immediateContext->VSSetShader (vertexShader, nullptr, 0);
	immediateContext->PSSetShader (pixelShader, nullptr, 0);

	immediateContext->Draw (3, 0);

	dxgiSwapChain->Present (0, 0);
}