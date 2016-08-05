#include <stdio.h>
#include <stdint.h>

#include "spr_numeros.c"
#include "spr_borrado_hr.c"
#include "spr_borrado_vr.c"
#include "spr_numeros_z1.c"
#include "spr_numeros_z2.c"

#define COLOR_RGB(r, g, b) (((r<<3) & 0x00F8) | ((g>>3) & 0x0007) | ((g<<13) & 0xE000) | ((b<<8) & 0x1F00))

const uint32_t nPixelSprNumeros = SPR_NUMEROS_WIDTH * SPR_NUMEROS_HEIGHT * SPR_NUMEROS_BYTES_PER_PIXEL;
const uint32_t nPixelSprBorradoHr = SPR_BORRADO_HR_WIDTH * SPR_BORRADO_HR_HEIGHT * SPR_BORRADO_HR_BYTES_PER_PIXEL;
const uint32_t nPixelSprBorradoVr = SPR_BORRADO_VR_WIDTH * SPR_BORRADO_VR_HEIGHT * SPR_BORRADO_VR_BYTES_PER_PIXEL;
const uint32_t nPixelSprNumerosZ1 = SPR_NUMEROS_Z1_WIDTH * SPR_NUMEROS_Z1_HEIGHT * SPR_NUMEROS_Z1_BYTES_PER_PIXEL;
const uint32_t nPixelSprNumerosZ2 = SPR_NUMEROS_Z2_WIDTH * SPR_NUMEROS_Z2_HEIGHT * SPR_NUMEROS_Z2_BYTES_PER_PIXEL;

const uint32_t anchoFilaTxtSprNumeros = 48;
const uint32_t anchoFilaTxtSprBorradoHr = 15;
const uint32_t anchoFilaTxtSprBorradoVr = 48;
const uint32_t anchoFilaTxtSprNumerosZ1 = 48;
const uint32_t anchoFilaTxtSprNumerosZ2 = 48;

int main() {
  uint32_t i, j = 0;
  uint16_t r, g, b;
  FILE *pArchSprNumeros;
  FILE *pArchSprBorradoHr;
  FILE *pArchSprBorradoVr;
  FILE *pArchSprNumerosZ1;
  FILE *pArchSprNumerosZ2;

  pArchSprNumeros = fopen("../arduino/dosala-N/sprites/spr_numeros.h", "w");
  for (i=0; i<nPixelSprNumeros; i+=3) {
    if (j==0)
      fprintf(pArchSprNumeros, "  ");

    r = SPR_NUMEROS_PIXEL_DATA[i+0] >> 3;
    g = SPR_NUMEROS_PIXEL_DATA[i+1] >> 2;
    b = SPR_NUMEROS_PIXEL_DATA[i+2] >> 3;

    fprintf(pArchSprNumeros, "0x%04x, ", COLOR_RGB(r, g, b));

    if (j==anchoFilaTxtSprNumeros-1)
      fprintf(pArchSprNumeros, "\n");

    j = (j+1) % anchoFilaTxtSprNumeros;
  }
  fclose(pArchSprNumeros);

  pArchSprBorradoHr = fopen("../arduino/dosala-N/sprites/spr_borrado_hr.h", "w");
  for (i=0; i<nPixelSprBorradoHr; i+=3) {
    if (j==0)
      fprintf(pArchSprBorradoHr, "  ");

    r = SPR_BORRADO_HR_PIXEL_DATA[i+0] >> 3;
    g = SPR_BORRADO_HR_PIXEL_DATA[i+1] >> 2;
    b = SPR_BORRADO_HR_PIXEL_DATA[i+2] >> 3;

    fprintf(pArchSprBorradoHr, "0x%04x, ", COLOR_RGB(r, g, b));

    if (j==anchoFilaTxtSprBorradoHr-1)
      fprintf(pArchSprBorradoHr, "\n");

    j = (j+1) % anchoFilaTxtSprBorradoHr;
  }
  fclose(pArchSprBorradoHr);

  pArchSprBorradoVr = fopen("../arduino/dosala-N/sprites/spr_borrado_vr.h", "w");
  for (i=0; i<nPixelSprBorradoVr; i+=3) {
    if (j==0)
      fprintf(pArchSprBorradoVr, "  ");

    r = SPR_BORRADO_VR_PIXEL_DATA[i+0] >> 3;
    g = SPR_BORRADO_VR_PIXEL_DATA[i+1] >> 2;
    b = SPR_BORRADO_VR_PIXEL_DATA[i+2] >> 3;

    fprintf(pArchSprBorradoVr, "0x%04x, ", COLOR_RGB(r, g, b));

    if (j==anchoFilaTxtSprBorradoVr-1)
      fprintf(pArchSprBorradoVr, "\n");

    j = (j+1) % anchoFilaTxtSprBorradoVr;
  }
  fclose(pArchSprBorradoVr);

  pArchSprNumerosZ1 = fopen("../arduino/dosala-N/sprites/spr_numeros_z1.h", "w");
  for (i=0; i<nPixelSprNumerosZ1; i+=3) {
    if (j==0)
      fprintf(pArchSprNumerosZ1, "  ");

    r = SPR_NUMEROS_Z1_PIXEL_DATA[i+0] >> 3;
    g = SPR_NUMEROS_Z1_PIXEL_DATA[i+1] >> 2;
    b = SPR_NUMEROS_Z1_PIXEL_DATA[i+2] >> 3;

    fprintf(pArchSprNumerosZ1, "0x%04x, ", COLOR_RGB(r, g, b));

    if (j==anchoFilaTxtSprNumerosZ1-1)
      fprintf(pArchSprNumerosZ1, "\n");

    j = (j+1) % anchoFilaTxtSprNumerosZ1;
  }
  fclose(pArchSprNumerosZ1);

  pArchSprNumerosZ2 = fopen("../arduino/dosala-N/sprites/spr_numeros_z2.h", "w");
  for (i=0; i<nPixelSprNumerosZ2; i+=3) {
    if (j==0)
      fprintf(pArchSprNumerosZ2, "  ");

    r = SPR_NUMEROS_Z2_PIXEL_DATA[i+0] >> 3;
    g = SPR_NUMEROS_Z2_PIXEL_DATA[i+1] >> 2;
    b = SPR_NUMEROS_Z2_PIXEL_DATA[i+2] >> 3;

    fprintf(pArchSprNumerosZ2, "0x%04x, ", COLOR_RGB(r, g, b));

    if (j==anchoFilaTxtSprNumerosZ2-1)
      fprintf(pArchSprNumerosZ2, "\n");

    j = (j+1) % anchoFilaTxtSprNumerosZ2;
  }
  fclose(pArchSprNumerosZ2);

  return 0;
}