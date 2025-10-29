#include "SDL3/SDL.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>

#define Pi32 3.141592

typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

static bool Running = true;

// NOTE: Always 32-bits pixels. SDL_PIXELFORMAT_ABGR8888. Little Endian.
struct sdl_offscreen_buffer {
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

static void RenderWeirdGradiant(sdl_offscreen_buffer *Buffer, int x_offset,
                                int y_offset) {

  uint8 *Row = (uint8 *)Buffer -> Memory;
  for (int Y = 0; Y < Buffer -> Height; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer -> Width; ++X) {

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
    Row += Buffer -> Pitch;
  }
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

    RenderWeirdGradiant(&GlobalBackBuffer, 0, 0);
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

  static int x_offset = 0;
  static int y_offset = 0;

  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_W]) {
    y_offset -= 1;
    SDL_ResumeAudioDevice(GlobalAudioDeviceID);
    RenderWeirdGradiant(&GlobalBackBuffer, x_offset, y_offset);
  }
  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_S]) {
    y_offset += 1;
    RenderWeirdGradiant(&GlobalBackBuffer, x_offset, y_offset);
  }
  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_A]) {
    x_offset -= 1;

    RenderWeirdGradiant(&GlobalBackBuffer, x_offset, y_offset);
  }
  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_D]) {
    x_offset += 1;
    RenderWeirdGradiant(&GlobalBackBuffer, x_offset, y_offset);
  }

  if (GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_F4] &&
      GlobalPlayerWindowInput.KeyStates[SDL_SCANCODE_LALT]) {
    Running = false;
  }
}

// NOTE: Sound test
static SDL_AudioStream *GlobalAudioStream = nullptr;
int SamplesPerSecond = 48000;
int ToneHz = 440;
int16 ToneVolume = 1200;
uint32 RunningSampleIndex = 0;
int WavePeriod = SamplesPerSecond / ToneHz;
int BytesPerSample = sizeof(int16) * 2;
int BytesToWrite = 800 * BytesPerSample;

static void SDLInitAudio() {
  static SDL_AudioSpec audio_spec = {SDL_AUDIO_S16LE, 2, 48000};

  GlobalAudioStream = SDL_CreateAudioStream(&audio_spec, &audio_spec);
  GlobalAudioDeviceID =
      SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec);

  if (GlobalAudioStream && GlobalAudioDeviceID) {
    SDL_BindAudioStream(GlobalAudioDeviceID, GlobalAudioStream);

    int SampleCount = BytesToWrite / BytesPerSample;
    void *AudioBuffer = malloc(BytesToWrite);
    int16 *SampleOut = (int16 *)AudioBuffer;
    for (int SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex) {
      real32 t = 2.0f * Pi32 * (real32)RunningSampleIndex;
      t /= (real32)WavePeriod;
      real32 SineValue = sinf(t);
      int16 SampleValue = (int16)(SineValue * ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;
      RunningSampleIndex++;
    }

    SDL_PutAudioStreamData(GlobalAudioStream, AudioBuffer, BytesToWrite);
    free(AudioBuffer);
    std::cout << "Audio device successfully initialized and started.\n";
  } else {
    std::cerr << "Failed to initialize audio: " << SDL_GetError() << "\n";
  }
}

static int AudioBufferBytes =
    SamplesPerSecond * BytesPerSample; // 1 second of audio
static void SDLFillerAudioBuffer() {
  int BytesQueued = SDL_GetAudioStreamAvailable(GlobalAudioStream);
  int BytesToGenerate = AudioBufferBytes - BytesQueued;

  if (BytesToGenerate > 0) {
    void *AudioBuffer = malloc(BytesToGenerate);
    int SampleCount = BytesToGenerate / BytesPerSample;
    int16 *SampleOut = (int16 *)AudioBuffer;

    for (int SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex) {
      real32 t = 2.0f * Pi32 * (real32)RunningSampleIndex;
      t /= (real32)WavePeriod;
      real32 SineValue = sinf(t);
      int16 SampleValue = (int16)(SineValue * ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;
      RunningSampleIndex++;
    }
    SDL_PutAudioStreamData(GlobalAudioStream, AudioBuffer, BytesToGenerate);
    free(AudioBuffer);
  }
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
  RenderWeirdGradiant(&GlobalBackBuffer, 0, 0);

  uint64 PerfCountFrecuency = SDL_GetPerformanceFrequency();
  while (Running) {
    uint64 LastCounter = SDL_GetPerformanceCounter();
    SDL_Event registeredEvent;
    while (SDL_PollEvent(&registeredEvent)) {
      if (HandleEvent(&registeredEvent)) {
        Running = false;
      }
    }
    SDLFillerAudioBuffer();
    KeyBoardStatusChange();
    SDLDisplayBufferWindow(r, GlobalBackBuffer);

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
