#include "SDL3/SDL.h"

int main() {
  SDL_Init(0);
  SDL_Window *w = SDL_CreateWindow("", 640, 480, 0);
  SDL_Renderer *r = SDL_CreateRenderer(w, NULL);

  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
  SDL_RenderClear(r);
  SDL_RenderPresent(r);

  SDL_Delay(2000);

  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(w);
  SDL_Quit();
  return 0;
}
