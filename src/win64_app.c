#pragma warning(push, 0)
#include <windows.h>
#include <d3d11.h>
#include <stdint.h>
#include <math.h>
#include <hidusage.h>
#pragma warning(pop)

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef float r32;
typedef double r64;
typedef s32 b32;

typedef r32 MATRIX4F[16];

typedef struct {
  r32 x, y;
} VECTOR2F;

typedef struct {
  r32 x, y, z;
} VECTOR3F;

typedef struct {
  r32 x, y, z, w;
} VECTOR4F;

#define SizeofArray(x) (sizeof(x) / (sizeof((x)[0])))

#define PI_32 3.1415926535897932384626f
#define DegreesToRadians(deg) ((deg) * PI_32 / 180.0f)
#define RadiansToDegrees(rad) ((rad) * 180.0f / PI_32)

r32 Clampf(r32 value, r32 min, r32 max) {
  return (value < min) ? min : ((value > max) ? max : value);
}

void CreateIdentityMatrix(MATRIX4F *m) {
  (*m)[0] = 1;
  (*m)[1] = 0;
  (*m)[2] = 0;
  (*m)[3] = 0;
  (*m)[4] = 0;
  (*m)[5] = 1;
  (*m)[6] = 0;
  (*m)[7] = 0;
  (*m)[8] = 0;
  (*m)[9] = 0;
  (*m)[10] = 1;
  (*m)[11] = 0;
  (*m)[12] = 0;
  (*m)[13] = 0;
  (*m)[14] = 0;
  (*m)[15] = 1;
}

void CreatePerspectiveViewMatrix(MATRIX4F *m, r32 aspect, r32 fov, r32 znear, r32 zfar) {
  CreateIdentityMatrix(m);
  (*m)[0] = 1.0f / (aspect * tanf(fov / 2.0f));
  (*m)[5] = 1.0f / tanf(fov / 2.0f);
  (*m)[10] = (-znear - zfar) / (znear - zfar);
  (*m)[11] = (2.0f * zfar * znear) / (znear - zfar);
  (*m)[14] = 1;
  (*m)[15] = 0;
}

void CreateTransformMatrix(MATRIX4F *m, r32 tx, r32 ty, r32 tz, r32 rx, r32 ry, r32 rz) {
  CreateIdentityMatrix(m);
  (*m)[3] = tx;
  (*m)[7] = ty;
  (*m)[11] = tz;

  r32 srx = sinf(rx);
  r32 crx = cosf(rx);
  r32 sry = sinf(ry);
  r32 cry = cosf(ry);
  r32 srz = sinf(rz);
  r32 crz = cosf(rz);
  
  (*m)[0] = crz*cry;
  (*m)[1] = crz*sry*srx - srz*crx;
  (*m)[2] = crz*sry*crx + srz*srx;
  (*m)[4] = srz*cry;
  (*m)[5] = srz*sry*srx + crz*crx;
  (*m)[6] = srz*sry*crx - crz*srx;
  (*m)[8] = -sry;
  (*m)[9] = cry*srx;
  (*m)[10] = cry*crx;
}

