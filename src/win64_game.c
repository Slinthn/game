void RenderWorldObject(GAME *game, WORLDOBJECT *obj) {
  CreateTransformMatrix(&game->cbuffer0.data.transform, obj->position.x, obj->position.y, obj->position.z, obj->rotation.x, obj->rotation.y, obj->rotation.z);

  UINT stride = sizeof(VERTEX);
  UINT offset = 0;
  game->context->lpVtbl->IASetVertexBuffers(game->context, 0, 1, &obj->model->vertexbuffer, &stride, &offset);
  game->context->lpVtbl->IASetIndexBuffer(game->context, obj->model->indexbuffer, DXGI_FORMAT_R32_UINT, 0);

  DXUpdateResource(game, (ID3D11Resource *)game->cbuffer0.buffer, &game->cbuffer0.data, sizeof(game->cbuffer0.data));

  game->context->lpVtbl->DrawIndexed(game->context, obj->model->facecount * 3, 0, 0);
}

void GameInit(GAME *game) {
  D3D11_INPUT_ELEMENT_DESC ied[3] = {0};
  ied[0].SemanticName = "POSITION";
  ied[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  ied[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

  ied[1].SemanticName = "NORMAL";
  ied[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  ied[1].AlignedByteOffset = 3 * sizeof(r32);
  ied[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

  ied[2].SemanticName = "TEXTURE";
  ied[2].Format = DXGI_FORMAT_R32G32_FLOAT;
  ied[2].AlignedByteOffset = 3 * sizeof(r32) + 3 * sizeof(r32);
  ied[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  game->shader = DXCreateShader(game, L"res\\shaderv.cso", L"res\\shaderp.cso", ied, SizeofArray(ied));

  
  D3D11_INPUT_ELEMENT_DESC iedgui[2] = {0};
  iedgui[0].SemanticName = "POSITION";
  iedgui[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  iedgui[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

  iedgui[1].SemanticName = "TEXTURE";
  iedgui[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  iedgui[1].AlignedByteOffset = 2 * sizeof(r32);
  iedgui[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  game->guishader = DXCreateShader(game, L"res\\guiv.cso", L"res\\guip.cso", iedgui, SizeofArray(iedgui));

  game->trunkmodel = DXCreateVertexBufferPLY(game, LoadPLY(L"res\\trunk.ply"));
  game->cubemodel = DXCreateVertexBufferPLY(game, LoadPLY(L"res\\cube.ply"));

  D3D11_BUFFER_DESC bd = {0};
  bd.ByteWidth = sizeof(game->cbuffer0.data);
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  game->device->lpVtbl->CreateBuffer(game->device, &bd, 0, &game->cbuffer0.buffer);
  game->context->lpVtbl->VSSetConstantBuffers(game->context, 0, 1, &game->cbuffer0.buffer);

  CreatePerspectiveViewMatrix(&game->cbuffer0.data.view, 16.0f / 9.0f, DegreesToRadians(90.0f), 0.1f, 100.0f);

  D3D11_SAMPLER_DESC sd = {0};
  sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  game->device->lpVtbl->CreateSamplerState(game->device, &sd, &game->samplerstate);

  game->context->lpVtbl->PSSetSamplers(game->context, 0, 1, &game->samplerstate);

  game->texture = DXCreateTextureBMP(game, L"res\\bark.bmp");
  
  game->worldobject[0].model = &game->trunkmodel;
}

void GameUpdate(GAME *game) {
  CreateInverseTransformMatrix(&game->cbuffer0.data.camera, 5, 10, 0, DegreesToRadians(90.0f), 0, 0);

  game->context->lpVtbl->ClearRenderTargetView(game->context, game->rendertargetview, (float[4]) {1, 1, 1, 1});
  game->context->lpVtbl->ClearDepthStencilView(game->context, game->depthstencilview, D3D11_CLEAR_DEPTH, 1.0f, 0);

  game->context->lpVtbl->VSSetShader(game->context, game->shader.vertexshader, 0, 0);
  game->context->lpVtbl->PSSetShader(game->context, game->shader.pixelshader, 0, 0);
  game->context->lpVtbl->IASetInputLayout(game->context, game->shader.inputlayout);
  game->context->lpVtbl->IASetPrimitiveTopology(game->context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  game->context->lpVtbl->PSSetShaderResources(game->context, 0, 1, &game->texture.resourceview);

  for (u32 i = 0; i < MAX_WORLDOBJECTS; i++) {
    WORLDOBJECT *obj = &game->worldobject[i];
    if (obj->model == 0)
      continue;

    RenderWorldObject(game, obj);
  }

  PLAYER *player = &game->players[SizeofArray(game->players)];
  while (player != game->players) {
    player--;
    if (player->handle0 == 0)
      continue;

    r32 movspeed = 0.2f;
    player->position.x += player->input.move.x * movspeed;
    player->position.z += player->input.move.y * movspeed;
    player->rotation.x = 0;
   
    if (sqrt(player->input.look.x*player->input.look.x + player->input.look.y*player->input.look.y) > 0.95) {
      r32 roty = atan2f(player->input.look.y, player->input.look.x);
      if (player->rotating != roty) {
        player->rotating = roty;
        player->rotatingprev = player->rotation.y;
        player->rotatetime = 0;
      }

      player->rotatetime = Minf(1, player->rotatetime + 0.06f);
      r32 newrot = atan2f((1 - player->rotatetime) * sinf(player->rotatingprev) + player->rotatetime * sinf(player->rotating), (1 - player->rotatetime) * cosf(player->rotatingprev) + player->rotatetime * cosf(player->rotating));

      player->rotation.y = newrot;
    }

    WORLDOBJECT playerobj = {0};
    playerobj.model = &game->cubemodel;
    playerobj.position = player->position;
    playerobj.rotation = player->rotation;
    playerobj.rotation.x = 0;
    playerobj.rotation.z = 0;
    RenderWorldObject(game, &playerobj);

    player->input.look = (VECTOR2F){0};
  
    RIOutput(player, 0);
  }

  game->context->lpVtbl->VSSetShader(game->context, game->guishader.vertexshader, 0, 0);
  game->context->lpVtbl->PSSetShader(game->context, game->guishader.pixelshader, 0, 0);
  game->context->lpVtbl->IASetInputLayout(game->context, game->guishader.inputlayout);
  game->context->lpVtbl->IASetPrimitiveTopology(game->context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
#if 0
  //for (u32 i = 0; i < 1920*1080 / 2; i++)
    //game->guipixels[i] = 0x12345678;
  
  game->context->lpVtbl->UpdateSubresource(game->context, (ID3D11Resource *)game->guitexture.texture, 0, 0, game->guipixels, APP_WIDTH * sizeof(u32), 0);

  UINT stride = sizeof(r32) * 4;
  UINT offset = 0;
  game->context->lpVtbl->IASetVertexBuffers(game->context, 0, 1, &game->guivertexbuffer, &stride, &offset);

  game->context->lpVtbl->PSSetShaderResources(game->context, 0, 1, &game->guitexture.resourceview);
  game->context->lpVtbl->Draw(game->context, 4, 0);
#endif
  game->swapchain->lpVtbl->Present(game->swapchain, 1, 0);
}
