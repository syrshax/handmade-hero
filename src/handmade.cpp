#include "handmade.h"
#include <cmath>

#define Pi32 3.14159265359f

static void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz) {
  static real32 tSine = 0.0f;
  int16 ToneVolume = 3000;
  int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

  int16 *SampleOut = SoundBuffer->Samples;
  for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount;
       ++SampleIndex) {
    real32 SineValue = sinf(tSine);
    int16 SampleValue = (int16)(SineValue * ToneVolume);

    *SampleOut++ = SampleValue; // Left channel
    *SampleOut++ = SampleValue; // Right channel

    tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
  }
}
static void RenderWeirdGradient(game_offscreen_buffer *Buffer, int x_offset,
                                int y_offset) {
  uint8 *Row = (uint8 *)Buffer->Memory;
  for (int Y = 0; Y < Buffer->Height; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer->Width; ++X) {

      /*
       * Pixel in memory:	RR	GG	BB	padding
       * LITTLE ENDIAN ARCHITECTURE
       * 0x000000
       */

      uint8 red = (X + x_offset);
      uint8 green = (Y + y_offset);
      uint8 blue = 50;
      uint8 opacity = 188;

      *Pixel++ = (opacity << 24) | (blue << 16) | (green << 8) | red;
    }
    Row += Buffer->Pitch;
  }
}

static void GameUpdateAndRender(game_memory *Memory, game_input *Input,
                                game_offscreen_buffer *Buffer,
                                game_sound_output_buffer *SoundBuffer) {

  game_state *GameState = (game_state *)Memory->PermanentStorage;
  if (!Memory->IsInitialized) {
    GameState->ToneHz = 220;
    // TODO: Maybe more appropiated to do on platform layer;
    Memory->IsInitialized = true;
  }

  game_controller_input *Input0 = &Input->Controllers[0];
  if (Input0->IsAnalog) {
    GameState->GreenOffset += (int)32.0f * (Input0->EndX);
    GameState->BlueOffset -= (int)32.0f * (Input0->EndY);
  } else {
  }

  GameOutputSound(SoundBuffer, GameState->ToneHz);
  RenderWeirdGradient(Buffer, GameState->GreenOffset, GameState->BlueOffset);
}
