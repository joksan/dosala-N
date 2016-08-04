#include <SPI.h>
#include "LCD_ILI9320.h"

const int pinLcdCS = 10;
const int pinLcdReset = 14;

LCD_ILI9320 lcd(pinLcdCS, pinLcdReset);

uint16_t imagen[32 * 64];
const uint16_t sprites[48*48*16] = {
  #include "sprites_z1.h"
};
//const uint16_t datosFlash[8] = { 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0xFFFF };

void setup() {
  Serial.begin(9600);

  lcd.inicializar();

  Serial.println("Iniciando Prueba...");

  for (int i = 0; i < 32 * 64; i++) //Fondo
    imagen[i] = COLOR_RGB(31, 63, 31);
  for (int i = 0; i < 32; i++) //Linea superior
    imagen[i] = COLOR_RGB(0, 0, 31);
  for (int i = 0; i < 32; i++) //Linea inferior
    imagen[63*32+i] = COLOR_RGB(0, 0, 31);
  for (int i = 0; i < 64; i++) //Linea izquierda
    imagen[i*32] = COLOR_RGB(0, 0, 31);
  for (int i = 0; i < 64; i++) //Linea derecha
    imagen[i*32+31] = COLOR_RGB(0, 0, 31);
  for (int i = 0; i < 32; i++) {  //Diagonal "\"
    imagen[i*64+i] = COLOR_RGB(31, 0, 0);
    imagen[i*64+i+32] = COLOR_RGB(31, 0, 0);
  }
  for (int i = 0; i < 32; i++) {  //Diagonal "/"
    imagen[i*64+31-i] = COLOR_RGB(31, 0, 0);
    imagen[i*64+31-i+32] = COLOR_RGB(31, 0, 0);
  }
}

void loop() {
  lcd.blitSol(0, 0, 240, 64, COLOR_RGB(31, 63, 31));
  lcd.blitSol(0, 64, 240, 64, COLOR_RGB(31, 63, 31));
  lcd.blitSol(0, 128, 240, 64, COLOR_RGB(31, 63, 31));
  lcd.blitSol(0, 192, 240, 64, COLOR_RGB(31, 63, 31));
  lcd.blitSol(0, 256, 240, 64, COLOR_RGB(31, 63, 31));

  for (int j=0; j<4; j++) {
    for (int i=0; i<4; i++) {
      lcd.blitImg(60*i+6, 60*j+6, 48, 48, &sprites[48*48*(j*4+i)]);      
    }
  }

  for (;;);

  /*
  lcd.blitSol(0, 0, 240, 64, COLOR_RGB(31, 63, 0));
  lcd.blitSol(0, 64, 240, 64, COLOR_RGB(31, 63, 0));
  lcd.blitSol(0, 128, 240, 64, COLOR_RGB(31, 63, 0));
  lcd.blitSol(0, 192, 240, 64, COLOR_RGB(31, 63, 0));
  lcd.blitSol(0, 256, 240, 64, COLOR_RGB(31, 63, 0));
  //delay(1000);
  lcd.blitImg(160, 120, 32, 64, imagen);
  delay(1000);
  lcd.blitSol(0, 0, 240, 64, COLOR_RGB(0, 63, 31));
  lcd.blitSol(0, 64, 240, 64, COLOR_RGB(0, 63, 31));
  lcd.blitSol(0, 128, 240, 64, COLOR_RGB(0, 63, 31));
  lcd.blitSol(0, 192, 240, 64, COLOR_RGB(0, 63, 31));
  lcd.blitSol(0, 256, 240, 64, COLOR_RGB(0, 63, 31));
  //delay(1000);
  lcd.blitImg(160, 120, 32, 64, imagen);
  delay(1000);
  */
}
