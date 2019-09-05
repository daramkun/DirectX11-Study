#include <framework.h>

#define SAFE_RELEASE(x)										if ((x)) (x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;
ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* depthStencilBuffer;
ID3D11ShaderResourceView* depthStencilBufferSRV;
ID3D11RenderTargetView* renderTargets[3];
ID3D11Texture2D* renderTargetBuffers[3];
ID3D11ShaderResourceView* renderTargetBufferSRVs[3];

ID3D11Buffer* vertexBuffer;
UINT vertices;
ID3D11Buffer* transformConstantBuffer;
ID3D11Buffer* lightSourceConstantBuffer;
ID3D11InputLayout* inputLayout;
ID3D11VertexShader* vertexShader1, * vertexShader2;
ID3D11PixelShader* pixelShader1, * pixelShader2;

ID3D11RasterizerState* rasterizerState;
ID3D11DepthStencilState* depthStencilState;

D3D11_VIEWPORT viewport = {};

float transformRotationAngle;

struct TRANSFORM
{
	DirectX::XMMATRIX worldTransform;
	DirectX::XMMATRIX viewTransform;
	DirectX::XMMATRIX projectionTransform;
};
TRANSFORM transform;

struct LIGHTSOURCE
{
	DirectX::XMFLOAT4 PointLights[256];
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

