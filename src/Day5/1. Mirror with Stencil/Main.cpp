#include <framework.h>

#define SAFE_RELEASE(x)										if ((x)) (x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;
ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* depthStencilBuffer;

ID3D11Buffer* sphereVertexBuffer, * paneVertexBuffer;
UINT sphereVertices, paneVertices;
ID3D11Buffer* worldConstantBuffer;
ID3D11Buffer* viewProjConstantBuffer;
ID3D11InputLayout* inputLayout;
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

ID3D11RasterizerState* rasterizerState;
ID3D11DepthStencilState* depthStencilStateObject, *depthStencilStateStencil, *depthStencilStateMirrored;

D3D11_VIEWPORT viewport = {};

float fov;
float moveFactor;

struct WORLDTRANSFORM
{
	DirectX::XMMATRIX worldTransform;
	DirectX::XMFLOAT4 color;
};

struct VIEWPROJTRANSFORM
{
	DirectX::XMMATRIX viewTransform;
	DirectX::XMMATRIX projectionTransform;
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

	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.Width = width;
	depthStencilBufferDesc.Height = height;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (FAILED (d3dDevice->CreateTexture2D (&depthStencilBufferDesc, nullptr, &depthStencilBuffer)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateDepthStencilView (depthStencilBuffer, nullptr, &depthStencilView)))
		return E_FAIL;

	if (FAILED (CreateSphere (d3dDevice, &sphereVertexBuffer, &sphereVertices)))
		return E_FAIL;
	if (FAILED (CreateRectangle (d3dDevice, &paneVertexBuffer, &paneVertices)))
		return E_FAIL;

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof (WORLDTRANSFORM);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &worldConstantBuffer)))
		return E_FAIL;

	constantBufferDesc.ByteWidth = sizeof (VIEWPROJTRANSFORM);

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &viewProjConstantBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day5\\1\\VertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day5\\1\\PixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader)))
		return E_FAIL;

	if (FAILED (CreateInputLayoutFromVertexShader (d3dDevice, vertexShaderData, vertexShaderDataLength, &inputLayout)))
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
	if (FAILED (d3dDevice->CreateDepthStencilState (&depthStencilDesc, &depthStencilStateObject)))
		return E_FAIL;

	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0xffffffff;
	depthStencilDesc.StencilWriteMask = 0xffffffff;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	if (FAILED (d3dDevice->CreateDepthStencilState (&depthStencilDesc, &depthStencilStateStencil)))
		return E_FAIL;

	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0xffffffff;
	depthStencilDesc.StencilWriteMask = 0xffffffff;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	if (FAILED (d3dDevice->CreateDepthStencilState (&depthStencilDesc, &depthStencilStateMirrored)))
		return E_FAIL;

	fov = width / (float)height;

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (depthStencilStateMirrored);
	SAFE_RELEASE (depthStencilStateStencil);
	SAFE_RELEASE (depthStencilStateObject);
	SAFE_RELEASE (rasterizerState);

	SAFE_RELEASE (pixelShader);
	SAFE_RELEASE (vertexShader);
	SAFE_RELEASE (inputLayout);
	SAFE_RELEASE (viewProjConstantBuffer);
	SAFE_RELEASE (worldConstantBuffer);
	SAFE_RELEASE (paneVertexBuffer);
	SAFE_RELEASE (sphereVertexBuffer);

	SAFE_RELEASE (depthStencilBuffer);
	SAFE_RELEASE (depthStencilView);
	SAFE_RELEASE (renderTargetView);

	SAFE_RELEASE (immediateContext);
	SAFE_RELEASE (d3dDevice);
	SAFE_RELEASE (dxgiSwapChain);
}

void Update (float dt)
{
	VIEWPROJTRANSFORM transform;

	DirectX::XMFLOAT3 camPosOrigin = DirectX::XMFLOAT3 (5, 5, 5),
		camTargetOrigin = DirectX::XMFLOAT3 (0, 0, 0),
		camUpOrigin = DirectX::XMFLOAT3 (0, 1, 0);
	DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3 (&camPosOrigin),
		camTarget = DirectX::XMLoadFloat3 (&camTargetOrigin),
		camUp = DirectX::XMLoadFloat3 (&camUpOrigin);
	transform.viewTransform = DirectX::XMMatrixLookAtLH (camPos, camTarget, camUp);

	transform.projectionTransform = DirectX::XMMatrixPerspectiveFovLH (3.141592f / 4, fov, 0.0001f, 1000);

	immediateContext->UpdateSubresource (viewProjConstantBuffer, 0, nullptr, &transform, sizeof (transform), 0);

	moveFactor += dt;
}

void Render (float dt)
{
	float clearColor[] = { 0, 0, 0, 1 };
	immediateContext->ClearRenderTargetView (renderTargetView, clearColor);
	immediateContext->ClearDepthStencilView (depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	immediateContext->OMSetRenderTargets (1, &renderTargetView, depthStencilView);
	immediateContext->RSSetViewports (1, &viewport);
	immediateContext->RSSetState (rasterizerState);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediateContext->IASetInputLayout (inputLayout);

	immediateContext->VSSetShader (vertexShader, nullptr, 0);
	immediateContext->VSSetConstantBuffers (0, 1, &worldConstantBuffer);
	immediateContext->VSSetConstantBuffers (1, 1, &viewProjConstantBuffer);
	immediateContext->PSSetShader (pixelShader, nullptr, 0);

	UINT strides = FrameworkVertexStride (), offset = 0;
	WORLDTRANSFORM transform;

	// Original Object
	transform.worldTransform = DirectX::XMMatrixTranslation (2 + sin(moveFactor), 0, 0);
	transform.color = DirectX::XMFLOAT4 (1, 0, 1, 1);
	immediateContext->UpdateSubresource (worldConstantBuffer, 0, nullptr, &transform, sizeof (transform), 0);
	
	immediateContext->IASetVertexBuffers (0, 1, &sphereVertexBuffer, &strides, &offset);
	immediateContext->OMSetDepthStencilState (depthStencilStateObject, 0x1);
	immediateContext->Draw (sphereVertices, 0);

	// Stencil Object(Mirror)
	transform.worldTransform = DirectX::XMMatrixRotationY (3.141592f / 2);
	transform.color = DirectX::XMFLOAT4 (0.75f, 0.75f, 0.75f, 1);
	immediateContext->UpdateSubresource (worldConstantBuffer, 0, nullptr, &transform, sizeof (transform), 0);

	immediateContext->IASetVertexBuffers (0, 1, &paneVertexBuffer, &strides, &offset);
	immediateContext->OMSetDepthStencilState (depthStencilStateStencil, 0x1);
	immediateContext->Draw (paneVertices, 0);

	// Mirrored Object
	transform.worldTransform = DirectX::XMMatrixTranslation (-2 - sin (moveFactor), 0, 0);
	transform.color = DirectX::XMFLOAT4 (1, 1, 1, 1);
	immediateContext->UpdateSubresource (worldConstantBuffer, 0, nullptr, &transform, sizeof (transform), 0);

	immediateContext->IASetVertexBuffers (0, 1, &sphereVertexBuffer, &strides, &offset);
	immediateContext->OMSetDepthStencilState (depthStencilStateMirrored, 0x1);
	immediateContext->Draw (sphereVertices, 0);

	dxgiSwapChain->Present (0, 0);
}