void CreateInverseTransformMatrix(MATRIX4F *m, r32 tx, r32 ty, r32 tz, r32 rx, r32 ry, r32 rz) {
  r32 srx = sinf(rx);
  r32 crx = cosf(rx);
  r32 sry = sinf(ry);
  r32 cry = cosf(ry);
  r32 srz = sinf(rz);
  r32 crz = cosf(rz);
  
  r32 d0 = crz*cry;
  r32 d1 = crz*sry*srx - srz*crx;
  r32 d2 = crz*sry*crx + srz*srx;
  r32 d4 = srz*cry;
  r32 d5 = srz*sry*srx + crz*crx;
  r32 d6 = srz*sry*crx - crz*srx;
  r32 d8 = -sry;
  r32 d9 = cry*srx;
  r32 d10 = cry*crx;
  
  r32 A1223 = d6*tz - d10*ty;
  r32 A0223 = d2*tz - d10*tx;
  r32 A0123 = d2*ty - d6*tx;
  r32 A1213 = d5*tz - d9*ty;
  r32 A1212 = d5*d10 - d9*d6;
  r32 A0213 = d1*tz - d9*tx;
  r32 A0212 = d1*d10 - d9*d2;
  r32 A0113 = d1*ty - d5*tx;
  r32 A0112 = d1*d6 - d5*d2;

  r32 det = d0*(d5*d10 - d9*d6) - d4*(d1*d10 - d9*d2) + d8*(d1*d6 - d5*d2);
  det = 1 / det;

  (*m)[0] = det*(d5*d10 - d9*d6);
  (*m)[4] = -det*(d4*d10 - d8*d6);
  (*m)[8] = det*(d4*d9 - d8*d5);
  (*m)[12] = 0;
  (*m)[1] = -det*(d1*d10 - d9*d2);
  (*m)[5] = det*(d0*d10 - d8*d2);
  (*m)[9] = -det*(d0*d9 - d8*d1);
  (*m)[13] = 0;
  (*m)[2] = det*(d1*d6 - d5*d2);
  (*m)[6] = -det*(d0*d6 - d4*d2);
  (*m)[10] = det*(d0*d5 - d4*d1);
  (*m)[14] = 0;
  (*m)[3] = -det*(d1*A1223 - d5*A0223 + d9*A0123);
  (*m)[7] = det*(d0*A1223 - d4*A0223 + d8*A0123);
  (*m)[11] = -det*(d0*A1213 - d4*A0213 + d8*A0113);
  (*m)[15] = det*(d0*A1212 - d4*A0212 + d8*A0112);
}

r32 clampf(r32 val, r32 min, r32 max) {
  if (val < min)
    return min;
  else if (val > max)
    return max;
  return val;
}

typedef struct {
  VECTOR2F move;
  VECTOR2F look;
} PLAYERINPUT;

typedef struct {
  void *memory;
  u32 size;
} FILE;

typedef struct {
  HANDLE handle0;
  HANDLE handle1; // NOTE If with keyboard
  VECTOR3F position;
  VECTOR3F rotation;
  PLAYERINPUT input;
} PLAYER;

#pragma pack(push, 1)
typedef struct {
  VECTOR3F transform;
  VECTOR3F normal;
  VECTOR2F texture;
} VERTEX;

typedef struct {
  u32 vertexcount;
  u32 facecount;
} PLYHEADER;

typedef struct {
  u32 width;
  u32 height;
} BMPHEADER;
#pragma pack(pop)

typedef struct {
  PLYHEADER *header;
  r32 *vertices;
  u32 *faces;
} PLY;

typedef struct {
  BMPHEADER *header;
  u32 *pixels;
} BMP;

typedef u16 WSTR;

typedef struct {
  u32 facecount;
  ID3D11Buffer *vertexbuffer;
  ID3D11Buffer *indexbuffer;
} MODEL;

#pragma pack(push, 4)
typedef struct {
  MATRIX4F transform;
  MATRIX4F view;
  MATRIX4F camera;
} CONSTANTBUFFER0DATA;
#pragma pack(pop)

typedef struct {
  CONSTANTBUFFER0DATA data;
  ID3D11Buffer *buffer;
} CONSTANTBUFFER0;

typedef struct {
  MODEL *model;
  VECTOR3F position;
  VECTOR3F rotation;
} WORLDOBJECT;

#define MAX_WORLDOBJECTS 64

typedef struct {
  ID3D11Texture2D *texture;
  ID3D11ShaderResourceView *resourceview;
} TEXTURE;

typedef struct {
  ID3D11VertexShader *vertexshader;
  ID3D11PixelShader *pixelshader;
  ID3D11InputLayout *inputlayout;
} SHADER;

#pragma pack(push, 1)
typedef struct {
  u8 identifier0;
  u8 motorenabled;
  u8 unknown[2];
  u8 rumbleright;
  u8 rumbleleft;
  u8 red;
  u8 green;
  u8 blue;
  u8 unknown2[23];
} PS4CONTROLLEROUT;

#define PS4_FORMAT {0x05, 0xFF, 0x00, 0x00};

typedef struct {
  u8 identifier0;
  u8 identifier1;
  u8 unknown0;
  u8 motorenabled;
  u8 unknown1[2];
  u8 rumbleright;
  u8 rumbleleft;
  u8 red;
  u8 green;
  u8 blue;
  u8 unknown2[68];
} PS4CONTROLLERBTOUT;

#define PS4BT_FORMAT {0x11, 0x80, 0x00, 0xFF, 0x00, 0x00};

