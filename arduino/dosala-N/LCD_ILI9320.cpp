#include <Arduino.h>
#include <usb_serial.h>
#include <SPI.h>
#include "LCD_ILI9320.h"

static const byte LCD_INSTR_SET_INDEX = 0x70;
static const byte LCD_INSTR_DATA_WR = 0x72;
static const byte LCD_INSTR_DATA_RD = 0x73;

LCD_ILI9320::LCD_ILI9320(int pin_cs, int pin_reset):
  pinCS(pin_cs), pinReset(pin_reset)
{}

void LCD_ILI9320::inicializar() {
  //Se inicializan los pines de control
  pinMode(pinCS, OUTPUT);
  pinMode(pinReset, OUTPUT);

  //Se inicializa el periferico SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);

  //Inicializacion de DMA
  CanalDMA.disableOnCompletion();                         //El DMA se desactiva al terminar la transferencia
  CanalDMA.destination((volatile uint8_t &)SPI0_PUSHR);   //El destino sera siempre el buffer de transmision SPI
  SPI0_RSER |= SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS;     //Se habilita al SPI para solicitar datos via DMA
  CanalDMA.triggerAtHardwareEvent(DMAMUX_SOURCE_SPI0_TX); //El DMA envia datos siempre que el SPI lo solicita

  //Secuencia de pin de reset
  digitalWrite(pinReset, HIGH);
  delay(1);
  digitalWrite(pinReset, LOW);
  delay(1);
  digitalWrite(pinReset, HIGH);
  delay(1);

  //Se intenta detectar la pantalla
  if (leerReg(0x00) != 0x9320) {
    Serial.println("Pantalla no detectada!");
    return;
  }

  //Se configuran todos los registros del display
  escribirReg(0x00, 0x0000); //Start Oscillation
  escribirReg(0x01, 0x0000); //Driver Output Control: SS=0, SM=0
  escribirReg(0x02, 0x0700); //LCD Driving Wave Control: B/C=1, EOR=1
  escribirReg(0x03, 0x1030); //Entry Mode: TRI=0, DFM=0, BGR=1, HWM=0, ORG=0, I/D=11, AM=0
  escribirReg(0x04, 0x0000); //Resizing Control Register: RCV=00, RCH=00, RSZ=00
  escribirReg(0x08, 0x0202); //Display Control 2: FP=0010, BP=0010
  escribirReg(0x09, 0x0000); //Display Control 3: PST=000, PTG=00, ISC=0000
  escribirReg(0x0a, 0x0000); //Display Control 4: FMARKOE=0, FMI=000
  escribirReg(0x0c, 0x0001); //RGB Display Interface Control 1: ENC=000, RM=0, DM=00, RIM=01
  escribirReg(0x0d, 0x0000); //Frame Maker Position: FMP=000000000
  escribirReg(0x0f, 0x0000); //RGB Display Interface Control 2: VSPL=0, HSPL=0, EPL=0, DPL=0
  delay(50);
  escribirReg(0x07, 0x0101); //Display Control: PDTE=00, BASEE=1, GON=0, DTE=0, CL=0, D=01
  delay(50);
  escribirReg(0x10, 0x10C0); //Power Control 1: SAP=1, BT=0000, APE=1, AP=100, DSTB=0, SLP=0
  escribirReg(0x11, 0x0007); //Power Control 2: DC1=000, DC0=000, VC=7
  escribirReg(0x12, 0x0110); //Power Control 3: VCMR=1, PON=1, VRH=0000
  escribirReg(0x13, 0x0B00); //Power Control 4: VDV=01011
  escribirReg(0x29, 0x0000); //Power Control 7: VCM=00000
  escribirReg(0x2B, 0x0010); //Frame Rate and Color Control: EXT_R=0, FR_SEL=01

  escribirReg(0x50, 0);      //Horizontal start address
  escribirReg(0x51, 239);    //Horizontal end address
  escribirReg(0x52, 0);      //Vertical start address
  escribirReg(0x53, 319);    //Vertical end address
  delay(50);

  escribirReg(0x60, 0xA700); //Gate Scan Control: GS=1, NL=100111, SCN=000000
  escribirReg(0x61, 0x0001); //Gate Scan Control: NDL=0, VLE=0, REV=1
  escribirReg(0x6a, 0x0000); //Gate Scan Control: VL=000000000

  escribirReg(0x80, 0x0000); //Partial Image 1 Display Position: PTD=000000000
  escribirReg(0x81, 0x0000); //Partial Image 1 RAM Start/End Address: PTS=000000000
  escribirReg(0x82, 0x0000); //Partial Image 1 RAM Start/End Address: PTE=000000000
  escribirReg(0x83, 0x0000); //Partial Image 2 Display Position: PTD=000000000
  escribirReg(0x84, 0x0000); //Partial Image 2 RAM Start/End Address: PTS=000000000
  escribirReg(0x85, 0x0000); //Partial Image 2 RAM Start/End Address: PTE=000000000

  escribirReg(0x90, 0x0010); //Panel Interface Control 1: DIV=00, RTNI=10000
  escribirReg(0x92, 0x0000); //Panel Interface Control 2: NOWI=000
  escribirReg(0x93, 0x0001); //Panel Interface Control 3: MCPI=001
  escribirReg(0x95, 0x0110); //Panel Interface Control 4: DIVE=01, RTNE=010000
  escribirReg(0x97, 0x0000); //Panel Interface Control 5: NOWE=0000
  escribirReg(0x98, 0x0000); //Panel Interface Control 6: MCPE=000

  //Finalmente se habilita el display
  escribirReg(0x07, 0x0133); //Display Control: PDTE=00, BASEE=1, GON=1, DTE=1, CL=0, D=11
  delay(100);
}

