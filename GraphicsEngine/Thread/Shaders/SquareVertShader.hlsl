
struct VSinput
{
    uint id : SV_VertexID;
};

struct FSinput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

FSinput main(VSinput input)
{
    float4 pos = float4(0, 0, 0, 1);
    float4 color = float4(0, 0, 0, 0);
	
    if (input.id == 0)
    {
        pos.xy = float2(-0.5f, 0.5f);
        color = float4(1, 0, 0, 1);
    }
    if (input.id == 1)
    {
        pos.xy = float2(0.5f, 0.5f);
        color = float4(0, 1, 0, 1);
    }
    if (input.id == 2)
    {
        pos.xy = float2(0.5f, -0.5f);
        color = float4(0, 0, 1, 1);
    }
    if (input.id == 3)
    {
        pos.xy = float2(-0.5f, -0.5f);
        color = float4(1, 1, 1, 1);
    }
    FSinput result;
    result.pos = pos;
    result.color = color;
    
    return result;
}