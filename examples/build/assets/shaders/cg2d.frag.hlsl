#define M_PI 3.14159265358979323846
#define CG2D_DEG2RAD 3.14159 / 180.0

Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

cbuffer UBO : register(b0, space3)
{
    float2 screenRes;
    float time;
    float2 pos;
};


struct Input
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float2 modeType : TEXCOORD2;
    float4 mask : TEXCOORD3;
    float4 outlinew : TEXCOORD4;
    
};

float4 main(Input input) : SV_Target0
{   
    //set the mode and type
    //type is the type of primitive being crawn
    //mode is set to 1 if the layer is set to mask blend in cg2d
    float type=input.modeType.y;
    float mode=input.modeType.x;

    float4 outCol=input.Color;



    //if it's a image
    if(type==3.0){
        outCol=input.Color * Texture.Sample(Sampler, input.TexCoord);

    //if it's an oval    
    }else if(type==4){
        float dst=distance(input.TexCoord.xy,float2(0.5,0.5));
        if(dst<0.5){
            outCol=input.Color;
        }else{
            discard;
        }
    //else if its a textured line  
    }else if(type==5){
        outCol=input.Color * Texture.Sample(Sampler, input.TexCoord);
    
    //if its a rect outline
    }else if(type==6){
        float maxX = 1.0 - input.outlinew.z;
        float minX = input.outlinew.z;
        float maxY = maxX / (16/9);
        float minY = minX / (16/9);

       if (input.TexCoord.x < maxX && input.TexCoord.x > minX &&
           input.TexCoord.y < maxY && input.TexCoord.y > minY) {
            discard;
       }
    //oval outline
    }else if(type==7){
        float dst=distance(input.TexCoord.xy,float2(0.5,0.5));
        if(dst<0.5 && dst>0.5-input.outlinew.z){
            outCol=input.Color;
        }else{
            discard;
        }
    
    //point gradiated
    }else if(type==9){
        float dst=distance(input.TexCoord.xy,float2(0.5,0.5));
        if(dst<0.5){
            outCol=input.Color;
            outCol.w=1.0-(dst*2);
        }else{
            discard;
        }
    }

    //discard if layer is maskblend and alpha is <0.5
    if(mode==1 && outCol.w<0.5){
        discard;
    }

    //discard if its the mask colour
    if(mode==1){
        if(outCol.r==input.mask.r && outCol.g==input.mask.g && outCol.g==input.mask.b){
            discard;
        }
    }

    

    
   


    return outCol;
   
}
