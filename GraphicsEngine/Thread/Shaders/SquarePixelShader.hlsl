
struct FSinput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

float4 main(FSinput input) : SV_Target
{
    return input.color;
}