#define PS4_BTN_TRIANGLE 0x80
#define PS4_BTN_CIRCLE 0x40
#define PS4_BTN_CROSS 0x20
#define PS4_BTN_SQUARE 0x10
#define PS4_BTN_R3 0x8000
#define PS4_BTN_L3 0x4000
#define PS4_BTN_OPTIONS 0x2000
#define PS4_BTN_SHARE 0x1000
#define PS4_BTN_R2 0x800
#define PS4_BTN_L2 0x400
#define PS4_BTN_R1 0x200
#define PS4_BTN_L1 0x100

#define PS4_PID 0x5C4
#define PS4_SLIM_PID 0x9CC

typedef struct {
  u8 reportid;
  u8 lx;
  u8 ly;
  u8 rx;
  u8 ry;
  u16 buttons;
  u8 counter;
  u8 l2;
  u8 r2;
  u16 unknown0;
  u8 battery;
  u16 gyrox;
  u16 gyroy;
  u16 gyroz;
  u16 accelx;
  u16 accely;
  u16 accelz;
} PS4CONTROLLERIN;

#define WII_BTN_LEFT 0x1
#define WII_BTN_RIGHT 0x2
#define WII_BTN_DOWN 0x4
#define WII_BTN_UP 0x8
#define WII_BTN_PLUS 0x10
#define WII_BTN_TWO 0x100
#define WII_BTN_ONE 0x200
#define WII_BTN_B 0x400
#define WII_BTN_A 0x800
#define WII_BTN_MINUS 0x1000
#define WII_BTN_HOME 0x8000

#define WII_PID 0x306
#define WII_PLUS_PID 0x330

typedef struct {
  u8 reportid;
  u16 buttons;
  u8 accelx;
  u8 accely;
  u8 accelz;
} WIICONTROLLERIN;
#pragma pack(pop)

#define SWITCH_PID 0x2006

#define SWITCH_BATTERY_EMPTY 0x0
#define SWITCH_BATTERY_CRITICAL 0x2
#define SWITCH_BATTERY_LOW 0x4
#define SWITCH_BATTERY_MEDIUM 0x6
#define SWITCH_BATTERY_FULL 0x8

#define SWITCH_BATTERY 0xF0
#define SWITCH_CONNECTION 0x0F

typedef struct {
  u8 reportid;
  u8 timer;
  u8 batteryconnection;
  u8 buttonsright;
  u8 buttonsshared;
  u8 buttonsleft;
  u8 lstick[3];
  u8 rstick[3];
  u8 vibrator;
  u8 axes[35];
} SWITCHCONTROLLERIN;

typedef struct { // TODO finish this concept
  u8 playerid;
  u8 rumble;
} CONTROLLEROUT;

typedef struct {
  IDXGISwapChain *swapchain;
  ID3D11Device *device;
  ID3D11DeviceContext *context;
  ID3D11Texture2D *backbuffer;
  ID3D11RenderTargetView *rendertargetview;
  ID3D11DepthStencilView *depthstencilview;
  ID3D11SamplerState *samplerstate;

  SHADER shader;
  MODEL trunkmodel, cubemodel;

  b32 capturemouse;
  b32 debugmode;
  CONSTANTBUFFER0 cbuffer0;

  WORLDOBJECT worldobject[MAX_WORLDOBJECTS];
  PLAYER players[4];
} GAME;

IID SIID_ID3D11Texture2D = {1863690994, 53768, 20105, {154, 180, 72, 149, 53, 211, 79, 156}};

