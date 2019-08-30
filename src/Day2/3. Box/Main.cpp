#include <framework.h>

#define SAFE_RELEASE(x)										(x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;

ID3D11Buffer* vertexBuffer;
ID3D11Buffer* constantBuffer;
ID3D11InputLayout* inputLayout;
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

ID3D11RasterizerState* rasterizerState;

D3D11_VIEWPORT viewport = {};

float transformRotationAngle;
float fov;

struct VERTEX
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct TRANSFORM
{
	DirectX::XMMATRIX worldTransform;
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

	VERTEX vertices[] =
	{
		{ { -0.5f, -0.5f, -0.5f }, { 1, 0, 0, 1 } },
		{ { +0.5f, +0.5f, -0.5f }, { 1, 0, 0, 1 } },
		{ { +0.5f, -0.5f, -0.5f }, { 1, 0, 0, 1 } },
		{ { +0.5f, +0.5f, -0.5f }, { 1, 0, 0, 1 } },
		{ { -0.5f, -0.5f, -0.5f }, { 1, 0, 0, 1 } },
		{ { -0.5f, +0.5f, -0.5f }, { 1, 0, 0, 1 } },

		{ { -0.5f, -0.5f, +0.5f }, { 0, 1, 0, 1 } },
		{ { +0.5f, -0.5f, +0.5f }, { 0, 1, 0, 1 } },
		{ { +0.5f, +0.5f, +0.5f }, { 0, 1, 0, 1 } },
		{ { +0.5f, +0.5f, +0.5f }, { 0, 1, 0, 1 } },
		{ { -0.5f, +0.5f, +0.5f }, { 0, 1, 0, 1 } },
		{ { -0.5f, -0.5f, +0.5f }, { 0, 1, 0, 1 } },

		{ { -0.5f, +0.5f, +0.5f }, { 0, 0, 1, 1 } },
		{ { -0.5f, +0.5f, -0.5f }, { 0, 0, 1, 1 } },
		{ { -0.5f, -0.5f, -0.5f }, { 0, 0, 1, 1 } },
		{ { -0.5f, -0.5f, -0.5f }, { 0, 0, 1, 1 } },
		{ { -0.5f, -0.5f, +0.5f }, { 0, 0, 1, 1 } },
		{ { -0.5f, +0.5f, +0.5f }, { 0, 0, 1, 1 } },

		{ { +0.5f, +0.5f, +0.5f }, { 1, 1, 0, 1 } },
		{ { +0.5f, -0.5f, -0.5f }, { 1, 1, 0, 1 } },
		{ { +0.5f, +0.5f, -0.5f }, { 1, 1, 0, 1 } },
		{ { +0.5f, -0.5f, -0.5f }, { 1, 1, 0, 1 } },
		{ { +0.5f, +0.5f, +0.5f }, { 1, 1, 0, 1 } },
		{ { +0.5f, -0.5f, +0.5f }, { 1, 1, 0, 1 } },

		{ { -0.5f, -0.5f, -0.5f }, { 0, 1, 1, 1 } },
		{ { +0.5f, -0.5f, -0.5f }, { 0, 1, 1, 1 } },
		{ { +0.5f, -0.5f, +0.5f }, { 0, 1, 1, 1 } },
		{ { +0.5f, -0.5f, +0.5f }, { 0, 1, 1, 1 } },
		{ { -0.5f, -0.5f, +0.5f }, { 0, 1, 1, 1 } },
		{ { -0.5f, -0.5f, -0.5f }, { 0, 1, 1, 1 } },

		{ { -0.5f, +0.5f, -0.5f }, { 1, 0, 1, 1 } },
		{ { +0.5f, +0.5f, +0.5f }, { 1, 0, 1, 1 } },
		{ { +0.5f, +0.5f, -0.5f }, { 1, 0, 1, 1 } },
		{ { +0.5f, +0.5f, +0.5f }, { 1, 0, 1, 1 } },
		{ { -0.5f, +0.5f, -0.5f }, { 1, 0, 1, 1 } },
		{ { -0.5f, +0.5f, +0.5f }, { 1, 0, 1, 1 } },
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

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof (TRANSFORM);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &constantBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day2\\3\\TriangleVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day2\\3\\TrianglePixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader)))
		return E_FAIL;

	if (FAILED (CreateInputLayoutFromVertexShader (d3dDevice, vertexShaderData, vertexShaderDataLength, &inputLayout)))
		return E_FAIL;

	D3D11_RASTERIZER_DESC rasterizerStateDesc = {};
	rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	if (FAILED (d3dDevice->CreateRasterizerState (&rasterizerStateDesc, &rasterizerState)))
		return E_FAIL;

	fov = width / (float)height;

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (rasterizerState);

	SAFE_RELEASE (pixelShader);
	SAFE_RELEASE (vertexShader);
	SAFE_RELEASE (inputLayout);
	SAFE_RELEASE (constantBuffer);
	SAFE_RELEASE (vertexBuffer);

	SAFE_RELEASE (renderTargetView);

	SAFE_RELEASE (immediateContext);
	SAFE_RELEASE (d3dDevice);
	SAFE_RELEASE (dxgiSwapChain);
}

void Update (float dt)
{
	transformRotationAngle += dt;

	TRANSFORM transform;
	transform.worldTransform = DirectX::XMMatrixRotationY (transformRotationAngle);

	DirectX::XMFLOAT3 camPosOrigin = DirectX::XMFLOAT3 (1, 1, 1),
		camTargetOrigin = DirectX::XMFLOAT3 (0, 0, 0),
		camUpOrigin = DirectX::XMFLOAT3 (0, 1, 0);
	DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3 (&camPosOrigin),
		camTarget = DirectX::XMLoadFloat3 (&camTargetOrigin),
		camUp = DirectX::XMLoadFloat3 (&camUpOrigin);
	transform.viewTransform = DirectX::XMMatrixLookAtLH (camPos, camTarget, camUp);

	transform.projectionTransform = DirectX::XMMatrixPerspectiveFovLH (3.141592f / 4, fov, 0.0001f, 1000);

	immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (transform), 0);
}

void Render (float dt)
{
	float clearColor[] = { 0x64 / 255.0f, 0x95 / 255.0f, 0xed / 255.0f, 1 };
	immediateContext->ClearRenderTargetView (renderTargetView, clearColor);

	immediateContext->OMSetRenderTargets (1, &renderTargetView, nullptr);
	immediateContext->RSSetViewports (1, &viewport);
	immediateContext->RSSetState (rasterizerState);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT strides = sizeof (VERTEX), offset = 0;
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer, &strides, &offset);
	immediateContext->IASetInputLayout (inputLayout);

	immediateContext->VSSetShader (vertexShader, nullptr, 0);
	immediateContext->VSSetConstantBuffers (0, 1, &constantBuffer);
	immediateContext->PSSetShader (pixelShader, nullptr, 0);

	immediateContext->Draw (36, 0);

	dxgiSwapChain->Present (0, 0);
}