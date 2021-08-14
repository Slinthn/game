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

typedef u16 WSTR;

IID SIID_ID3D11Texture2D = {1863690994, 53768, 20105, {154, 180, 72, 149, 53, 211, 79, 156}};

#define APP_WIDTH 1920
#define APP_HEIGHT 1080

#include "win64_math.c"

#define SizeofArray(x) (sizeof(x) / (sizeof((x)[0])))

typedef struct {
  VECTOR2F move;
  VECTOR2F look;
} PLAYERINPUT;

typedef struct {
  void *memory;
  u32 size;
} FILE;

typedef struct {
  u32 type;
} ITEM;

typedef struct {
  ITEM items[4];
  u32 selectedslot;
} INVENTORY;

typedef struct {
  HANDLE handle0;
  HANDLE handle1; // NOTE If with keyboard
  b32 bluetooth;
  u8 playerid;
  VECTOR3F position;
  r32 rotating;
  r32 rotatingprev;
  r32 rotatetime;
  VECTOR3F rotation;
  PLAYERINPUT input;
} PLAYER;

#pragma pack(push, 1)
typedef struct {
  VECTOR3F transform;
  VECTOR3F normal;
  VECTOR2F texture;
} VERTEX;
#pragma pack(pop)

#include "win64_loader.c"

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

typedef struct {
  IDXGISwapChain *swapchain;
  ID3D11Device *device;
  ID3D11DeviceContext *context;
  ID3D11Texture2D *backbuffer;
  ID3D11RenderTargetView *rendertargetview;
  ID3D11DepthStencilView *depthstencilview;
  ID3D11SamplerState *samplerstate;
  ID3D11Buffer *guivertexbuffer;

  u32 guipixels[APP_WIDTH * APP_HEIGHT];
  TEXTURE texture, guitexture;
  SHADER shader, guishader;
  MODEL trunkmodel, cubemodel;

  b32 capturemouse;
  b32 debugmode;
  CONSTANTBUFFER0 cbuffer0;

  WORLDOBJECT worldobject[MAX_WORLDOBJECTS];
  PLAYER players[4];
} GAME;

#include "win64_rawinput.c"
#include "win64_dx11.c"

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

      RIProcessInput(game, window, input, size);

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

#include "win64_game.c"

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

  GAME *game = VirtualAlloc(0, 1024*1024*1024, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  HWND window = CreateWindowExW(0, wc.lpszClassName, L"Slin App", WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, instance, game);

  DXInit(game, window);

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

  GameInit(game);

  while (1) {
    MSG message;
    while (PeekMessageW(&message, window, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
 
    GameUpdate(game);

    u64 nowcount;
    QueryPerformanceCounter((LARGE_INTEGER *)&nowcount);
    u64 delta = (nowcount - lastcount) * 1000 / frequency;

    s64 remaining = (1000 / 60) - delta;

    WSTR buf[128];
    wsprintfW(buf, L"Time taken: %u\n", delta);

    OutputDebugStringW(buf);

    if (remaining > 0)
      Sleep((DWORD)remaining);
    
    QueryPerformanceCounter((LARGE_INTEGER *)&lastcount);
  }
}

// TODO wtf is this
void _fltused(void) {}
