#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>

#include "SDL3/SDL.h"

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool Running = true;

// NOTE: Always 32-bits pixels. SDL_PIXELFORMAT_ABGR8888. Little Endian.
struct sdl_offscreen_buffer {
  SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

struct sdl_window_dimension {
  int Width;
  int Height;
};

global_variable sdl_offscreen_buffer GlobalBackBuffer{};

internal sdl_window_dimension SDLGetWindowDimension(SDL_Window *w) { // ignore
  sdl_window_dimension r;
  SDL_GetWindowSize(w, &r.Width, &r.Height);
  return r;
}

internal void RenderWeirdGradiant(sdl_offscreen_buffer Buffer, int x_offset,
                                  int y_offset) {

  uint8 *Row = (uint8 *)Buffer.Memory;
  for (int Y = 0; Y < Buffer.Height; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer.Width; ++X) {

      /*
       * Pixel in memory:	RR	GG	BB	padding
       * LITTLE ENDIAN ARCHITECTURE
       * 0x000000
       */
      uint8 red = (X + x_offset);
      uint8 green = (Y + y_offset);
      uint8 blue = ((x_offset + y_offset) * 100) / 500;
      uint8 opacity = 255;

      *Pixel++ = (opacity << 24) | (blue << 16) | (green << 8) | red;
    }
    Row += Buffer.Pitch;
  }
}

// TODO: This will be filed

internal void SDLResizeTextureBuffer(sdl_offscreen_buffer *Buffer,
                                     SDL_Renderer *r, int width, int height) {
  int BitsPerPixel = 4;

  if (Buffer->Memory) {
    int ok = munmap(Buffer->Memory,
                    ((Buffer->Width * Buffer->Height) * BitsPerPixel));
    std::cerr << "Its ok?: " << ok << "\n";
  }
  if (Buffer->Texture) {
    SDL_DestroyTexture(Buffer->Texture);
  }

  Buffer->Texture = SDL_CreateTexture(
      r, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);

  Buffer->Width = width;
  Buffer->Height = height;

  Buffer->Memory =
      mmap(0, (Buffer->Width * Buffer->Height) * BitsPerPixel,
           PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  Buffer->Pitch = Buffer->Width * BitsPerPixel;
}

internal void SDLDisplayBufferWindow(SDL_Renderer *r,
                                     sdl_offscreen_buffer Buffer) {
  SDL_UpdateTexture(Buffer.Texture, 0, Buffer.Memory, Buffer.Width * 4);
  SDL_RenderTexture(r, Buffer.Texture, 0, 0);
  SDL_RenderPresent(r);
}

bool HandleEvent(SDL_Event *Event) {
  bool ShouldQuit = false;

  switch (Event->type) {
  case SDL_EVENT_WINDOW_RESIZED: {
    SDL_Window *w = SDL_GetWindowFromID(Event->window.windowID);
    SDL_Renderer *r = SDL_GetRenderer(w);
    sdl_window_dimension window_size = SDLGetWindowDimension(w);

    SDLResizeTextureBuffer(&GlobalBackBuffer, r, window_size.Width,
                           window_size.Height);
    SDLDisplayBufferWindow(r, GlobalBackBuffer);

  } break;
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
    ShouldQuit = true;
  } break;

  case SDL_EVENT_WINDOW_EXPOSED: {
    SDL_Window *w = SDL_GetWindowFromID(Event->window.windowID);
    SDL_Renderer *r = SDL_GetRenderer(w);
    SDLDisplayBufferWindow(r, GlobalBackBuffer);
  } break;
  }

  return (ShouldQuit);
}

int main() {
  if (!SDL_Init(0)) {
    std::cerr << "Error initializing the VIDEO DRIVER\n";
  };

  SDL_Window *Window = SDL_CreateWindow(
      "", 1280, 720,
      SDL_WINDOW_RESIZABLE); // NOTE: SDL_WINDOW_RESIZABLE to stick
  if (!Window) {
    std::cerr << "Failed to create Window\n";
  }
  SDL_Renderer *r = SDL_CreateRenderer(Window, NULL);
  if (!r) {
    std::cerr << "Failed to create Renderer\n";
  }
  SDLResizeTextureBuffer(&GlobalBackBuffer, r, 1280, 720);

  while (Running) {
    SDL_Event registeredEvent;
    while (SDL_PollEvent(&registeredEvent)) {
      if (HandleEvent(&registeredEvent)) {
        Running = false;
      }
    }
    const bool *KeyStates = SDL_GetKeyboardState(NULL);

    local_persist int x_offset = 0;
    local_persist int y_offset = 0;

    if (KeyStates[SDL_SCANCODE_W]) {
      y_offset -= 1;
    }
    if (KeyStates[SDL_SCANCODE_S]) {
      y_offset += 1;
    }
    if (KeyStates[SDL_SCANCODE_A]) {
      x_offset -= 1;
    }
    if (KeyStates[SDL_SCANCODE_D]) {
      x_offset += 1;
    }

    RenderWeirdGradiant(GlobalBackBuffer, x_offset, y_offset);
    SDLDisplayBufferWindow(r, GlobalBackBuffer);
  }

  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(Window);
  SDL_Quit();

  return 0;
}
