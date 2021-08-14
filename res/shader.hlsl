cbuffer CBUF0 : register(b0) {
  matrix<float, 4, 4> transform;
  matrix<float, 4, 4> view;
  matrix<float, 4, 4> camera;
};

struct VSIN {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 texturepos : TEXTURE;
};

struct PSIN {
  float4 position : SV_POSITION;
  float2 texturepos : TEXTURE;
};

Texture2D shadertexture;
SamplerState samplerstate;

PSIN VertexMain(VSIN input) {
  PSIN result;
  result.position = mul(mul(mul(float4(input.position, 1), transform), camera), view);
  result.texturepos = input.texturepos;
  return result;
}

float4 PixelMain(PSIN input) : SV_TARGET {
  float4 colour = shadertexture.Sample(samplerstate, input.texturepos);
  return colour;
}
