
void DXInit(GAME *game, HWND window) {
  u32 width = APP_WIDTH; // TODO change hardcoded value?
  u32 height = APP_HEIGHT;

  DXGI_SWAP_CHAIN_DESC sc = {0};
  sc.BufferDesc.Width = width;
  sc.BufferDesc.Height = height;
  sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sc.SampleDesc.Count = 1;
  sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sc.BufferCount = 2;
  sc.OutputWindow = window;
  sc.Windowed = 1;
  sc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  u32 d3d11flags = 0;
#ifdef SLINAPP_DEBUG
  d3d11flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, d3d11flags, 0, 0, D3D11_SDK_VERSION, &sc, &game->swapchain, &game->device, 0, &game->context);

  ID3D11Texture2D *depthstencil;
  D3D11_TEXTURE2D_DESC td = {0};
  td.Width = width;
  td.Height = height;
  td.MipLevels = 1;
  td.ArraySize = 1;
  td.Format = DXGI_FORMAT_D32_FLOAT;
  td.SampleDesc.Count = 1;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  game->device->lpVtbl->CreateTexture2D(game->device, &td, 0, &depthstencil);

  ID3D11DepthStencilState *depthstencilstate;
  D3D11_DEPTH_STENCIL_DESC dsd = {0};
  dsd.DepthEnable = 1;
  dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dsd.DepthFunc = D3D11_COMPARISON_GREATER;

  game->device->lpVtbl->CreateDepthStencilState(game->device, &dsd, &depthstencilstate);

  D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {0};
  dsvd.Format = DXGI_FORMAT_D32_FLOAT;
  dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

  game->device->lpVtbl->CreateDepthStencilView(game->device, (ID3D11Resource *)depthstencil, &dsvd, &game->depthstencilview);

  game->swapchain->lpVtbl->GetBuffer(game->swapchain, 0, &SIID_ID3D11Texture2D, &game->backbuffer);
  game->device->lpVtbl->CreateRenderTargetView(game->device, (ID3D11Resource *)game->backbuffer, 0, &game->rendertargetview);

  game->context->lpVtbl->OMSetRenderTargets(game->context, 1, &game->rendertargetview, game->depthstencilview);

  D3D11_VIEWPORT vp = {0};
  vp.Width = (r32)width;
  vp.Height = (r32)height;
  vp.MaxDepth = 1;
  game->context->lpVtbl->RSSetViewports(game->context, 1, &vp);

  td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  game->device->lpVtbl->CreateTexture2D(game->device, &td, 0, &game->guitexture.texture);
  D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {0};
  srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvd.Texture2D.MipLevels = (UINT)-1;
  game->device->lpVtbl->CreateShaderResourceView(game->device, (ID3D11Resource *)game->guitexture.texture, &srvd, &game->guitexture.resourceview);

  r32 vertices[] =
  {
    -1, -1, 0, 1,
    -1, 1, 0, 0,
    1, -1, 1, 1,
    1, 1, 1, 0
  };

  D3D11_BUFFER_DESC bd = {0};
  bd.ByteWidth = sizeof(vertices);
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.StructureByteStride = sizeof(r32) * 4;

  D3D11_SUBRESOURCE_DATA sd = {0};
  sd.pSysMem = vertices;

  game->device->lpVtbl->CreateBuffer(game->device, &bd, &sd, &game->guivertexbuffer);
}

MODEL DXCreateVertexBufferPLY(GAME *game, PLY plymodel) {
  MODEL result = {0};

  D3D11_BUFFER_DESC bd = {0};
  bd.ByteWidth = plymodel.header->vertexcount * sizeof(VERTEX);
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.StructureByteStride = sizeof(VERTEX);

  D3D11_SUBRESOURCE_DATA sd = {0};
  sd.pSysMem = plymodel.vertices;

  game->device->lpVtbl->CreateBuffer(game->device, &bd, &sd, &result.vertexbuffer);

  bd.ByteWidth = plymodel.header->facecount * 3 * sizeof(u32);
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.StructureByteStride = sizeof(u32);

  sd.pSysMem = plymodel.faces;

  game->device->lpVtbl->CreateBuffer(game->device, &bd, &sd, &result.indexbuffer);
  result.facecount = plymodel.header->facecount;
  return result;
}

TEXTURE DXCreateTextureBMP(GAME *game, LPWSTR filename) {
  TEXTURE result = {0};
  BMP bmp = LoadBMP(filename);

  D3D11_TEXTURE2D_DESC td = {0};
  td.Width = bmp.header->width;
  td.Height = bmp.header->height;
  td.MipLevels = 1;
  td.ArraySize = 1;
  td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  td.SampleDesc.Count = 1;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  
  game->device->lpVtbl->CreateTexture2D(game->device, &td, 0, &result.texture);
  game->context->lpVtbl->UpdateSubresource(game->context, (ID3D11Resource *)result.texture, 0, 0, bmp.pixels, bmp.header->width * sizeof(u32), 0);

  D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {0};
  srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvd.Texture2D.MipLevels = (UINT)-1;
  game->device->lpVtbl->CreateShaderResourceView(game->device, (ID3D11Resource *)result.texture, &srvd, &result.resourceview);

  return result;
}

SHADER DXCreateShader(GAME *game, LPWSTR vfilename, LPWSTR pfilename, D3D11_INPUT_ELEMENT_DESC *ied, u32 iedcount) {
  SHADER result = {0};
  FILE vshaderfile = LoadFile(vfilename);
  FILE pshaderfile = LoadFile(pfilename);

  game->device->lpVtbl->CreateVertexShader(game->device, vshaderfile.memory, vshaderfile.size, 0, &result.vertexshader);
  game->device->lpVtbl->CreatePixelShader(game->device, pshaderfile.memory, pshaderfile.size, 0, &result.pixelshader);
  game->device->lpVtbl->CreateInputLayout(game->device, ied, iedcount, vshaderfile.memory, vshaderfile.size, &result.inputlayout);

  return result;
}

void DXUpdateResource(GAME *game, ID3D11Resource *resource, void *data, u32 size) {
  D3D11_MAPPED_SUBRESOURCE ms;
  game->context->lpVtbl->Map(game->context, resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
  memcpy(ms.pData, data, size);
  game->context->lpVtbl->Unmap(game->context, resource, 0);
}
