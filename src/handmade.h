#if !defined(HANDMADE_H)
#define HANDMADE_H

#define Assert(Expression)                                                     \
  if (!(Expression)) {                                                         \
    *(int *)0 = 0;                                                             \
  }
#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)

#include "SDL3/SDL.h"
#include <cstdint> // Needed for standard integer types like uint8_t

typedef uint8_t uint8;
typedef int16_t int16;
typedef int32_t int32;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef char32_t bool32;

typedef float real32;
typedef double real64;

//
// NOTE: Structs
// This is the game's abstract representation of the buffer.
//
struct game_offscreen_buffer {
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

struct game_sound_output_buffer {
  int SamplesPerSecond;
  int SampleCount;
  int16 *Samples;
};

struct game_button_state {
  int halftransitioncount;
  bool32 endeddown;
};

struct game_controller_input {
  bool32 IsAnalog;

  real32 StartX;
  real32 StartY;

  real32 MinX;
  real32 MinY;

  real32 MaxX;
  real32 MaxY;

  real32 EndX;
  real32 EndY;

  union {
    game_button_state Buttons[6];
    struct {
      game_button_state Up;
      game_button_state Down;
      game_button_state Left;
      game_button_state Right;
      game_button_state LeftShoulder;
      game_button_state RightShoulder;
    };
  };
};

struct game_input {
  game_controller_input Controllers[4];
};

struct debug_read_file_result {
  uint32 ContentsSize;
  void *Contents;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name)                                  \
  debug_read_file_result name(const char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name)                                 \
  bool32 name(const char *Filename, uint32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

struct game_memory {
  bool32 IsInitialized;
  uint64 PermanentStorageSpaceSize;
  void *PermanentStorage;
  uint64 TransientStorageSize;
  void *TransientStorage;

  debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
  debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
  debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
};

//
// NOTE: Function Prototypes
// The signature must match the definition in handmade.cpp (which takes
// offsets).
//
//
static void GameUpdateAndRender(game_memory *Memory, game_input *Input,
                                game_offscreen_buffer *Buffer,
                                game_sound_output_buffer *SoundBuffer);

struct loaded_bitmap {
  int32 Width;
  int32 Height;
  uint32 *Pixels;
};

struct game_state {
  int ToneHz;
  int GreenOffset;
  int BlueOffset;
  loaded_bitmap TestBitmap;
};

#pragma pack(push, 1)
struct bitmap_header {
  uint16 FileType;
  uint32 FileSize;
  uint16 Reserved1;
  uint16 Reserved2;
  uint32 BitmapOffSet; // starts of pixel data
  uint32 Size;
  int32 Width;
  int32 Height;
  uint16 Planes;
  uint16 BitCount;    // bits per pixel
  uint32 Compression; // 0, 1, 2 or 3 type;
  uint32 ImageSize;
  int32 HorzResolution;
  int32 VertResolution;
  uint32 ColorsUsed;
  uint32 ColorsImportant;
};
#pragma pack(pop)

#endif
