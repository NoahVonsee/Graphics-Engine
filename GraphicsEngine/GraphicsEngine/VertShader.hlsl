
struct VertexData
{
    float3 position;
    float3 normal;
    float2 texcoord;
};

StructuredBuffer<VertexData> vertexDataBuffer : register(t0);

struct CameraData
{
    //float4x4 WVPMatrix;
    float4 clipFromViewX;
    float4 clipFromViewY;
    float4 clipFromViewZ;
    float4 clipFromViewW;
};

ConstantBuffer<CameraData> cameraData : register(b0);

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
    
    float3 localPos = vertexDataBuffer[input.id].position;
    float3 localNormal = vertexDataBuffer[input.id].normal;

    pos = float4(localPos.x, localPos.y, localPos.z, 1);
    
    //pos.x -= 0;
    //pos.y -= 2;
    pos.z += 4;
    
    FSinput result;
    result.pos = mul(pos, float4x4(cameraData.clipFromViewX, cameraData.clipFromViewY, cameraData.clipFromViewZ, cameraData.clipFromViewW));
    
    result.color = float4(localNormal * 0.5f + 0.5f, 1.0f);
    
    return result;
}