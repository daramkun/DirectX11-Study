struct PIXEL_IN
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

PIXEL_IN vs_main (uint vertexId : SV_VertexID)
{
    PIXEL_IN pin;
    switch (vertexId)
    {
        case 0:
            pin.position = float4(-1, +1, 0, 1);
            pin.texcoord = float2(0, 0);
            break;

        case 1:
            pin.position = float4(+1, +1, 0, 1);
            pin.texcoord = float2(1, 0);
            break;

        case 2:
            pin.position = float4(-1, -1, 0, 1);
            pin.texcoord = float2(0, 1);
            break;

        case 3:
            pin.position = float4(+1, -1, 0, 1);
            pin.texcoord = float2(1, 1);
            break;
    }

    return pin;
}

SamplerState samplerState;
Texture2D tex;

float4 ps_main (PIXEL_IN pin) : SV_Target
{
    return tex.Sample(samplerState, pin.texcoord);
}