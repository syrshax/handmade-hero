#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

int main(int argc, char *argv[]) {

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error initializing",
                             "Error initialzing SDL WITH VIDEO", 0);
  }

  SDL_Window *Window;

  Window = SDL_CreateWindow("Handmade Hero", 0, 0, SDL_WINDOW_RESIZABLE);

  return 0;
}
