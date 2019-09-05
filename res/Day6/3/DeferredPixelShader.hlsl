struct PIXEL_IN
{
    float4 position1 : SV_Position;
    float4 position2 : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

struct PIXEL_OUT
{
    float4 color : SV_Target0;
    float4 position : SV_Target1;
    float4 normal : SV_Target2;
};

PIXEL_OUT main (PIXEL_IN pin)
{
    PIXEL_OUT pout;
    pout.color = pin.color;
    pout.position = pin.position2;
    pout.normal = float4(normalize(pin.normal), 1);

    return pout;
}