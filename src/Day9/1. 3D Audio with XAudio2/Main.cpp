#include <framework.h>

#include <vector>

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
	CComPtr<IXAudio2> xaudio;
	IXAudio2MasteringVoice* masteringVoice;
	UINT masteringVoiceChannels;
	IXAudio2SourceVoice* sourceVoice[4];
	X3DAUDIO_HANDLE x3dInstance;

	X3DAUDIO_LISTENER audioListener = {};
	X3DAUDIO_EMITTER audioEmitters[4] = {};

public:
	DirectX::XMFLOAT3 cameraPosition;
	float yaw, pitch, roll;
	DirectX::XMFLOAT3 moveFactor;

public:
	DirectX::XMFLOAT3 rabbitPos[4] = {
		{ -3, 0, -3 },
		{ +3, 0, -3 },
		{ -3, 0, +3 },
		{ +3, 0, +3 },
	};

public:
	virtual HRESULT Initialize (HWND hWnd, UINT width, UINT height) override
	{
		if (FAILED (MFStartup (MF_VERSION)))
			return E_FAIL;

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

		if (FAILED (XAudio2Create (&xaudio)))
			return E_FAIL;

		if (FAILED (xaudio->StartEngine ()))
			return E_FAIL;

		if (FAILED (xaudio->CreateMasteringVoice (&masteringVoice)))
			return E_FAIL;

		DWORD dwChannelMask;
		masteringVoice->GetChannelMask (&dwChannelMask);
		XAUDIO2_VOICE_DETAILS masteringVoiceDetails;
		masteringVoice->GetVoiceDetails (&masteringVoiceDetails);
		masteringVoiceChannels = masteringVoiceDetails.InputChannels;

		if(FAILED(X3DAudioInitialize (dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, x3dInstance)))
			return E_FAIL;

		audioListener.OrientTop = DirectX::XMFLOAT3 (0, 1, 0);
		audioListener.Position = cameraPosition;
		audioListener.OrientFront = DirectX::XMFLOAT3 (0, 0, 0);
		audioListener.Velocity = DirectX::XMFLOAT3 (0, 0, 0);

		static LPCTSTR audioFiles[] = {
			TEXT ("Day9//1//Sample1.mp3"),
			TEXT ("Day9//1//Sample2.mp3"),
			TEXT ("Day9//1//Sample3.mp3"),
			TEXT ("Day9//1//Sample4.mp3")
		};

		int index = 0;
		for (LPCTSTR filename : audioFiles)
		{
			CComPtr<IStream> stream;
			CComPtr<IMFByteStream> mfStream;
			CComPtr<IMFSourceReader> reader;

			if (FAILED (CreateStream (filename, STGM_READ, &stream)))
				return E_FAIL;
			if (FAILED (MFCreateMFByteStreamOnStream (stream, &mfStream)))
				return E_FAIL;
			if (FAILED (MFCreateSourceReaderFromByteStream (mfStream, nullptr, &reader)))
				return E_FAIL;

			CComPtr<IMFMediaType> mediaType;
			if (FAILED (MFCreateMediaType (&mediaType)))
				return E_FAIL;
			mediaType->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			mediaType->SetGUID (MF_MT_SUBTYPE, MFAudioFormat_PCM);
			mediaType->SetUINT32 (MF_MT_AUDIO_NUM_CHANNELS, 1);
			mediaType->SetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
			mediaType->SetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);

			if (FAILED (reader->SetCurrentMediaType (MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaType)))
				return E_FAIL;

			WAVEFORMATEX* sourceFormat;
			sourceFormat = (WAVEFORMATEX*)CoTaskMemAlloc (sizeof (WAVEFORMATEX));
			if (sourceFormat == nullptr)
				return E_FAIL;

			sourceFormat->cbSize = sizeof (WAVEFORMATEX);
			sourceFormat->nChannels = 1;
			sourceFormat->wBitsPerSample = 16;
			sourceFormat->nSamplesPerSec = 44100;
			sourceFormat->nBlockAlign = sourceFormat->nChannels * (sourceFormat->wBitsPerSample / 8);
			sourceFormat->nAvgBytesPerSec = sourceFormat->nBlockAlign * sourceFormat->nSamplesPerSec;
			sourceFormat->wFormatTag = WAVE_FORMAT_PCM;

			HRESULT hr;
			if (FAILED (hr = xaudio->CreateSourceVoice (&sourceVoice[index], sourceFormat)))
			{
				CoTaskMemFree (sourceFormat);
				return E_FAIL;
			}

			CoTaskMemFree (sourceFormat);

			std::vector<BYTE> audioData;
			while (true)
			{
				CComPtr<IMFSample> sample;
				DWORD flags;
				DWORD actualStream;
				if (FAILED (reader->ReadSample (MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &actualStream, &flags, nullptr, &sample)))
					return E_FAIL;
				if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
					break;

				CComPtr<IMFMediaBuffer> buffer;
				if (FAILED (sample->ConvertToContiguousBuffer (&buffer)))
					return E_FAIL;

				BYTE* bufferData;
				DWORD length;
				if (FAILED (buffer->Lock (&bufferData, nullptr, &length)))
					return E_FAIL;

				size_t offset = audioData.size ();
				audioData.resize (audioData.size () + length);
				memcpy (audioData.data () + offset, bufferData, length);

				buffer->Unlock ();
			}

			if ((audioData.size () + 3) / 4 * 4 != audioData.size ())
				audioData.resize ((audioData.size () + 3) / 4 * 4);

			XAUDIO2_BUFFER xaudioBuffer = {};
			xaudioBuffer.AudioBytes = audioData.size ();
			xaudioBuffer.pAudioData = audioData.data ();
			xaudioBuffer.Flags = XAUDIO2_END_OF_STREAM;
			xaudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
			if (FAILED (sourceVoice[index]->SubmitSourceBuffer (&xaudioBuffer)))
				return E_FAIL;

			audioEmitters[index].ChannelCount = 1;
			audioEmitters[index].CurveDistanceScaler = FLT_MIN;
			audioEmitters[index].DopplerScaler = 1;
			audioEmitters[index].Position = rabbitPos[index];
			audioEmitters[index].Velocity = DirectX::XMFLOAT3 (0, 0, 0);
			audioEmitters[index].OrientTop = DirectX::XMFLOAT3 (0, 1, 0);
			audioEmitters[index].OrientFront = DirectX::XMFLOAT3 (0, 0, 0);

			float matrixCoefficients[8];
			X3DAUDIO_DSP_SETTINGS dspSettings = { matrixCoefficients };
			X3DAudioCalculate (x3dInstance, &audioListener, &audioEmitters[index],
				X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dspSettings);
			
			if (FAILED (sourceVoice[index]->SetOutputMatrix (masteringVoice, 1, masteringVoiceChannels, dspSettings.pMatrixCoefficients, 0)))
				return E_FAIL;

			if(FAILED(sourceVoice[index]->Start ()))
				return E_FAIL;

			++index;
		}

		return S_OK;
	}

