#pragma pack(push, 1)
typedef struct {
  u8 rumbleright;
  u8 rumbleleft;
  u8 red;
  u8 green;
  u8 blue;
} PS4CONTROLLERCOMMONOUT;

typedef struct {
  u8 identifier0;
  u8 motorenabled;
  u8 unknown[2];
  PS4CONTROLLERCOMMONOUT common;
  u8 unknown2[23];
} PS4CONTROLLEROUT;

#define PS4_FORMAT {0x05, 0xFF, 0x00, 0x00};

typedef struct {
  u8 identifier0;
  u8 identifier1;
  u8 unknown0;
  u8 motorenabled;
  u8 unknown1[2];
  PS4CONTROLLERCOMMONOUT common;
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
#pragma pack(pop)

void RIProcessInput(GAME *game, HWND window, RAWINPUT *input, u32 size) {
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

  if (player->playerid == 0) {
    player->playerid = (u8)(((u64)player - (u64)game->players) / sizeof(PLAYER)) + 1;
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
      player->bluetooth = size == 96 ? 0 : 1; // TODO a bit scuffed to put this here
    } break;

    case SWITCH_PID: { // TODO kinda struggling with horzintal and vertical values so i never finished this - UPDATE it's probably because i didn't put the SWITCHCONTROLLERIN in the pragma pack push 1. fixed this now
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
}

void RIOutput(PLAYER *player, u8 rumble) {
  WSTR filename[4048];
  u32 filenamesize = sizeof(filename);
  GetRawInputDeviceInfoW(player->handle0, RIDI_DEVICENAME, filename, &filenamesize);
  HANDLE file = CreateFileW(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
  if (file == INVALID_HANDLE_VALUE)
    return;
 
  // TODO check controller type. Assuming ps4 for now
  PS4CONTROLLERBTOUT btout = PS4BT_FORMAT;
  PS4CONTROLLEROUT usbout = PS4_FORMAT;
  PS4CONTROLLERCOMMONOUT *common;

  if (player->bluetooth)
    common = &btout.common;
  else
    common = &usbout.common;
  
  common->rumbleright = rumble;
  common->rumbleleft = rumble;

  if (player->playerid == 1)
    common->blue = 255;
  else if (player->playerid == 2)
    common->red = 255;
  else if (player->playerid == 3)
    common->green = 255;
  else if (player->playerid == 4) {
    common->red = 255;
    common->blue = 100;
  } else {
    common->red = 255;
    common->green = 255;
    common->blue = 255;
  }

  DWORD written;
  if (player->bluetooth)
    WriteFile(file, &btout, sizeof(PS4CONTROLLERBTOUT), &written, 0);
  else
    WriteFile(file, &usbout, sizeof(PS4CONTROLLEROUT), &written, 0);
}