FILE LoadFile(WSTR *filename) {
  FILE result = {0};
  HANDLE filehandle = CreateFileW((LPCWSTR)filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if (filehandle == INVALID_HANDLE_VALUE)
    return result;

  result.size = GetFileSize(filehandle, 0);
  result.memory = VirtualAlloc(0, result.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  DWORD read;
  ReadFile(filehandle, result.memory, result.size, &read, 0);
  CloseHandle(filehandle);
  return result;
}

PLY LoadPLY(WSTR *filename) {
  PLY result = {0};
  FILE file = LoadFile(filename);
  if (file.memory == 0)
    return result;

  result.header = (PLYHEADER *)file.memory;
  result.vertices = (r32 *)file.memory + (sizeof(PLYHEADER) / 4);
  result.faces = (u32 *)file.memory + (sizeof(PLYHEADER) / 4) + (result.header->vertexcount * 8);
  return result;
}

BMP LoadBMP(WSTR *filename) {
  BMP result = {0};
  FILE file = LoadFile(filename);
  if (file.memory == 0)
    return result;

  result.header = (BMPHEADER *)file.memory;
  result.pixels = (u32 *)file.memory + (sizeof(BMPHEADER) / 4);
  return result;
}

LRESULT AppWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_CREATE: {
    SetWindowLongPtrW(window, 0, (LONG_PTR)(((CREATESTRUCT *)lParam)->lpCreateParams));
    return 1;
  } break;
  
  case WM_DESTROY:
  case WM_CLOSE: {
    ExitProcess(0);
  } break;

  case WM_LBUTTONDOWN: {
    GAME *game = (GAME *)GetWindowLongPtrW(window, 0);

    RECT windowpos;
    GetClientRect(window, &windowpos);
    ClientToScreen(window, (LPPOINT)&windowpos.left);
    ClientToScreen(window, (LPPOINT)&windowpos.right);

    if (!game->capturemouse) {
      game->capturemouse = 1;
      ShowCursor(0);
      ClipCursor(&windowpos);
    }
  } break;

  case WM_INPUT_DEVICE_CHANGE: {
    HANDLE handle = (HANDLE)lParam;
    GAME *game = (GAME *)GetWindowLongPtrW(window, 0);

    RID_DEVICE_INFO info;
    u32 infosize = sizeof(RID_DEVICE_INFO);
    GetRawInputDeviceInfoW(handle, RIDI_DEVICEINFO, &info, &infosize);

    if (wParam != GIDC_REMOVAL)
      break;

    PLAYER *player = &game->players[SizeofArray(game->players)];
    while (player != game->players) {
      player--;

      if (player->handle0 == handle || player->handle1 == handle) {
        *player = (PLAYER){0};
      }
    }
  } break;

  case WM_INPUT: {
    GAME *game = (GAME *)GetWindowLongPtrW(window, 0);

    if (wParam != RIM_INPUTSINK) {
      u32 size = 0;
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &size, sizeof(RAWINPUTHEADER));
      RAWINPUT *input = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); // TODO i don't like this VirtualAlloc
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));

      RID_DEVICE_INFO info;
      u32 infosize = sizeof(RID_DEVICE_INFO);
      GetRawInputDeviceInfoW(input->header.hDevice, RIDI_DEVICEINFO, &info, &infosize);


      PLAYER *player = 0;
      for (u32 i = 0; i < SizeofArray(game->players); i++) {
        if (game->players[i].handle0 == input->header.hDevice || game->players[i].handle1 == input->header.hDevice) {
          player = &game->players[i];
          break;
        }
      }

      if (player == 0) {
        if (info.dwType == RIM_TYPEMOUSE || info.dwType == RIM_TYPEKEYBOARD) {
          for (u32 i = 0; i < SizeofArray(game->players); i++) {
            RID_DEVICE_INFO info1;
            u32 info1size = sizeof(RID_DEVICE_INFO);
            GetRawInputDeviceInfoW(game->players[i].handle0, RIDI_DEVICEINFO, &info1, &info1size);
            if ((info1.dwType == RIM_TYPEMOUSE && info.dwType == RIM_TYPEKEYBOARD)
                || (info1.dwType == RIM_TYPEKEYBOARD && info.dwType == RIM_TYPEMOUSE)) {
              game->players[i].handle1 = input->header.hDevice;
              player = &game->players[i];
              break;
            }
          }
        }
      }

      if (player == 0) {
        for (u32 i = 0; i < SizeofArray(game->players); i++) {
          if (game->players[i].handle0 == 0) {
            game->players[i].handle0 = input->header.hDevice;
            player = &game->players[i];
            break;
          }
        }
      }

      switch (input->header.dwType) {
      case RIM_TYPEMOUSE: {
        RECT windowpos;
        POINT cursorpos;
        GetClientRect(window, &windowpos);
        ClientToScreen(window, (LPPOINT)&windowpos.left);
        ClientToScreen(window, (LPPOINT)&windowpos.right);
        GetCursorPos(&cursorpos);

        if (cursorpos.x < windowpos.left || cursorpos.x > windowpos.right || cursorpos.y < windowpos.top || cursorpos.y > windowpos.bottom)
          break;

        if (game->capturemouse) {
          player->input.look.x = (r32)input->data.mouse.lLastX;
          player->input.look.y = (r32)input->data.mouse.lLastY;
        }
      } break;

      case RIM_TYPEKEYBOARD: {
        u16 key = input->data.keyboard.VKey;
        b32 down = !(input->data.keyboard.Flags & RI_KEY_BREAK);
        if (!game->capturemouse)
          down = 0;

        switch (key) {
        case 'W': {
          player->input.move.y = (r32)down;
        } break;

        case 'S': {
          player->input.move.y = -(r32)down;
        } break;
        
        case 'A': {
          player->input.move.x = -(r32)down;
        } break;

        case 'D': {
          player->input.move.x = (r32)down;
        } break;

        case VK_ESCAPE: {
          if (game->capturemouse) {
            game->capturemouse = 0;
            ShowCursor(1);
            ClipCursor(0);
          }
        } break;

        case VK_F1: {
          if (down) {
            game->debugmode = !game->debugmode;
            if (game->debugmode)
              OutputDebugString("Enabled debug mode\n");
            else
              OutputDebugString("Disabled debug mode\n");
          }
        } break;
        }
      } break;

      case RIM_TYPEHID: {
        if (!game->capturemouse)
          break;

        WSTR filename[4048];
        u32 filenamesize = sizeof(filename);
        GetRawInputDeviceInfoW(input->header.hDevice, RIDI_DEVICENAME, filename, &filenamesize);
        HANDLE file = CreateFileW(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (file == INVALID_HANDLE_VALUE)
          break;

        switch (info.hid.dwProductId) {
        case WII_PID:
        case WII_PLUS_PID: {
          WIICONTROLLERIN *wiiin = (WIICONTROLLERIN *)input->data.hid.bRawData;
          if (wiiin->reportid != 0x31) {
      
            u8 buf[3];
            buf[0] = 0x12;
            buf[1] = 0x04;
            buf[2] = 0x31;

            DWORD written;
            WriteFile(file, buf, sizeof(buf), &written, 0);
            break;
          }

          if (wiiin->buttons & WII_BTN_UP)
            player->input.move.y = 1;
          else if (wiiin->buttons & WII_BTN_DOWN)
            player->input.move.y = -1;
          else
            player->input.move.y = 0;

          if (wiiin->buttons & WII_BTN_RIGHT)
            player->input.move.x = 1;
          else if (wiiin->buttons & WII_BTN_LEFT)
            player->input.move.x = -1;
          else
            player->input.move.x = 0;

          player->input.look.x = (wiiin->accelx / 128.0f) - 1;
          player->input.look.y = (wiiin->accelz / 128.0f) - 1;
        } break;

        case PS4_PID:
        case PS4_SLIM_PID: {
          PS4CONTROLLERIN *ps4in = (PS4CONTROLLERIN *)input->data.hid.bRawData;
          
          player->input.move.x = (ps4in->lx / 128.0f) - 1;
          player->input.move.y = 1 - (ps4in->ly / 128.0f);
          player->input.look.x = (ps4in->rx / 128.0f) - 1;
          player->input.look.y = (ps4in->ry / 128.0f) - 1;
        
          if (ps4in->buttons & PS4_BTN_CROSS) {
            b32 bluetooth = size == 96 ? 0 : 1;
            if (bluetooth) {
              PS4CONTROLLERBTOUT out = PS4BT_FORMAT;
              out.rumbleright = 255;
              out.rumbleleft = 255;
              out.red = 255;
              DWORD written;
              WriteFile(file, &out, sizeof(out), &written, 0);
              break;
            } else {
              PS4CONTROLLEROUT out = PS4_FORMAT;
              out.rumbleright = 255;
              out.rumbleleft = 255;
              out.red = 255;
              DWORD written;
              WriteFile(file, &out, sizeof(out), &written, 0);
              break;
            }
          }
        } break;

        case SWITCH_PID: { // TODO kinda struggling with horzintal and vertical values so i never finished this
          SWITCHCONTROLLERIN *switchin = (SWITCHCONTROLLERIN *)input->data.hid.bRawData;
          if (switchin->reportid != 0x30) {
            u8 buf[0x40] = {0};
            buf[0] = 0x01;
            buf[1] = 0;
            buf[10] = 0x03;
            buf[11] = 0x30;
            DWORD written;
            WriteFile(file, buf, sizeof(buf), &written, 0);
            break;
          }

          u16 horizontal = (switchin->lstick[0] | ((switchin->lstick[1] & 0xF) << 8));
          u16 vertical = ((switchin->lstick[1] >> 4) | (switchin->lstick[2] << 4));
          
          WSTR buf[512];
          wsprintfW(buf, L"x: %u y: %u\n", (u32)horizontal, (u32)vertical);

          OutputDebugStringW(buf);

          player->input.move.x = (r32)horizontal;
          player->input.move.y = (r32)vertical;
        } break;
        }
        CloseHandle(file);
      } break;
      }
      VirtualFree(input, 0, MEM_RELEASE);
    }

    DefWindowProcW(window, message, wParam, lParam);
  } break;

  default: {
    return DefWindowProcW(window, message, wParam, lParam);
  } break;
  }
  return 0;
}

