#if !defined(HANDMADE_H)
#define HANDMADE_H

#include "SDL3/SDL.h"
#include <cstdint> // Needed for standard integer types like uint8_t

typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

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

//
// NOTE: Function Prototypes
// The signature must match the definition in handmade.cpp (which takes
// offsets).
//
//
static void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset,
                                int YOffset,
                                game_sound_output_buffer *SoundBuffer,
                                int ToneHz);

#endif
