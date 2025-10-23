#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "SDL3/SDL.h"

#define internal static
#define local_persist static
#define global_variable static

global_variable SDL_Texture *Texture;
global_variable void *Pixels;
global_variable int TextureWidth;

// TODO: This will be filed

internal void SDLResizeTextureBuffer(SDL_Renderer *r, int height, int width) {
  if (Pixels) {
    free(Pixels);
  }
  if (Texture) {
    SDL_DestroyTexture(Texture);
  }

  Texture = SDL_CreateTexture(r, SDL_PIXELFORMAT_ABGR8888,
                              SDL_TEXTUREACCESS_STREAMING, width, height);

  printf("IM HERE OR NOT");

  TextureWidth = width;
  Pixels = malloc(width * height * 4);
}

internal void SDLUpdateWindow(SDL_Renderer *r) {
  SDL_UpdateTexture(Texture, 0, Pixels, TextureWidth * 4);
  SDL_RenderTexture(
      r, Texture, 0,
      0); // Its like RenderCopy.. : Copy a portion of the texture to the
          // current rendering target at subpixel precision.
  SDL_RenderPresent(r);
}

bool HandleEvent(SDL_Event *Event) {
  bool ShouldQuit = false;

  switch (Event->type) {
  case SDL_EVENT_WINDOW_RESIZED: {

    printf("Window RESIZED!\n Size is kinda nuts!: %d x %d\n",
           Event->window.data1, Event->window.data2);

  } break;
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
    printf("WE OUT!");
    ShouldQuit = true;
  } break;

  case SDL_EVENT_WINDOW_EXPOSED: {
    SDL_Window *w = SDL_GetWindowFromID(Event->window.windowID);
    SDL_Renderer *r = SDL_GetRenderer(w);
    SDLResizeTextureBuffer(r, Event->window.data1, Event->window.data2);
    SDLUpdateWindow(r);
  } break;
  }

  return (ShouldQuit);
}

int main() {
  if (!SDL_Init(0)) {
    std::cerr << "Error initializing the VIDEO DRIVER\n";
  };

  SDL_Window *Window =
      SDL_CreateWindow("Handmade Hero", 900, 900, SDL_WINDOW_RESIZABLE);
  if (!Window) {
    std::cerr << "Failed to create Window\n";
  }
  SDL_Renderer *r = SDL_CreateRenderer(Window, NULL);
  if (!r) {
    std::cerr << "Failed to create Renderer\n";
  }

  for (;;) {
    SDL_Event registeredEvent;
    SDL_WaitEvent(&registeredEvent);
    if (HandleEvent(&registeredEvent)) {
      break;
    }
  }

  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(Window);
  SDL_Quit();

  return 0;
}
