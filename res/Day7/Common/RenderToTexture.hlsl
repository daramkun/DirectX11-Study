struct VERTEX_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

struct PIXEL_IN
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

cbuffer TRANSFORM
{
    float4x4 world : WORLD;
    float4x4 view : VIEW;
    float4x4 proj : PROJECTION;
};

PIXEL_IN vs_main (VERTEX_IN vin)
{
    PIXEL_IN pin;
    pin.position = mul(float4(vin.position, 1), world);
    pin.position = mul(pin.position, view);
    pin.position = mul(pin.position, proj);
    float3 newNormal = normalize(mul(vin.normal, (float3x3) world));
    float dist = dot(newNormal, float3(5, 0, -3));
    pin.color = float4(dist, dist, dist, 1);

    return pin;
}

float4 ps_main (PIXEL_IN pin) : SV_Target
{
    return pin.color;
}