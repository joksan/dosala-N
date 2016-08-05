#include <SPI.h>
#include <SparkFun_APDS9960.h>
#include "LCD_ILI9320.h"
#include "Juego.h"

const int pinLcdCS = 10;
const int pinLcdReset = 14;

//Instancia de la clase del controlador de la LCD
LCD_ILI9320 lcd(pinLcdCS, pinLcdReset);

//Instancia de la clase del sensor de gestos
SparkFun_APDS9960 sensor = SparkFun_APDS9960();

void setup() {
  //Se inicializa el puerto serie
  Serial.begin(9600);

  //La semilla del generador de numeros pseudo-aleatorios se alimenta con un pin de ADC abierto
  randomSeed(analogRead(1));

  //Se inicializa el sensor de gestos
  sensor.init();
  sensor.enableGestureSensor(false);
  sensor.setLEDDrive(LED_DRIVE_12_5MA);  //Este par de calibraciones se hace porque el sensor esta muy sensible
  sensor.setGestureGain(GGAIN_1X);       //por default. Mermar la ganancia y la luz mejora la respuesta.

  //Se inicializa la pantalla LCD
  lcd.inicializar();
}

void loop() {
  static long tSiguienteAct = 0;
  long tActual;
  char caracter = 0;

  //Verifica si ya se alcanzo el tiempo para efectuar la siguiente actualizacion de la logica
  tActual = millis();
  if (tActual < tSiguienteAct) return;  //Si aun no se alcanza, retorna
  tSiguienteAct = tActual + 40;         //Si se alcanzo, programa la siguiente actualizacion y prosigue

  //Se lee el puerto serie y se emite un comando a la logica de juego
  //segun el caracter recibido
  if (Serial.available()) {
    caracter = Serial.read();
    switch (caracter) {
      case 'w':
        accion(CC_ARRIBA);
        break;
      case 'a':
        accion(CC_IZQUIERDA);
        break;
      case 's':
        accion(CC_ABAJO);
        break;
      case 'd':
        accion(CC_DERECHA);
        break;
      case 'r':
        accion(CC_REINICIAR);
        break;
    }
  }

  //Se procede a leer el sensor y se genera un comando a la logica de juego
  //segun el gesto detectado (si se detecto uno)
  if (sensor.isGestureAvailable()) {
    switch (sensor.readGesture()) {
      case DIR_LEFT:
        accion(CC_IZQUIERDA);
        break;
      case DIR_RIGHT:
        accion(CC_DERECHA);
        break;
      case DIR_UP:
        accion(CC_ARRIBA);
        break;
      case DIR_DOWN:
        accion(CC_ABAJO);
      case DIR_FAR:
        accion(CC_REINICIAR);
        break;
    }
  }

  //Se actualiza toda la logica del juego una vez
  hacerLogicaJuego();
}
