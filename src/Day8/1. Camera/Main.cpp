#include <framework.h>

struct TRANSFORM
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
};

class Application : IDUR
{
public:
	CComPtr<ID3D11Device> d3dDevice;
	CComPtr<ID3D11DeviceContext> immediateContext;
	CComPtr<IDXGISwapChain> dxgiSwapChain;

public:
	CComPtr<ID3D11RenderTargetView> renderTargetView;
	D3D11_VIEWPORT viewport;
	CComPtr<ID3D11Texture2D> depthStencilBuffer;
	CComPtr<ID3D11DepthStencilView> depthStencilView;

public:
	CComPtr<ID3D11RasterizerState> rasterizerState;

public:
	CComPtr<ID3D11Buffer> vertexBuffer;
	UINT vertices;
	CComPtr<ID3D11Buffer> constantBuffer;
	TRANSFORM transform;

public:
	CComPtr<ID3D11VertexShader> vertexShader;
	CComPtr<ID3D11PixelShader> pixelShader;
	CComPtr<ID3D11InputLayout> inputLayout;

public:
	DirectX::XMFLOAT3 cameraPosition;
	float yaw, pitch, roll;
	DirectX::XMFLOAT3 moveFactor;

public:
	virtual HRESULT Initialize (HWND hWnd, UINT width, UINT height) override
	{
		if (FAILED (CreateDevice11 (hWnd, width, height, &d3dDevice, &immediateContext, &dxgiSwapChain)))
			return E_FAIL;

		CComPtr<ID3D11Texture2D> backBuffer;
		if (FAILED (dxgiSwapChain->GetBuffer (0, __uuidof(ID3D11Texture2D), (void**)& backBuffer)))
			return E_FAIL;
		if (FAILED (d3dDevice->CreateRenderTargetView (backBuffer, nullptr, &renderTargetView)))
			return E_FAIL;

		viewport.TopLeftX = viewport.TopLeftY = 0;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;

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

		if (FAILED (CreateModelFromOBJFile (d3dDevice, TEXT ("Day8\\Common\\Sample.obj"), &vertexBuffer, &vertices)))
			return E_FAIL;

		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = sizeof (TRANSFORM);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &constantBuffer)))
			return E_FAIL;

		uint8_t vertexShaderData[4096], pixelShaderData[4096];
		UINT vertexShaderDataLength, pixelShaderDataLength;
		if (FAILED (ReadAndCompile (TEXT ("Day8\\Common\\RenderObject.hlsl"), RAC_VERTEXSHADER, "vs_main", vertexShaderData, 4096, &vertexShaderDataLength)))
			return E_FAIL;
		if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader)))
			return E_FAIL;

		if (FAILED (ReadAndCompile (TEXT ("Day8\\Common\\RenderObject.hlsl"), RAC_PIXELSHADER, "ps_main", pixelShaderData, 4096, &pixelShaderDataLength)))
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

		transform.world = DirectX::XMMatrixIdentity ();
		transform.proj = DirectX::XMMatrixPerspectiveFovLH (3.141592f / 4, width / (float)height, 0.0001f, 1000);
		cameraPosition = DirectX::XMFLOAT3 (0, 0, 5);

		return S_OK;
	}

public:
	~Application ()
	{

	}

public:
	virtual void Update (float dt) override
	{
		static DirectX::XMFLOAT3 targetInitValue (0, 0, -1);
		static DirectX::XMFLOAT3 upInitValue (0, 1, 0);

		if (IsKeyDown ('W')) moveFactor.z -= dt;
		if (IsKeyDown ('S')) moveFactor.z += dt;
		if (IsKeyDown ('A')) moveFactor.x += dt;
		if (IsKeyDown ('D')) moveFactor.x -= dt;

		if (IsKeyDown (VK_UP)) pitch += dt;
		if (IsKeyDown (VK_DOWN)) pitch -= dt;
		if (IsKeyDown (VK_LEFT)) yaw -= dt;
		if (IsKeyDown (VK_RIGHT)) yaw += dt;

		DirectX::XMMATRIX matrix = DirectX::XMMatrixRotationRollPitchYaw (0, yaw, 0);
		DirectX::XMVECTOR tempVector = DirectX::XMVector3Transform (DirectX::XMLoadFloat3 (&moveFactor), matrix);

		tempVector = DirectX::XMVectorAdd (DirectX::XMLoadFloat3 (&cameraPosition), tempVector);
		DirectX::XMStoreFloat3 (&cameraPosition, tempVector);

		matrix = DirectX::XMMatrixRotationRollPitchYaw (pitch, yaw, roll);
		DirectX::XMVECTOR targetVector = DirectX::XMVector3Transform (DirectX::XMLoadFloat3 (&targetInitValue), matrix);
		DirectX::XMVECTOR upVector = DirectX::XMVector3Transform (DirectX::XMLoadFloat3 (&upInitValue), matrix);

		targetVector = DirectX::XMVectorAdd (targetVector, tempVector);
		transform.view = DirectX::XMMatrixLookAtLH (tempVector, targetVector, upVector);
		moveFactor = DirectX::XMFLOAT3 (0, 0, 0);
	}

	virtual void Render (float dt) override
	{
		static float clearColor[] = { 0, 0, 0, 1 };
		immediateContext->ClearRenderTargetView (renderTargetView, clearColor);
		immediateContext->ClearDepthStencilView (depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);

		ID3D11RenderTargetView* rtvs[] = { renderTargetView };
		immediateContext->OMSetRenderTargets (1, rtvs, depthStencilView);
		immediateContext->RSSetViewports (1, &viewport);
		immediateContext->RSSetState (rasterizerState);

		immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UINT strides = FrameworkVertexStride (), offset = 0;
		ID3D11Buffer* vbs[] = { vertexBuffer };
		immediateContext->IASetVertexBuffers (0, 1, vbs, &strides, &offset);
		immediateContext->IASetInputLayout (inputLayout);

		immediateContext->VSSetShader (vertexShader, nullptr, 0);
		ID3D11Buffer* cbs[] = { constantBuffer };
		immediateContext->VSSetConstantBuffers (0, 1, cbs);
		immediateContext->PSSetShader (pixelShader, nullptr, 0);

		immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (TRANSFORM), 0);
		immediateContext->Draw (vertices, 0);

		dxgiSwapChain->Present (1, 0);
	}
};

REGISTER_IDUR (Application);