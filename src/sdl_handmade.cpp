
/*
 * Save location
 * Getting a handle to our own executable file
 * Asset loading path
 * Threading (launching a thread)
 * Raw Input -> support for multiple keyboards
 * Sleep/timeBegingPeriod (to not to burn the CPU)
 * ClipCursor() for multimonitor support
 * Fullscreen support
 * WH_SETCURSOR (win32) -> control cursor
 * NOT_ACTIVE_APP -> control when not main app
 * Blit speed impriovements (image buffer)
 * Hardware Acceleration (openGL)
 * GetKeyboardLayout (international support)
 *
 * Just a litte list of todos for a shipping state...
 */

#include "handmade.h"

#include "SDL3/SDL.h"
#include "handmade.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>
static bool Running = true;

// NOTE: Always 32-bits pixels. SDL_PIXELFORMAT_ABGR8888. Little Endian.
struct sdl_offscreen_buffer { // NOTE: THIS HAVE THE TEXTURES
  SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

struct player_inputs {
  const bool *KeyStates;
};

struct sdl_window_dimension {
  int Width;
  int Height;
};

static sdl_offscreen_buffer GlobalBackBuffer{};
static player_inputs GlobalPlayerWindowInput{};

static sdl_window_dimension SDLGetWindowDimension(SDL_Window *w) { // ignore
  sdl_window_dimension r;
  SDL_GetWindowSize(w, &r.Width, &r.Height);
  return r;
}

// TODO: This will be filed
static void SDLResizeTextureBuffer(sdl_offscreen_buffer *Buffer,
                                   SDL_Renderer *r, int width, int height) {
  int BitsPerPixel = 4;

  if (Buffer->Memory) {
    munmap(Buffer->Memory, ((Buffer->Width * Buffer->Height) * BitsPerPixel));
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

static void SDLDisplayBufferWindow(SDL_Renderer *r,
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

static int GlobalAudioDeviceID = 0;

static void KeyBoardStatusChange() {
  GlobalPlayerWindowInput.KeyStates = SDL_GetKeyboardState(NULL);

  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_W]) {
    SDL_ResumeAudioDevice(GlobalAudioDeviceID);
  }
  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_S]) {
  }
  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_A]) {
  }
  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_D]) {
  }

  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_F4] &&
      GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_LALT]) {
    Running = false;
  }
}

// NOTE: Sound test
static SDL_AudioStream *GlobalAudioStream = nullptr;

static void SDLInitAudio() {
  static SDL_AudioSpec audio_spec = {SDL_AUDIO_S16LE, 2, 48000};
  GlobalAudioStream = SDL_CreateAudioStream(&audio_spec, &audio_spec);
  GlobalAudioDeviceID =
      SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec);

  if (GlobalAudioStream && GlobalAudioDeviceID) {
    SDL_BindAudioStream(GlobalAudioDeviceID, GlobalAudioStream);
    SDL_ResumeAudioDevice(GlobalAudioDeviceID);
    std::cout << "Audio device initialized.\n";
  } else {
    std::cerr << "Failed to initialize audio: " << SDL_GetError() << "\n";
  }
}

static void SDLFillAudioBuffer(game_sound_output_buffer *SoundBuffer) {
  int BytesPerSample = sizeof(int16) * 2;
  int BytesToWrite = SoundBuffer->SampleCount * BytesPerSample;

  SDL_PutAudioStreamData(GlobalAudioStream, SoundBuffer->Samples, BytesToWrite);
}

int main() {
  if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
    std::cerr << "Error initializing the VIDEO DRIVER | AUDIO DRIVER! \n";
  };

  /* We create the Window and Renderer (our layout)
   */

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

  /* This generates the Audio Stream. Gets the default Device and Binds to it.
   * Then you need to use the functions to send data with PutAudioStreamData to
   * it.
   */

  /* Now the Game infinte loop */

  SDLInitAudio();
  SDLResizeTextureBuffer(&GlobalBackBuffer, r, 1280, 720);

  game_offscreen_buffer GameBuffer;

  int16 *Samples =
      (int16 *)malloc(48000 * sizeof(int16) * 2); // 1 second buffer
  game_sound_output_buffer SoundBuffer = {};
  SoundBuffer.SamplesPerSecond = 48000;
  SoundBuffer.SampleCount = SoundBuffer.SamplesPerSecond / 30; // 30 fps worth
  SoundBuffer.Samples = Samples;

  uint64 PerfCountFrecuency = SDL_GetPerformanceFrequency();
  while (Running) {
    uint64 LastCounter = SDL_GetPerformanceCounter();
    SDL_Event registeredEvent;
    while (SDL_PollEvent(&registeredEvent)) {
      if (HandleEvent(&registeredEvent)) {
        Running = false;
      }
    }
    KeyBoardStatusChange();

    GameBuffer.Memory = GlobalBackBuffer.Memory;
    GameBuffer.Width = GlobalBackBuffer.Width;
    GameBuffer.Height = GlobalBackBuffer.Height;
    GameBuffer.Pitch = GlobalBackBuffer.Pitch;

    GameUpdateAndRender(&GameBuffer, 0, 0, &SoundBuffer, 440);

    SDLDisplayBufferWindow(r, GlobalBackBuffer);
    SDLFillAudioBuffer(&SoundBuffer);

    uint64 EndCounter = SDL_GetPerformanceCounter();
    uint64 CounterElapsed = EndCounter - LastCounter;

    real64 MSPerFrame =
        (((1000.0f * (real64)CounterElapsed) / (real64)PerfCountFrecuency));
    real64 FPS = (real64)PerfCountFrecuency / (real64)CounterElapsed;

    printf("MILLISECONDS PER FRAME -> %.02f ms/f, \n FPS -> %.02ff/s\n",
           MSPerFrame, FPS);
  }

  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(Window);
  SDL_Quit();

  return 0;
}