void LCD_ILI9320::blitImg(uint16_t x, uint16_t y, uint16_t ancho,
                          uint16_t alto, const uint16_t *img)
{
  escribirReg(0x20, x);              //Coordenada X
  escribirReg(0x21, y);              //Coordenada Y
  escribirReg(0x50, x);              //Inicio X
  escribirReg(0x51, x + ancho - 1);  //Fin X
  escribirReg(0x52, y);              //Inicio Y
  escribirReg(0x53, y + alto - 1);   //Fin Y

  //Selecciona el registro de escritura de datos a GRAM
  selReg(0x22);

  //Habilita el estrobo de la LCD y prepara la instruccion de escritura de
  //datos
  digitalWrite(pinCS, LOW);
  SPI.transfer(LCD_INSTR_DATA_WR);

  //Prepara la transaccion DMA
  CanalDMA.sourceBuffer(img, ancho*alto*2);
  CanalDMA.transferCount(ancho * alto * 2);
  CanalDMA.transferSize(1);

  //Habiltia la transacccion DMA y espera a que termine
  CanalDMA.enable();
  while (!CanalDMA.complete());
  delayMicroseconds(5);         //Espera a que el buffer SPI se vacie

  //Deshabilita el estrobo de la LCD
  digitalWrite(pinCS, HIGH);
}

void LCD_ILI9320::blitSol(uint16_t x, uint16_t y, uint16_t ancho,
                          uint16_t alto, uint16_t color)
{
  escribirReg(0x20, x);              //Coordenada X
  escribirReg(0x21, y);              //Coordenada Y
  escribirReg(0x50, x);              //Inicio X
  escribirReg(0x51, x + ancho - 1);  //Fin X
  escribirReg(0x52, y);              //Inicio Y
  escribirReg(0x53, y + alto - 1);   //Fin Y

  //Selecciona el registro de escritura de datos a GRAM
  selReg(0x22);

  //Habilita el estrobo de la LCD y prepara la instruccion de escritura de
  //datos
  digitalWrite(pinCS, LOW);
  SPI.transfer(LCD_INSTR_DATA_WR);

  //Prepara la transaccion DMA
  CanalDMA.sourceCircular(&color, 2);
  CanalDMA.transferCount(ancho * alto * 2);
  CanalDMA.transferSize(1);

  //Habiltia la transacccion DMA y espera a que termine
  CanalDMA.enable();
  while (!CanalDMA.complete());
  delayMicroseconds(5);         //Espera a que el buffer SPI se vacie

  //Deshabilita el estrobo de la LCD
  digitalWrite(pinCS, HIGH);
}

//Metodos privados
//--------------------------------------------------------------------------------
void LCD_ILI9320::selReg(uint8_t reg) {
  digitalWrite(pinCS, LOW);
  SPI.transfer(LCD_INSTR_SET_INDEX);
  SPI.transfer(0x00);
  SPI.transfer(reg);
  digitalWrite(pinCS, HIGH);
}

uint16_t LCD_ILI9320::leerReg(uint8_t reg) {
  byte datoL, datoH;

  //Selecciona el registro a acceder
  digitalWrite(pinCS, LOW);
  SPI.transfer(LCD_INSTR_SET_INDEX);
  SPI.transfer(0x00);
  SPI.transfer(reg);
  digitalWrite(pinCS, HIGH);

  //Realiza la operacion de lectura
  digitalWrite(pinCS, LOW);
  SPI.transfer(LCD_INSTR_DATA_RD);
  SPI.transfer(0x00);
  datoH = SPI.transfer(0x00);
  datoL = SPI.transfer(0x00);
  digitalWrite(pinCS, HIGH);

  return word(datoH, datoL);
}

void LCD_ILI9320::escribirReg(uint8_t reg, uint16_t dato) {
  //Selecciona el registro a acceder
  digitalWrite(pinCS, LOW);
  SPI.transfer(LCD_INSTR_SET_INDEX);
  SPI.transfer(0x00);
  SPI.transfer(reg);
  digitalWrite(pinCS, HIGH);

  //Realiza la operacion de escritura
  digitalWrite(pinCS, LOW);
  SPI.transfer(LCD_INSTR_DATA_WR);
  SPI.transfer(highByte(dato));
  SPI.transfer(lowByte(dato));
  digitalWrite(pinCS, HIGH);
}