public:
	~Application ()
	{
		for (int i = 0; i < 4; ++i)
		{
			if (sourceVoice[i])
				sourceVoice[i]->DestroyVoice ();
			sourceVoice[i] = nullptr;
		}

		if (masteringVoice) masteringVoice->DestroyVoice (); masteringVoice = nullptr;

		MFShutdown ();

		xaudio->StopEngine ();
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

		audioListener.Position = cameraPosition;
		DirectX::XMFLOAT3 target, up;
		DirectX::XMStoreFloat3 (&target, targetVector);
		DirectX::XMStoreFloat3 (&up, upVector);
		audioListener.OrientFront = target;
		audioListener.OrientTop = up;
		audioListener.Velocity = moveFactor;

		moveFactor = DirectX::XMFLOAT3 (0, 0, 0);

		for (int i = 0; i < 4; ++i)
		{
			float matrixCoefficients[8];
			X3DAUDIO_DSP_SETTINGS dspSettings = { matrixCoefficients };
			X3DAudioCalculate (x3dInstance, &audioListener, &audioEmitters[i],
				X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dspSettings);

			sourceVoice[i]->SetOutputMatrix (masteringVoice, 1, masteringVoiceChannels, dspSettings.pMatrixCoefficients, 0);
		}
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

		for (DirectX::XMFLOAT3 rabbitPosition : rabbitPos)
		{
			transform.world = DirectX::XMMatrixTranslation (rabbitPosition.x, rabbitPosition.y, rabbitPosition.z);
			immediateContext->UpdateSubresource (constantBuffer, 0, nullptr, &transform, sizeof (TRANSFORM), 0);
			immediateContext->Draw (vertices, 0);
		}

		dxgiSwapChain->Present (1, 0);
	}
};

REGISTER_IDUR (Application);