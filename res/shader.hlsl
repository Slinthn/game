cbuffer CBUF0 : register(b0) {
  matrix<float, 4, 4> transform;
  matrix<float, 4, 4> view;
  matrix<float, 4, 4> camera;
};

struct VSIn {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 texturepos : TEXTURE;
};

struct PSIn {
  float4 position : SV_POSITION;
  float2 texturepos : TEXTURE;
};

Texture2D shadertexture;
SamplerState samplerstate;

PSIn VertexMain(VSIn input) {
  PSIn output;
  output.position = mul(mul(mul(float4(input.position, 1), transform), camera), view);
  output.texturepos = input.texturepos;

  return output;
}

float4 PixelMain(PSIn input) : SV_TARGET {
  return shadertexture.Sample(samplerstate, input.texturepos);
}
