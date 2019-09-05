#include <framework.h>

#define SAFE_RELEASE(x)										if ((x)) (x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;
ID3D11Texture2D* customRenderTargetBuffer;
ID3D11ShaderResourceView* customRenderTargetBufferSRV;
ID3D11RenderTargetView* customRenderTarget;
ID3D11Texture2D* depthStencilBuffer;
ID3D11DepthStencilView* depthStencilView;

ID3D11Buffer* vertexBuffer;
UINT vertices;
ID3D11Buffer* transformConstantBuffer;
ID3D11InputLayout* inputLayout;
ID3D11VertexShader* renderVertexShader, * drawVertexShader;
ID3D11PixelShader* renderPixelShader, * drawPixelShader;

ID3D11RasterizerState* rasterizerState;

ID3D11Texture2D* rwBuffer;
ID3D11UnorderedAccessView* rwBufferUAV;
ID3D11ComputeShader* gaussianBlurShader;

D3D11_VIEWPORT viewport = {};

float transformRotationAngle;

struct TRANSFORM
{
	DirectX::XMMATRIX worldTransform;
	DirectX::XMMATRIX viewTransform;
	DirectX::XMMATRIX projectionTransform;
};
TRANSFORM transform;

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

	D3D11_TEXTURE2D_DESC renderTargetDesc = {};
	renderTargetDesc.ArraySize = 1;
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = height;
	renderTargetDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (FAILED (d3dDevice->CreateTexture2D (&renderTargetDesc, nullptr, &customRenderTargetBuffer)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateRenderTargetView (customRenderTargetBuffer, nullptr, &customRenderTarget)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateShaderResourceView (customRenderTargetBuffer, nullptr, &customRenderTargetBufferSRV)))
		return E_FAIL;

	renderTargetDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	if (FAILED (d3dDevice->CreateTexture2D (&renderTargetDesc, nullptr, &rwBuffer)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateUnorderedAccessView (rwBuffer, nullptr, &rwBufferUAV)))
		return E_FAIL;

	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.Width = width;
	depthStencilBufferDesc.Height = height;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;

	if (FAILED (d3dDevice->CreateTexture2D (&depthStencilBufferDesc, nullptr, &depthStencilBuffer)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateDepthStencilView (depthStencilBuffer, nullptr, &depthStencilView)))
		return E_FAIL;

	if (FAILED (CreateModelFromOBJFile (d3dDevice, TEXT ("Day7\\Common\\Sample.obj"), &vertexBuffer, &vertices)))
		return E_FAIL;

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof (TRANSFORM);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &transformConstantBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day7\\Common\\RenderToTexture.hlsl"), RAC_VERTEXSHADER, "vs_main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day7\\Common\\RenderToTexture.hlsl"), RAC_PIXELSHADER, "ps_main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &renderVertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &renderPixelShader)))
		return E_FAIL;

	if (FAILED (CreateInputLayoutFromVertexShader (d3dDevice, vertexShaderData, vertexShaderDataLength, &inputLayout)))
		return E_FAIL;

	if (FAILED (ReadAndCompile (TEXT ("Day7\\Common\\RenderToScreen.hlsl"), RAC_VERTEXSHADER, "vs_main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day7\\Common\\RenderToScreen.hlsl"), RAC_PIXELSHADER, "ps_main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &drawVertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &drawPixelShader)))
		return E_FAIL;

	uint8_t computeShaderData[4096];
	UINT computeShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day7\\2\\GaussianBlur.hlsl"), RAC_COMPUTESHADER, "main", computeShaderData, 4096, &computeShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateComputeShader (computeShaderData, computeShaderDataLength, nullptr, &gaussianBlurShader)))
		return E_FAIL;

	D3D11_RASTERIZER_DESC rasterizerStateDesc = {};
	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	if (FAILED (d3dDevice->CreateRasterizerState (&rasterizerStateDesc, &rasterizerState)))
		return E_FAIL;

	DirectX::XMFLOAT3 camPosOrigin = DirectX::XMFLOAT3 (3, 3, 3),
		camTargetOrigin = DirectX::XMFLOAT3 (0, 0, 0),
		camUpOrigin = DirectX::XMFLOAT3 (0, 1, 0);
	DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3 (&camPosOrigin),
		camTarget = DirectX::XMLoadFloat3 (&camTargetOrigin),
		camUp = DirectX::XMLoadFloat3 (&camUpOrigin);
	transform.viewTransform = DirectX::XMMatrixLookAtLH (camPos, camTarget, camUp);
	transform.projectionTransform = DirectX::XMMatrixPerspectiveFovLH (3.141592f / 4, width / (float)height, 0.0001f, 1000);

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (rasterizerState);

	SAFE_RELEASE (drawPixelShader);
	SAFE_RELEASE (drawVertexShader);
	SAFE_RELEASE (renderPixelShader);
	SAFE_RELEASE (renderVertexShader);
	SAFE_RELEASE (inputLayout);
	SAFE_RELEASE (transformConstantBuffer);
	SAFE_RELEASE (vertexBuffer);

	SAFE_RELEASE (depthStencilBuffer);
	SAFE_RELEASE (depthStencilView);
	SAFE_RELEASE (gaussianBlurShader);
	SAFE_RELEASE (rwBufferUAV);
	SAFE_RELEASE (rwBuffer);
	SAFE_RELEASE (customRenderTargetBufferSRV);
	SAFE_RELEASE (customRenderTargetBuffer);
	SAFE_RELEASE (customRenderTarget);
	SAFE_RELEASE (renderTargetView);

	SAFE_RELEASE (immediateContext);
	SAFE_RELEASE (d3dDevice);
	SAFE_RELEASE (dxgiSwapChain);
}

void Update (float dt)
{
	transformRotationAngle += dt;
}

void Render (float dt)
{
	static ID3D11ShaderResourceView* srvs[] = { nullptr };
	static ID3D11RenderTargetView* rtvs[] = { nullptr };
	static float clearColor[] = { 0, 0, 0, 1 };

	immediateContext->ClearState ();
	immediateContext->ClearRenderTargetView (customRenderTarget, clearColor);
	immediateContext->ClearDepthStencilView (depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);

	immediateContext->OMSetRenderTargets (1, &customRenderTarget, depthStencilView);
	immediateContext->RSSetViewports (1, &viewport);
	immediateContext->RSSetState (rasterizerState);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides = FrameworkVertexStride (), offset = 0;
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer, &strides, &offset);
	immediateContext->IASetInputLayout (inputLayout);

	immediateContext->VSSetShader (renderVertexShader, nullptr, 0);
	immediateContext->VSSetConstantBuffers (0, 1, &transformConstantBuffer);
	immediateContext->PSSetShader (renderPixelShader, nullptr, 0);

	transform.worldTransform = DirectX::XMMatrixRotationY (transformRotationAngle);
	immediateContext->UpdateSubresource (transformConstantBuffer, 0, nullptr, &transform, sizeof (transform), 0);
	immediateContext->Draw (vertices, 0);

	D3D11_TEXTURE2D_DESC texDesc;
	rwBuffer->GetDesc (&texDesc);
	immediateContext->ClearState ();
	immediateContext->CSSetShader (gaussianBlurShader, nullptr, 0);
	immediateContext->CSSetShaderResources (0, 1, &customRenderTargetBufferSRV);
	immediateContext->CSSetUnorderedAccessViews (0, 1, &rwBufferUAV, nullptr);
	immediateContext->Dispatch (
		(int)ceil (texDesc.Width / 16.0),
		(int)ceil (texDesc.Height / 16.0),
		1);
	immediateContext->CopyResource (customRenderTargetBuffer, rwBuffer);

	immediateContext->ClearState ();
	immediateContext->ClearRenderTargetView (renderTargetView, clearColor);
	immediateContext->OMSetRenderTargets (1, &renderTargetView, nullptr);
	immediateContext->RSSetViewports (1, &viewport);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediateContext->IASetVertexBuffers (0, 0, nullptr, nullptr, nullptr);
	immediateContext->IASetInputLayout (nullptr);

	immediateContext->VSSetShader (drawVertexShader, nullptr, 0);
	immediateContext->PSSetShader (drawPixelShader, nullptr, 0);
	immediateContext->PSSetShaderResources (0, 1, &customRenderTargetBufferSRV);

	immediateContext->Draw (4, 0);

	dxgiSwapChain->Present (0, 0);
}