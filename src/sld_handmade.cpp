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
global_variable SDL_Texture *Texture;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BitsPerPixel = 4;

internal void RenderWeirdGradiant(int x_offset, int y_offset) {
  int Pitch = BitmapWidth * BitsPerPixel;
  uint8 *Row = (uint8 *)BitmapMemory;
  for (int Y = 0; Y < BitmapHeight; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < BitmapWidth; ++X) {

      /*
       * Pixel in memory:	RR	GG	BB	padding
       * LITTLE ENDIAN ARCHITECTURE
       * 0x000000
       */
      uint8 red = (X + x_offset);
      uint8 green = (Y + y_offset);
      uint8 blue = 100;
      uint8 opacity = 255;

      *Pixel++ = (opacity << 24) | (blue << 16) | (green << 8) | red;
    }
    Row += Pitch;
  }
}

// TODO: This will be filed

internal void SDLResizeTextureBuffer(SDL_Renderer *r, int width, int height) {
  if (BitmapMemory) {
    int ok =
        munmap(BitmapMemory, ((BitmapWidth * BitmapHeight) * BitsPerPixel));
    std::cerr << "Its ok?: " << ok << "\n";
  }
  if (Texture) {
    SDL_DestroyTexture(Texture);
  }

  Texture = SDL_CreateTexture(r, SDL_PIXELFORMAT_ABGR8888,
                              SDL_TEXTUREACCESS_STREAMING, width, height);

  BitmapWidth = width;
  BitmapHeight = height;
  BitmapMemory =
      mmap(0, (BitmapWidth * BitmapHeight) * BitsPerPixel,
           PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

internal void SDLUpdateWindow(SDL_Renderer *r) {
  SDL_UpdateTexture(Texture, 0, BitmapMemory, BitmapWidth * 4);
  SDL_RenderTexture(r, Texture, 0, 0);
  SDL_RenderPresent(r);
}

bool HandleEvent(SDL_Event *Event) {
  bool ShouldQuit = false;

  switch (Event->type) {
  case SDL_EVENT_WINDOW_RESIZED: {
    SDL_Window *w = SDL_GetWindowFromID(Event->window.windowID);
    SDL_Renderer *r = SDL_GetRenderer(w);
    SDLResizeTextureBuffer(r, Event->window.data1, Event->window.data2);
    SDLUpdateWindow(r);

  } break;
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
    ShouldQuit = true;
  } break;

  case SDL_EVENT_WINDOW_EXPOSED: {
    SDL_Window *w = SDL_GetWindowFromID(Event->window.windowID);
    SDL_Renderer *r = SDL_GetRenderer(w);
    SDLUpdateWindow(r);
  } break;
  }

  return (ShouldQuit);
}

int main() {
  if (!SDL_Init(0)) {
    std::cerr << "Error initializing the VIDEO DRIVER\n";
  };

  SDL_Window *Window = SDL_CreateWindow("", 900, 900, SDL_WINDOW_RESIZABLE);
  if (!Window) {
    std::cerr << "Failed to create Window\n";
  }
  SDL_Renderer *r = SDL_CreateRenderer(Window, NULL);
  if (!r) {
    std::cerr << "Failed to create Renderer\n";
  }
  SDLResizeTextureBuffer(r, 900, 900);

  int x_offset = 0;
  int y_offset = 0;

  while (Running) {
    SDL_Event registeredEvent;
    while (SDL_PollEvent(&registeredEvent)) {
      if (HandleEvent(&registeredEvent)) {
        Running = false;
      }
    }

    RenderWeirdGradiant(x_offset, y_offset);
    SDLUpdateWindow(r);
    ++x_offset;
    ++y_offset;
  }

  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(Window);
  SDL_Quit();

  return 0;
}
