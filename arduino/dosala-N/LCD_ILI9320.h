#ifndef LCD_ILI9320_H_INCLUIDA
#define LCD_ILI9320_H_INCLUIDA

#include <DMAChannel.h>

class LCD_ILI9320 {
public:
  explicit LCD_ILI9320(int pin_cs, int pin_reset);
  void inicializar();
  void blitImg(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, const uint16_t *img);
  void blitSol(uint16_t x, uint16_t y, uint16_t ancho, uint16_t alto, uint16_t color);

private:
  void selReg(uint8_t reg);
  uint16_t leerReg(uint8_t reg);
  void escribirReg(uint8_t reg, uint16_t dato);

  DMAChannel CanalDMA;
  const int pinCS;
  const int pinReset;
};

#define COLOR_RGB(r, g, b) (((r<<3) & 0x00F8) | ((g>>3) & 0x0007) | ((g<<13) & 0xE000) | ((b<<8) & 0x1F00))

#endif //LCD_ILI9320_H_INCLUIDA
