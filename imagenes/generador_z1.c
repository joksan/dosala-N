#include <stdio.h>
#include <stdint.h>

#include "sprites_z1.c"

#define COLOR_RGB(r, g, b) (((r<<3) & 0x00F8) | ((g>>3) & 0x0007) | ((g<<13) & 0xE000) | ((b<<8) & 0x1F00))

const uint32_t nPixel = GIMP_IMAGE_WIDTH * GIMP_IMAGE_HEIGHT * GIMP_IMAGE_BYTES_PER_PIXEL;
const uint32_t anchoFilaTxt = 48;

int main() {
  uint32_t i, j = 0;
  uint16_t r, g, b;
  FILE *pArch;

  pArch = fopen("../arduino/dosala-N/sprites_z1.h", "w");

  for (i=0; i<nPixel; i+=3) {
    if (j==0)
      fprintf(pArch, "  ");

    r = GIMP_IMAGE_PIXEL_DATA[i+0] >> 3;
    g = GIMP_IMAGE_PIXEL_DATA[i+1] >> 2;
    b = GIMP_IMAGE_PIXEL_DATA[i+2] >> 3;

    fprintf(pArch, "0x%04x, ", COLOR_RGB(r, g, b));

    if (j==anchoFilaTxt-1)
      fprintf(pArch, "\n");

    j = (j+1) % anchoFilaTxt;
  }

  fclose(pArch);

  return 0;
}