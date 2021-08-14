#pragma pack(push, 1)
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