	D3D11_TEXTURE2D_DESC renderTargetDesc = {};
	renderTargetDesc.ArraySize = 1;
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = height;
	renderTargetDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	if (FAILED (d3dDevice->CreateTexture2D (&renderTargetDesc, nullptr, &renderTargetBuffers[0])))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateTexture2D (&renderTargetDesc, nullptr, &renderTargetBuffers[1])))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateTexture2D (&renderTargetDesc, nullptr, &renderTargetBuffers[2])))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateRenderTargetView (renderTargetBuffers[0], nullptr, &renderTargets[0])))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateRenderTargetView (renderTargetBuffers[1], nullptr, &renderTargets[1])))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateRenderTargetView (renderTargetBuffers[2], nullptr, &renderTargets[2])))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateShaderResourceView (renderTargetBuffers[0], nullptr, &renderTargetBufferSRVs[0])))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateShaderResourceView (renderTargetBuffers[1], nullptr, &renderTargetBufferSRVs[1])))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateShaderResourceView (renderTargetBuffers[2], nullptr, &renderTargetBufferSRVs[2])))
		return E_FAIL;

	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.Width = width;
	depthStencilBufferDesc.Height = height;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	if (FAILED (d3dDevice->CreateTexture2D (&depthStencilBufferDesc, nullptr, &depthStencilBuffer)))
		return E_FAIL;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	if (FAILED (d3dDevice->CreateDepthStencilView (depthStencilBuffer, &depthStencilViewDesc, &depthStencilView)))
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc = {};
	depthStencilSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthStencilSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthStencilSRVDesc.Texture2D.MipLevels = 1;

	if (FAILED (d3dDevice->CreateShaderResourceView (depthStencilBuffer, &depthStencilSRVDesc, &depthStencilBufferSRV)))
		return E_FAIL;

	if (FAILED (CreateModelFromOBJFile (d3dDevice, TEXT ("Day6\\3\\Sample.obj"), &vertexBuffer, &vertices)))
	//if (FAILED (CreateBox (d3dDevice, &vertexBuffer, &vertices)))
	//if (FAILED (CreateSphere (d3dDevice, &vertexBuffer, &vertices)))
		return E_FAIL;

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof (TRANSFORM);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &transformConstantBuffer)))
		return E_FAIL;

	constantBufferDesc.ByteWidth = sizeof (LIGHTSOURCE);

	LIGHTSOURCE lightSource = {};
	lightSource.PointLights[0] = DirectX::XMFLOAT4 (0, 5, 0, 1);
	lightSource.PointLights[1] = DirectX::XMFLOAT4 (5, 5, 5, 1);
	//lightSource.PointLights[2] = DirectX::XMFLOAT4 (5, 5, -5, 1);
	//lightSource.PointLights[3] = DirectX::XMFLOAT4 (-5, 5, -5, 1);
	//lightSource.PointLights[4] = DirectX::XMFLOAT4 (-5, 5, 5, 1);

	D3D11_SUBRESOURCE_DATA lightSourceInitialData = {};
	lightSourceInitialData.pSysMem = &lightSource;
	lightSourceInitialData.SysMemPitch = sizeof (LIGHTSOURCE);

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, &lightSourceInitialData, &lightSourceConstantBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day6\\3\\DeferredVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day6\\3\\DeferredPixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader1)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader1)))
		return E_FAIL;

	if (FAILED (CreateInputLayoutFromVertexShader (d3dDevice, vertexShaderData, vertexShaderDataLength, &inputLayout)))
		return E_FAIL;

	if (FAILED (ReadAndCompile (TEXT ("Day6\\3\\DeferredLightingVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day6\\3\\DeferredLightingPixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader2)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader2)))
		return E_FAIL;

	D3D11_RASTERIZER_DESC rasterizerStateDesc = {};
	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	if (FAILED (d3dDevice->CreateRasterizerState (&rasterizerStateDesc, &rasterizerState)))
		return E_FAIL;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.StencilEnable = FALSE;
	if (FAILED (d3dDevice->CreateDepthStencilState (&depthStencilDesc, &depthStencilState)))
		return E_FAIL;

	DirectX::XMFLOAT3 camPosOrigin = DirectX::XMFLOAT3 (15, 15, 15),
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
	SAFE_RELEASE (depthStencilState);
	SAFE_RELEASE (rasterizerState);

	SAFE_RELEASE (pixelShader2);
	SAFE_RELEASE (vertexShader2);
	SAFE_RELEASE (pixelShader1);
	SAFE_RELEASE (vertexShader1);
	SAFE_RELEASE (inputLayout);
	SAFE_RELEASE (lightSourceConstantBuffer);
	SAFE_RELEASE (transformConstantBuffer);
	SAFE_RELEASE (vertexBuffer);

	SAFE_RELEASE (renderTargetBufferSRVs[2]);
	SAFE_RELEASE (renderTargetBuffers[2]);
	SAFE_RELEASE (renderTargets[2]);
	SAFE_RELEASE (renderTargetBufferSRVs[1]);
	SAFE_RELEASE (renderTargetBuffers[1]);
	SAFE_RELEASE (renderTargets[1]);
	SAFE_RELEASE (renderTargetBufferSRVs[0]);
	SAFE_RELEASE (renderTargetBuffers[0]);
	SAFE_RELEASE (renderTargets[0]);
	SAFE_RELEASE (depthStencilBufferSRV);
	SAFE_RELEASE (depthStencilBuffer);
	SAFE_RELEASE (depthStencilView);
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
	float clearColor[] = { 0, 0, 0, 1 };
	immediateContext->ClearRenderTargetView (renderTargets[0], clearColor);
	immediateContext->ClearRenderTargetView (renderTargets[1], clearColor);
	immediateContext->ClearRenderTargetView (renderTargets[2], clearColor);
	immediateContext->ClearDepthStencilView (depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);

	immediateContext->OMSetRenderTargets (3, renderTargets, depthStencilView);
	immediateContext->RSSetViewports (1, &viewport);
	immediateContext->RSSetState (rasterizerState);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides = FrameworkVertexStride (), offset = 0;
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer, &strides, &offset);
	immediateContext->IASetInputLayout (inputLayout);

	immediateContext->OMSetDepthStencilState (depthStencilState, 0);

	immediateContext->VSSetShader (vertexShader1, nullptr, 0);
	immediateContext->VSSetConstantBuffers (0, 1, &transformConstantBuffer);
	immediateContext->PSSetShader (pixelShader1, nullptr, 0);

	static DirectX::XMFLOAT3 objectPositions[] =
	{
		{ -15, 0, -15 }, { -10, 0, -15 }, { -5, 0, -15 }, { 0, 0, -15 }, { 5, 0, -15 }, { 10, 0, -15 }, { 15, 0, -15 },
		{ -15, 0, -10 }, { -10, 0, -10 }, { -5, 0, -10 }, { 0, 0, -10 }, { 5, 0, -10 }, { 10, 0, -10 }, { 15, 0, -10 },
		{ -15, 0, -5 }, { -10, 0, -5 }, { -5, 0, -5 }, { 0, 0, -5 }, { 5, 0, -5 }, { 10, 0, -5 }, { 15, 0, -5 },
		{ -15, 0, 0 }, { -10, 0, 0 }, { -5, 0, 0 }, { 0, 0, 0 }, { 5, 0, 0 }, { 10, 0, 0 }, { 15, 0, 0 },
		{ -15, 0, 5 }, { -10, 0, 5 }, { -5, 0, 5 }, { 0, 0, 5 }, { 5, 0, 5 }, { 10, 0, 5 }, { 15, 0, 5 },
		{ -15, 0, 10 }, { -10, 0, 10 }, { -5, 0, 10 }, { 0, 0, 10 }, { 5, 0, 10 }, { 10, 0, 10 }, { 15, 0, 10 },
		{ -15, 0, 15 }, { -10, 0, 15 }, { -5, 0, 15 }, { 0, 0, 15 }, { 5, 0, 15 }, { 10, 0, 15 }, { 15, 0, 15 },
	};

	for (DirectX::XMFLOAT3& pos : objectPositions)
	{
		transform.worldTransform = DirectX::XMMatrixMultiply (
			DirectX::XMMatrixRotationY (transformRotationAngle),
			DirectX::XMMatrixTranslation (pos.x, pos.y, pos.z)
		);
		immediateContext->UpdateSubresource (transformConstantBuffer, 0, nullptr, &transform, sizeof (transform), 0);
		immediateContext->Draw (vertices, 0);
	}

	immediateContext->ClearRenderTargetView (renderTargetView, clearColor);
	ID3D11RenderTargetView* renderTargetViews[] = { renderTargetView, nullptr, nullptr };
	immediateContext->OMSetRenderTargets (3, renderTargetViews, nullptr);
	immediateContext->RSSetViewports (1, &viewport);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediateContext->IASetVertexBuffers (0, 0, nullptr, nullptr, nullptr);
	immediateContext->IASetInputLayout (nullptr);

	immediateContext->VSSetShader (vertexShader2, nullptr, 0);
	immediateContext->PSSetShader (pixelShader2, nullptr, 0);
	immediateContext->PSSetShaderResources (0, 3, renderTargetBufferSRVs);
	immediateContext->PSSetConstantBuffers (0, 1, &lightSourceConstantBuffer);

	immediateContext->Draw (4, 0);

	dxgiSwapChain->Present (0, 0);
}