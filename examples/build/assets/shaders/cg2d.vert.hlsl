struct SpriteData
{
    float3 Position;
    float Rotation;
    float4 hand;
    float4 uv;
    float4 mask;
    float4 Color;
    float2 modeType;
    float2 Scale;
    float4 skew;
};



struct Output
{
    float2 Texcoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 modeType :TEXCOORD2;
    float4 mask : TEXCOORD3;
    float4 outlinew :TEXCOORD4;
    float4 Position : SV_Position;


};

// WARNING: StructuredBuffers are not natively supported by SDL's GPU API.
// They will work with SDL_shadercross because it does special processing to
// support them, but not with direct compilation via dxc.
// See https://github.com/libsdl-org/SDL/issues/12200 for details.
StructuredBuffer<SpriteData> DataBuffer : register(t0, space0);

cbuffer UniformBlock : register(b0, space1)
{
    float4x4 ViewProjectionMatrix;
    float2 scrn;
    float2 orig;//curently unused
};

static const uint triangleIndices[6] = {0, 1, 2, 3, 2, 1};

Output main(uint id : SV_VertexID)
{
    uint spriteIndex = id / 6;
    uint vert = triangleIndices[id % 6];
    SpriteData sprite = DataBuffer[spriteIndex];
    
    //offset vert positions by width,height and scewx/y
    //hand.z and hand.w are the width and height of the sprite
    float2 verts[4]={
        {0.0 + sprite.skew.x,           0.0 + sprite.skew.y},
        {sprite.hand.z + sprite.skew.x, 0.0 - sprite.skew.y},
        {0.0 - sprite.skew.x,           sprite.hand.w + sprite.skew.y},
        {sprite.hand.z - sprite.skew.x, sprite.hand.w - sprite.skew.y}
    };
    
    float2 texcoord[4] = {
        {sprite.uv.x,               sprite.uv.y              },
        {sprite.uv.x + sprite.uv.z, sprite.uv.y              },
        {sprite.uv.x,               sprite.uv.y + sprite.uv.w},
        {sprite.uv.x + sprite.uv.z, sprite.uv.y + sprite.uv.w}
    };

    float c = cos(sprite.Rotation);
    float s = sin(sprite.Rotation);

    //subtract the handle from the coord
    float2 coord = verts[vert]-float2(sprite.hand.x*sprite.hand.z,sprite.hand.y*sprite.hand.w);
   
    //scale;
    coord *= sprite.Scale;

    //rotation
    float2x2 rotation = {c, s, -s, c};
    coord = mul(coord, rotation);

    //final vert
    float3 coordWithDepth = float3(coord + sprite.Position.xy, sprite.Position.z);    


    //set output data
    Output output;
    output.mask=sprite.mask;
    output.outlinew=sprite.skew;
    output.Position = mul(ViewProjectionMatrix, float4(coordWithDepth, 1.0f));
    output.Texcoord = texcoord[vert];
    output.Color = sprite.Color;
    output.modeType=float4(sprite.modeType,0.0f,0.0f);   

    return output;
}
