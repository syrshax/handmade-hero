#include "handmade.h"
#include <cmath>
#include <iostream>

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

static void GameUpdateAndRender(game_input *Input,
                                game_offscreen_buffer *Buffer,
                                game_sound_output_buffer *SoundBuffer) {
  static int blueOffset = 0;
  static int greenOffset = 0;
  static int toneHz = 220;

  game_controller_input *Input0 = &Input->Controllers[0];
  if (Input0->IsAnalog) {
    blueOffset += (int)4.0f * (Input0->EndY);
  } else {
  }

  GameOutputSound(SoundBuffer, toneHz);
  RenderWeirdGradient(Buffer, greenOffset, blueOffset);
}
