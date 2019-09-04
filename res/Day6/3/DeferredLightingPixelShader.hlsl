struct PIXEL_IN
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

SamplerState samplerState;
Texture2D colorBuffer;
Texture2D positionBuffer;
Texture2D normalBuffer;

cbuffer LIGHTSOURCE
{
    float4 pointLight[256];
};

float4 main (PIXEL_IN pin) : SV_Target
{
    float4 colorSample = colorBuffer.Sample(samplerState, pin.texcoord);
    float4 positionSample = positionBuffer.Sample(samplerState, pin.texcoord);
    float4 normalSample = normalBuffer.Sample(samplerState, pin.texcoord);

    float4 result = float4(0, 0, 0, 0);
    for (int i = 0; i < 256; ++i)
    {
		if (pointLight[i].w)
        {
            float3 lightDir = positionSample.xyz - pointLight[i].xyz;
            float3 normalizedLightDir = normalize(lightDir);
            float3 color = saturate(dot(-normalizedLightDir, normalSample.xyz));
            result += float4(color, 1) * colorSample;
        }
    }

    return result;
}