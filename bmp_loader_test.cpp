#include <cstdint>
#include <cstdlib>
#include <stdio.h>

typedef uint8_t uint8;
typedef int16_t int16;
typedef int32_t int32;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef char32_t bool32;

typedef float real32;
typedef double real64;

struct debug_read_file_result {
  uint32_t ContentsSize;
  void *Contents;
};

void freeFile(void *Memory) {
  if (Memory) {
    free(Memory);
  }
}

#pragma pack(push, 1)
struct bitmap_header {
  uint16 FileType;
  uint32 FileSize;
  uint16 Reserved1;
  uint16 Reserved2;
  uint32 BitmapOffSet; // starts of pixel data
  uint32 Size;
  int32 Width;
  int32 Height;
  uint16 Planes;
  uint16 BitCount;    // bits per pixel
  uint32 Compression; // 0, 1, 2 or 3 type;
  uint32 ImageSize;
  int32 HorzResolution;
  int32 VertResolution;
  uint32 ColorsUsed;
  uint32 ColorsImportant;
};
#pragma pack(pop)

debug_read_file_result readFile(const char *Filename) {
  debug_read_file_result Result = {};
  FILE *File = fopen(Filename, "rb");
  if (File) {
    fseek(File, 0, SEEK_END);
    uint32_t FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    Result.Contents = malloc(FileSize);
    if (Result.Contents) {
      size_t BytesRead = fread(Result.Contents, 1, FileSize, File);
      if (BytesRead == FileSize) {
        Result.ContentsSize = FileSize;
      } else {
        freeFile(Result.Contents);
        Result.Contents = 0;
      }
    }
    fclose(File);
  }
  return Result;
}

static void loadBMP(const char *Filename) {
  debug_read_file_result ReadResult = readFile(Filename);

  if (!ReadResult.Contents) {
    printf("Error: no se pudo leer el archivo %s\n", Filename);
    return;
  }

  bitmap_header *Header = (bitmap_header *)ReadResult.Contents;

  // Verificar que es un BMP válido ("BM" = 0x4D42)
  if (Header->FileType != 0x4D42) {
    printf("Error: no es un BMP válido\n");
    freeFile(ReadResult.Contents);
    return;
  }

  printf("=== BMP Header ===\n");
  printf("FileSize:      %u bytes\n", Header->FileSize);
  printf("BitmapOffset:  %u bytes\n", Header->BitmapOffSet);
  printf("Width:         %d px\n", Header->Width);
  printf("Height:        %d px\n", Header->Height);
  printf("BitsPerPixel:  %u\n", Header->BitCount);
  printf("Compression:   %u\n", Header->Compression);
  printf("ImageSize:     %u bytes\n", Header->ImageSize);

  uint8 *PixelData = (uint8 *)ReadResult.Contents + Header->BitmapOffSet;

  int32 BitsPerPixel = Header->BitCount;
  int32 BytesPerPixel = BitsPerPixel / 8;

  printf("\n=== Primeros 5 píxeles ===\n");
  for (int i = 0; i < 5; i++) {
    uint8 *Pixel = PixelData + (i * BytesPerPixel);
    if (BitsPerPixel == 24) {
      printf("Pixel[%d]: B=%u G=%u R=%u\n", i, Pixel[0], Pixel[1], Pixel[2]);
    } else if (BitsPerPixel == 32) {
      printf("Pixel[%d]: B=%u G=%u R=%u A=%u\n", i, Pixel[0], Pixel[1],
             Pixel[2], Pixel[3]);
    }
  }

  freeFile(ReadResult.Contents);
}

int main() {
  loadBMP("src/data/lena24.bmp");
  return 0;
}
