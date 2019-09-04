#include <framework.h>

#define SAFE_RELEASE(x)										if ((x)) (x)->Release ()

IDXGISwapChain* dxgiSwapChain;
ID3D11Device* d3dDevice;
ID3D11DeviceContext* immediateContext;

ID3D11RenderTargetView* renderTargetView;

ID3D11Texture2D* customRenderTargetBuffer1, * customRenderTargetBuffer2;
ID3D11ShaderResourceView* customRenderTargetBuffer1SRV, * customRenderTargetBuffer2SRV;
ID3D11RenderTargetView* customRenderTargetView1, * customRenderTargetView2;

ID3D11Buffer* vertexBuffer;
UINT vertices;
ID3D11Buffer* constantBuffer;
ID3D11InputLayout* inputLayout;
ID3D11VertexShader* vertexShader1, * vertexShader2;
ID3D11PixelShader* pixelShader1, * pixelShader2;

D3D11_VIEWPORT viewport1 = {}, viewport2 = {};

float transformRotationAngle;

struct TRANSFORM
{
	DirectX::XMMATRIX worldTransform;
	DirectX::XMMATRIX viewTransform;
	DirectX::XMMATRIX projectionTransform;
} transform;

HRESULT Initialize (HWND hWnd, UINT width, UINT height)
{
	if (FAILED (CreateDevice11 (hWnd, width, height, &d3dDevice, &immediateContext, &dxgiSwapChain)))
		return E_FAIL;

	viewport1.TopLeftX = viewport1.TopLeftY = viewport2.TopLeftX = viewport2.TopLeftY = 0;
	viewport1.Width = width;
	viewport1.Height = height;
	viewport2.Width = 256;
	viewport2.Height = 256;
	viewport1.MinDepth = viewport2.MinDepth = 0;
	viewport1.MaxDepth = viewport2.MaxDepth = 1;

	ID3D11Texture2D* backBuffer;
	if (FAILED (dxgiSwapChain->GetBuffer (0, __uuidof(ID3D11Texture2D), (void**)& backBuffer)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateRenderTargetView (backBuffer, nullptr, &renderTargetView)))
		return E_FAIL;

	backBuffer->Release ();

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Width = 256;
	textureDesc.Height = 256;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	if (FAILED (d3dDevice->CreateTexture2D (&textureDesc, nullptr, &customRenderTargetBuffer1)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateTexture2D (&textureDesc, nullptr, &customRenderTargetBuffer2)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateRenderTargetView (customRenderTargetBuffer1, nullptr, &customRenderTargetView1)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateRenderTargetView (customRenderTargetBuffer2, nullptr, &customRenderTargetView2)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateShaderResourceView (customRenderTargetBuffer1, nullptr, &customRenderTargetBuffer1SRV)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreateShaderResourceView (customRenderTargetBuffer2, nullptr, &customRenderTargetBuffer2SRV)))
		return E_FAIL;

	if (FAILED (CreateBox (d3dDevice, &vertexBuffer, &vertices)))
		return E_FAIL;

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof (TRANSFORM);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	if (FAILED (d3dDevice->CreateBuffer (&constantBufferDesc, nullptr, &constantBuffer)))
		return E_FAIL;

	uint8_t vertexShaderData[4096], pixelShaderData[4096];
	UINT vertexShaderDataLength, pixelShaderDataLength;
	if (FAILED (ReadAndCompile (TEXT ("Day6\\1\\NormalColorVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day6\\1\\NormalColorPixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader1)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader1)))
		return E_FAIL;

	if (FAILED (ReadAndCompile (TEXT ("Day6\\1\\TextureVertexShader.hlsl"), RAC_VERTEXSHADER, "main", vertexShaderData, 4096, &vertexShaderDataLength)))
		return E_FAIL;
	if (FAILED (ReadAndCompile (TEXT ("Day6\\1\\TexturePixelShader.hlsl"), RAC_PIXELSHADER, "main", pixelShaderData, 4096, &pixelShaderDataLength)))
		return E_FAIL;

	if (FAILED (d3dDevice->CreateVertexShader (vertexShaderData, vertexShaderDataLength, nullptr, &vertexShader2)))
		return E_FAIL;
	if (FAILED (d3dDevice->CreatePixelShader (pixelShaderData, pixelShaderDataLength, nullptr, &pixelShader2)))
		return E_FAIL;

	if (FAILED (CreateInputLayoutFromVertexShader (d3dDevice, vertexShaderData, vertexShaderDataLength, &inputLayout)))
		return E_FAIL;

	float fov = width / (float)height;

	DirectX::XMFLOAT3 camPosOrigin = DirectX::XMFLOAT3 (5, 5, 5),
		camTargetOrigin = DirectX::XMFLOAT3 (0, 0, 0),
		camUpOrigin = DirectX::XMFLOAT3 (0, 1, 0);
	DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3 (&camPosOrigin),
		camTarget = DirectX::XMLoadFloat3 (&camTargetOrigin),
		camUp = DirectX::XMLoadFloat3 (&camUpOrigin);
	transform.viewTransform = DirectX::XMMatrixLookAtLH (camPos, camTarget, camUp);

	transform.projectionTransform = DirectX::XMMatrixPerspectiveFovLH (3.141592f / 4, fov, 0.0001f, 1000);

	return S_OK;
}

void Destroy ()
{
	SAFE_RELEASE (pixelShader2);
	SAFE_RELEASE (vertexShader2);
	SAFE_RELEASE (pixelShader1);
	SAFE_RELEASE (vertexShader1);
	SAFE_RELEASE (inputLayout);
	SAFE_RELEASE (vertexBuffer);

	SAFE_RELEASE (customRenderTargetBuffer2SRV);
	SAFE_RELEASE (customRenderTargetBuffer2);
	SAFE_RELEASE (customRenderTargetView2);
	SAFE_RELEASE (customRenderTargetBuffer1SRV);
	SAFE_RELEASE (customRenderTargetBuffer1);
	SAFE_RELEASE (customRenderTargetView1);
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
	UINT stride = FrameworkVertexStride (), offset = 0;

	float clearColor1[] = { 0x64 / 255.0f, 0x95 / 255.0f, 0xed / 255.0f, 1 };
	float clearColor2[] = { 0xed / 255.0f, 0x95 / 255.0f, 0x64 / 255.0f, 1 };
	immediateContext->ClearRenderTargetView (customRenderTargetView1, clearColor1);
	immediateContext->ClearRenderTargetView (customRenderTargetView2, clearColor2);

	ID3D11RenderTargetView* renderTargets[] = { customRenderTargetView1, customRenderTargetView2 };
	immediateContext->OMSetRenderTargets (2, renderTargets, nullptr);
	immediateContext->RSSetViewports (1, &viewport2);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediateContext->IASetInputLayout (inputLayout);
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer, &stride, &offset);

	immediateContext->VSSetShader (vertexShader1, nullptr, 0);
	immediateContext->VSSetConstantBuffers (0, 1, &constantBuffer);
	immediateContext->PSSetShader (pixelShader1, nullptr, 0);

	transform.worldTransform = DirectX::XMMatrixRotationY (transformRotationAngle);
	immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (transform), 0);

	immediateContext->Draw (vertices, 0);

	float clearColor3[] = { 0, 0, 0, 1 };
	immediateContext->ClearRenderTargetView (renderTargetView, clearColor3);

	immediateContext->OMSetRenderTargets (1, &renderTargetView, nullptr);
	immediateContext->RSSetViewports (1, &viewport1);

	immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediateContext->IASetInputLayout (inputLayout);
	immediateContext->IASetVertexBuffers (0, 1, &vertexBuffer, &stride, &offset);

	immediateContext->VSSetShader (vertexShader2, nullptr, 0);
	immediateContext->VSSetConstantBuffers (0, 1, &constantBuffer);
	immediateContext->PSSetShader (pixelShader2, nullptr, 0);
	immediateContext->PSSetShaderResources (0, 1, &customRenderTargetBuffer1SRV);

	transform.worldTransform = DirectX::XMMatrixMultiply (
		DirectX::XMMatrixRotationY (transformRotationAngle),
		DirectX::XMMatrixTranslation (2, 0, 0)
	);
	immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (transform), 0);

	immediateContext->Draw (vertices, 0);

	immediateContext->PSSetShaderResources (0, 1, &customRenderTargetBuffer2SRV);

	transform.worldTransform = DirectX::XMMatrixMultiply (
		DirectX::XMMatrixRotationY (transformRotationAngle),
		DirectX::XMMatrixTranslation (-2, 0, 0)
	);
	immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (transform), 0);

	immediateContext->Draw (vertices, 0);

	dxgiSwapChain->Present (0, 0);
}