void DXInit(GAME *game, HWND window) {
  u32 width = 1920; // TODO change hardcoded value?
  u32 height = 1080;

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
  game->context->lpVtbl->IASetPrimitiveTopology(game->context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  D3D11_VIEWPORT vp = {0};
  vp.Width = (r32)width;
  vp.Height = (r32)height;
  vp.MaxDepth = 1;
  game->context->lpVtbl->RSSetViewports(game->context, 1, &vp);
}

MODEL CreateVertexBufferFromPLY(GAME *game, PLY plymodel) {
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

TEXTURE DXCreateTexture(GAME *game, LPWSTR filename) {
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

SHADER DXCreateShader(GAME *game, LPWSTR vfilename, LPWSTR pfilename) {
  SHADER result = {0};
  FILE vshaderfile = LoadFile(vfilename);
  FILE pshaderfile = LoadFile(pfilename);

  game->device->lpVtbl->CreateVertexShader(game->device, vshaderfile.memory, vshaderfile.size, 0, &result.vertexshader);
  game->device->lpVtbl->CreatePixelShader(game->device, pshaderfile.memory, pshaderfile.size, 0, &result.pixelshader);

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
  game->device->lpVtbl->CreateInputLayout(game->device, ied, SizeofArray(ied), vshaderfile.memory, vshaderfile.size, &result.inputlayout);

  return result;
}

void GameInit(GAME *game) {
  game->shader = DXCreateShader(game, L"res\\shaderv.cso", L"res\\shaderp.cso");
  game->context->lpVtbl->IASetInputLayout(game->context, game->shader.inputlayout);

  game->trunkmodel = CreateVertexBufferFromPLY(game, LoadPLY(L"res\\trunk.ply"));
  game->cubemodel = CreateVertexBufferFromPLY(game, LoadPLY(L"res\\cube.ply"));

  D3D11_BUFFER_DESC bd = {0};
  bd.ByteWidth = sizeof(game->cbuffer0.data);
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  game->device->lpVtbl->CreateBuffer(game->device, &bd, 0, &game->cbuffer0.buffer);
  game->context->lpVtbl->VSSetConstantBuffers(game->context, 0, 1, &game->cbuffer0.buffer);

  CreatePerspectiveViewMatrix(&game->cbuffer0.data.view, 16.0f / 9.0f, DegreesToRadians(60.0f), 0.1f, 100.0f);

  D3D11_SAMPLER_DESC sd = {0};
  sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  game->device->lpVtbl->CreateSamplerState(game->device, &sd, &game->samplerstate);

  game->context->lpVtbl->PSSetSamplers(game->context, 0, 1, &game->samplerstate);

  TEXTURE texture = DXCreateTexture(game, L"res\\bark.bmp");
  game->context->lpVtbl->PSSetShaderResources(game->context, 0, 1, &texture.resourceview);
  
  game->worldobject[0].model = &game->trunkmodel;
}


void DXUpdateResource(GAME *game, ID3D11Resource *resource, void *data, u32 size) {
  D3D11_MAPPED_SUBRESOURCE ms;
  game->context->lpVtbl->Map(game->context, resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
  memcpy(ms.pData, data, size);
  game->context->lpVtbl->Unmap(game->context, resource, 0);
}

void RenderWorldObject(GAME *game, WORLDOBJECT *obj) {
  CreateTransformMatrix(&game->cbuffer0.data.transform, obj->position.x, obj->position.y, obj->position.z, obj->rotation.x, obj->rotation.y, obj->rotation.z);

  UINT stride = sizeof(VERTEX);
  UINT offset = 0;
  game->context->lpVtbl->IASetVertexBuffers(game->context, 0, 1, &obj->model->vertexbuffer, &stride, &offset);
  game->context->lpVtbl->IASetIndexBuffer(game->context, obj->model->indexbuffer, DXGI_FORMAT_R32_UINT, 0);

  DXUpdateResource(game, (ID3D11Resource *)game->cbuffer0.buffer, &game->cbuffer0.data, sizeof(game->cbuffer0.data));

  game->context->lpVtbl->DrawIndexed(game->context, obj->model->facecount * 3, 0, 0);
}

void GameUpdate(GAME *game) {
  CreateInverseTransformMatrix(&game->cbuffer0.data.camera, 0, 20, -5, DegreesToRadians(70.0f), 0, 0);

  game->context->lpVtbl->ClearRenderTargetView(game->context, game->rendertargetview, (float[4]) {1, 1, 1, 1});
  game->context->lpVtbl->ClearDepthStencilView(game->context, game->depthstencilview, D3D11_CLEAR_DEPTH, 1.0f, 0);

  game->context->lpVtbl->VSSetShader(game->context, game->shader.vertexshader, 0, 0);
  game->context->lpVtbl->PSSetShader(game->context, game->shader.pixelshader, 0, 0);

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
    
    if (player->input.move.x < 0 && player->input.move.y == 0)
      player->rotation.y = DegreesToRadians(180.0f);
    else if (player->input.move.x > 0 || player->input.move.y != 0)
      player->rotation.y = DegreesToRadians(360.0f) - 2 * atan(player->input.move.y / (sqrtf(player->input.move.x * player->input.move.x + player->input.move.y * player->input.move.y) + player->input.move.x));

    WORLDOBJECT playerobj = {0};
    playerobj.model = &game->cubemodel;
    playerobj.position = player->position;
    playerobj.rotation = player->rotation;
    playerobj.rotation.x = 0;
    playerobj.rotation.z = 0;
    RenderWorldObject(game, &playerobj);

    player->input.look = (VECTOR2F){0};
  }

  game->swapchain->lpVtbl->Present(game->swapchain, 1, 0);
}

void MainEntry(void) {
  HINSTANCE instance = GetModuleHandle(0);

  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = AppWindowProc;
  wc.cbWndExtra = sizeof(GAME);
  wc.hInstance = instance;
  wc.hCursor = LoadCursorA(0, IDC_ARROW);
  wc.lpszClassName = L"slinapp21/06/21";

  if (!RegisterClassExW(&wc))
    ExitProcess(1);

  GAME game = {0};

  HWND window = CreateWindowExW(0, wc.lpszClassName, L"Slin App", WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, instance, &game);

  DXInit(&game, window);

  RAWINPUTDEVICE rid[4] = {0};
  rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
  rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
  rid[0].hwndTarget = window;
  rid[0].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
  rid[1] = rid[0];
  rid[1].usUsage = HID_USAGE_GENERIC_MOUSE;
  rid[2] = rid[0];
  rid[2].usUsage = HID_USAGE_GENERIC_GAMEPAD;
  rid[3] = rid[2];
  rid[3].usUsage = HID_USAGE_GENERIC_JOYSTICK;
  RegisterRawInputDevices(rid, SizeofArray(rid), sizeof(RAWINPUTDEVICE));

  u64 frequency;
  u64 lastcount;

  timeBeginPeriod(1);

  QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
  QueryPerformanceCounter((LARGE_INTEGER *)&lastcount);

  GameInit(&game);

  while (1) {
    MSG message;
    while (PeekMessageW(&message, window, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
 
    GameUpdate(&game);

    u64 nowcount;
    QueryPerformanceCounter((LARGE_INTEGER *)&nowcount);
    u64 delta = (nowcount - lastcount) / frequency;

    u64 remaining = (1000 / 60) - delta;
    if (remaining > 0)
      Sleep((DWORD)remaining);
    
    lastcount = nowcount;
  }
}

// TODO wtf is this
void _fltused(void) {}
