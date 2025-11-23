#if !defined(HANDMADE_H)
#define HANDMADE_H

#include "SDL3/SDL.h"
#include <cstdint> // Needed for standard integer types like uint8_t

typedef uint8_t uint8;
typedef int16_t int16;
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

//
// NOTE: Function Prototypes
// The signature must match the definition in handmade.cpp (which takes
// offsets).
//
//
static void GameUpdateAndRender(game_offscreen_buffer *Buffer,
                                game_sound_output_buffer *SoundBuffer,
                                game_input *Input);

#endif
