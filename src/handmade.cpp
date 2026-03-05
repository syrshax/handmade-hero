#include "handmade.h"
#include <cmath>
#include <cstdlib>

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

static void DrawRectangle(game_offscreen_buffer *Buffer, real32 RealMinX,
                          real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
                          uint8 Red, uint8 Green, uint8 Blue) {
  int32 MinX = (int32)RealMinX;
  int32 MinY = (int32)RealMinY;
  int32 MaxX = (int32)RealMaxX;
  int32 MaxY = (int32)RealMaxY;

  if (MinX < 0)
    MinX = 0;
  if (MinY < 0)
    MinY = 0;
  if (MaxX > Buffer->Width)
    MaxX = Buffer->Width;
  if (MaxY > Buffer->Height)
    MaxY = Buffer->Height;

  for (int Y = MinY; Y < MaxY; Y++) {
    uint32 *pixel =
        (uint32 *)((uint8 *)Buffer->Memory + Y * Buffer->Pitch) + MinX;
    for (int X = MinX; X < MaxX; X++) {
      *pixel++ = (0xFF << 24) | (Blue << 16) | (Green << 8) | Red;
    }
  }
}

#define TILEMAP_COLS 16
#define TILEMAP_ROWS 9
uint32 TileMap[TILEMAP_ROWS][TILEMAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

static void DrawTileMap(game_offscreen_buffer *Buffer) {
  real32 TileWidth = (real32)Buffer->Width / (real32)TILEMAP_COLS;
  real32 TileHeight = (real32)Buffer->Height / (real32)TILEMAP_ROWS;

  for (int Row = 0; Row < TILEMAP_ROWS; Row++) {
    for (int Col = 0; Col < TILEMAP_COLS; Col++) {
      uint32 TileValue = TileMap[Row][Col];

      real32 MinX = (real32)Col * TileWidth;
      real32 MinY = (real32)Row * TileHeight;
      real32 MaxX = MinX + TileWidth;
      real32 MaxY = MinY + TileHeight;

      if (TileValue == 1) {
        DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, 100, 100, 100);
      } else {
        DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, 30, 30, 30);
      }
    }
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

static loaded_bitmap DEBUGLoadBMP(game_memory *Memory, const char *Filename) {
  loaded_bitmap Result = {};

  debug_read_file_result ReadResult =
      Memory->DEBUGPlatformReadEntireFile(Filename);

  if (!ReadResult.Contents) {
    return Result;
  }

  bitmap_header *Header = (bitmap_header *)ReadResult.Contents;

  if (Header->FileType != 0x4D42) {
    Memory->DEBUGPlatformFreeFileMemory(ReadResult.Contents);
    return Result;
  }

  Result.Width = Header->Width;
  Result.Height = Header->Height;

  bool32 TopDown = false;
  if (Result.Height < 0) {
    Result.Height = -Result.Height;
    TopDown = true;
  }

  uint8 *SourcePixels = (uint8 *)ReadResult.Contents + Header->BitmapOffSet;
  int32 BytesPerPixel = Header->BitCount / 8;
  int32 SourcePitch = ((Result.Width * BytesPerPixel + 3) & ~3);

  Result.Pixels =
      (uint32 *)malloc(Result.Width * Result.Height * sizeof(uint32));

  for (int32 Y = 0; Y < Result.Height; ++Y) {
    int32 SourceY;
    if (TopDown) {
      SourceY = Y;
    } else {
      SourceY = Result.Height - 1 - Y;
    }

    uint8 *SourceRow = SourcePixels + (SourceY * SourcePitch);
    for (int32 X = 0; X < Result.Width; ++X) {

      uint8 Blue = SourceRow[X * BytesPerPixel + 0];
      uint8 Green = SourceRow[X * BytesPerPixel + 1];
      uint8 Red = SourceRow[X * BytesPerPixel + 2];

      Result.Pixels[Y * Result.Width + X] =
          (0xFF << 24) | (Blue << 16) | (Green << 8) | Red;
    }
  }

  Memory->DEBUGPlatformFreeFileMemory(ReadResult.Contents);

  return Result;
}

static void DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap,
                       int32 OffsetX, int32 OffsetY) {
  // Clamp to buffer bounds
  int32 MinX = (OffsetX < 0) ? 0 : OffsetX;
  int32 MinY = (OffsetY < 0) ? 0 : OffsetY;
  int32 MaxX = OffsetX + Bitmap->Width;
  int32 MaxY = OffsetY + Bitmap->Height;
  if (MaxX > Buffer->Width)
    MaxX = Buffer->Width;
  if (MaxY > Buffer->Height)
    MaxY = Buffer->Height;

  for (int32 Y = MinY; Y < MaxY; ++Y) {
    uint32 *DestRow = (uint32 *)((uint8 *)Buffer->Memory + Y * Buffer->Pitch);
    int32 BmpY = Y - OffsetY;
    uint32 *SourceRow = Bitmap->Pixels + BmpY * Bitmap->Width;

    for (int32 X = MinX; X < MaxX; ++X) {
      int32 BmpX = X - OffsetX;
      DestRow[X] = SourceRow[BmpX];
    }
  }
}

static void GameUpdateAndRender(game_memory *Memory, game_input *Input,
                                game_offscreen_buffer *Buffer,
                                game_sound_output_buffer *SoundBuffer) {
  game_state *GameState = (game_state *)Memory->PermanentStorage;
  if (!Memory->IsInitialized) {
    GameState->ToneHz = 220;
    GameState->PlayerX = 100.0f;
    GameState->PlayerY = 100.0f;
    GameState->TestBitmap = DEBUGLoadBMP(Memory, "lena.bmp");
    GameState->TestBitmap2 = DEBUGLoadBMP(Memory, "new_guy.bmp");
    Memory->IsInitialized = true;
  }

  game_controller_input *Input0 = &Input->Controllers[0];
  if (Input0->IsAnalog) {
    GameState->GreenOffset += (int)(32.0f * Input0->EndX);
    GameState->BlueOffset -= (int)(32.0f * Input0->EndY);
    GameState->PlayerX += 4.0f * Input0->EndX;
    GameState->PlayerY -= 4.0f * Input0->EndY;
  }

  if (Input0->Down.endeddown) {
    GameState->PlayerY += 4.0f;
  }
  if (Input0->Up.endeddown) {
    GameState->PlayerY -= 4.0f;
  }
  if (Input0->Left.endeddown) {
    GameState->PlayerX -= 4.0f;
  }
  if (Input0->Right.endeddown) {
    GameState->PlayerX += 4.0f;
  }

  GameOutputSound(SoundBuffer, GameState->ToneHz);
  DrawTileMap(Buffer);
  // DrawBitmap(Buffer, &GameState->TestBitmap, 100, 100);
  // DrawBitmap(Buffer, &GameState->TestBitmap2, 600, 600);

  real32 PlayerWidth = 60.0f;
  real32 PlayerHeight = 80.0f;
  DrawRectangle(Buffer, GameState->PlayerX, GameState->PlayerY,
                GameState->PlayerX + PlayerWidth,
                GameState->PlayerY + PlayerHeight, 254, 20, 50);
}
