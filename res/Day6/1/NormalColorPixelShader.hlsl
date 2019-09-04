struct PIXEL_IN
{
    float4 position1 : SV_Position;
    float4 position2 : POSITION;
    float4 color : COLOR;
};

struct PIXEL_OUT
{
    float4 target1 : SV_Target0;
    float4 target2 : SV_Target1;
};

PIXEL_OUT main(PIXEL_IN pin)
{
    PIXEL_OUT pout;
    pout.target1 = pin.color;
    pout.target2 = pin.position2;
    return pout;
}