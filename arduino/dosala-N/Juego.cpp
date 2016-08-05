#include <Arduino.h>
#include "Juego.h"
#include "LCD_ILI9320.h"

extern LCD_ILI9320 lcd;

const uint16_t spr_numeros[48 * 48 * 16] = {
#include "sprites/spr_numeros.h"
};

const uint16_t spr_numeros_z1[48 * 48 * 16] = {
#include "sprites/spr_numeros_z1.h"
};

const uint16_t spr_numeros_z2[48 * 48 * 1] = {
#include "sprites/spr_numeros_z2.h"
};

const uint16_t spr_borrado_hr[15 * 48 * 8] = {
#include "sprites/spr_borrado_hr.h"
};

const uint16_t spr_borrado_vr[48 * 15 * 8] = {
#include "sprites/spr_borrado_vr.h"
};

//Enumeracion de los estados del automata finito de juego
enum ESTADO_JUEGO {
  EJ_INACTIVO, EJ_DESPLAZAR, EJ_COMBINAR, EJ_CREAR,
};

//Enumeracion de las direcciones de movimiento
enum DIR_MOVIMIENTO {
  DM_IZQ, DM_DER, DM_ARR, DM_ABA,
};

//Variables de estado del automata finito de juego
bool bJuegoNuevo = true;                 //Bandera que indica se esta iniciando un juego
ESTADO_JUEGO estadoJuego = EJ_INACTIVO;  //Estado actual del automata
DIR_MOVIMIENTO dirMovimiento;            //Direccion de movimiento cuando se desplaza
bool bConsolidar = false;                //Bandera que indica si se deben consolidar las celdas al mover
bool bCombinar = false;                  //Bandera que indica si se deben animar las celdas combinadas
bool bCrear = false;                     //Bandera que indica si se debe crear una nueva celda tras mover
bool bAnimando = false;                  //Bandera que indica si se debe realizar una animacion
uint8_t pasoAnimacion = 0;                  //Conteo del paso actual de animacion

//Arreglo con los valores de todas las celdas del tablero. Notese que solo se guardan las potencias de
//2 a la que se elevan los numeros, asi si se guarda un 2, en realidad se guarda un 1, mientras que para
//un 4 se guarda un 2, para un 1024 se guarda un 10, y asi sucesivamente. Los espacios vacios se marcan
//con valor de cero
uint8_t valorCelda[4 * 4] = {
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0,
  0,  0,  0,  0,
};

//Arreglo que marca las celdas que se estan desplazando en un momento dado
bool celdaDesplazada[4 * 4] = {
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
};

//Arreglo que marca las celdas que fueron combinadas durante un movimiento dado
bool celdaCombinada[4 * 4] = {
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
};

//Arreglo que marca las celdas que fueron creadas en un momento dado
bool celdaCreada[4 * 4] = {
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
};

//Indices relativos que apuntan a celdas contiguas. Esto es una optimizacion para unidimensionalizar
//los accesos dentro de los arreglos, explotando la geometria de 4x4 de los mismos.
const int8_t posCelIzq = -1;
const int8_t posCelDer = 1;
const int8_t posCelArr = -4;
const int8_t posCelAba = 4;

//Declaracion previa de las funciones locales
static void reiniciarJuego();
static void hacerAnimacion();
static void crearCelda();
static void dibujarTableroJuegoNuevo();

//----------------------------------------------------------------------------------------------------

void accion(COMANDO_CONTROL comando) {
  //Si el estado del juego esta realizando una accion previa, el nuevo
  //comando se ignora
  if (estadoJuego != EJ_INACTIVO) return;

  //Si el estado de juego es inactivo, se actua de acuerdo al comando recibido
  switch (comando) {
    case CC_IZQUIERDA:
      estadoJuego = EJ_DESPLAZAR;
      dirMovimiento = DM_IZQ;
      break;
    case CC_DERECHA:
      estadoJuego = EJ_DESPLAZAR;
      dirMovimiento = DM_DER;
      break;
    case CC_ARRIBA:
      estadoJuego = EJ_DESPLAZAR;
      dirMovimiento = DM_ARR;
      break;
    case CC_ABAJO:
      estadoJuego = EJ_DESPLAZAR;
      dirMovimiento = DM_ABA;
      break;
    case CC_REINICIAR:
      reiniciarJuego();
      break;
  }
}

//----------------------------------------------------------------------------------------------------

void hacerLogicaJuego() {
  int8_t i, i_ini = 0, i_fin = 0, i_inc = 0;
  int8_t j, j_ini = 0, j_fin = 0, j_inc = 0;
  int8_t posCel;
  int8_t posCelDest = 0;

  //Se verifica si es la primera vez que se itera la logica del juego
  if (bJuegoNuevo) {
    //De ser asi, al inicio se crea una celda
    crearCelda();
    //Acto seguido se programa la creacion de la siguiente celda, lo que causara un evento de animacion
    estadoJuego = EJ_CREAR;
    bCrear = true;

    //Luego se prepara la pantalla de juego
    dibujarTableroJuegoNuevo();

    //Finalmente se desmarca la bandera
    bJuegoNuevo = false;
  }

  switch (estadoJuego) {
    case EJ_INACTIVO:
      //En este estado no se hace nada. Reservado para acciones que ocurren solas, sin entrada de usuario
      break;
    case EJ_DESPLAZAR:
      //La logica de juego no se actualiza mientras se hace una animacion, asi se da tiempo de que las
      //animaciones terminen entre pasos de logica.
      if (!bAnimando) {
        //Las siguientes constantes establecen los parametros de iteracion y las posiciones relativas de
        //las celdas de destino hacia donde se hace el movimiento
        switch (dirMovimiento) {
          case DM_IZQ:
            i_ini = 1; i_fin = 4; i_inc = 1;    //Se iteran solo las 3 columnas de la derecha desde la izquierda
            j_ini = 0; j_fin = 4; j_inc = 1;    //Se iteran todas las filas
            posCelDest = posCelIzq;             //La celda de destino es hacia la izquierda
            break;
          case DM_DER:
            i_ini = 2; i_fin = -1; i_inc = -1;  //Se iteran solo las 3 columnas de la izquierda desde la derecha
            j_ini = 0; j_fin = 4; j_inc = 1;    //Se iteran todas las filas
            posCelDest = posCelDer;             //La celda de destino es hacia la derecha
            break;
          case DM_ARR:
            i_ini = 0; i_fin = 4; i_inc = 1;    //Se iteran todas las columnas
            j_ini = 1; j_fin = 4; j_inc = 1;    //Se iteran solo las 3 filas de abajo desde arriba
            posCelDest = posCelArr;             //La celda de destino es hacia arriba
            break;
          case DM_ABA:
            i_ini = 0; i_fin = 4; i_inc = 1;    //Se iteran todas las columnas
            j_ini = 2; j_fin = -1; j_inc = -1;  //Se iteran solo las 3 filas de arriba desde abajo
            posCelDest = posCelAba;             //La celda de destino es hacia abajo
            break;
        }

        //Se verifica si una consolidacion esta pendiente
        if (bConsolidar) {
          //De estar pendiente se desplazan todas las celdas en el arreglo valorCelda, asimismo se
          //procesan todas las posibles combinaciones

          //Se recorre el arreglo de acuerdo a los parametros de iteracion establecidos segun la
          //direccion de movimiento
          for (i = i_ini; i != i_fin; i += i_inc) {
            for (j = j_ini; j != j_fin; j += j_inc) {
              //Se calcula la posicion de la celda de origen en el arreglo
              posCel = j * 4 + i;

              //Se verifica si la celda de origen esta marcada para desplazamiento
              if (celdaDesplazada[posCel]) {
                //Si esta marcada, se verifica si la celda de destino esta ocupada
                if (!valorCelda[posCel + posCelDest])
                  //Si la celda de destino esta libre, la de origen la suplanta
                  valorCelda[posCel + posCelDest] = valorCelda[posCel];
                else {
                  //Si la celda de destino esta ocupada, se combinan. Notese que los valores del
                  //arreglo en si representan las potencias de 2 de los numeros que albergan, asi
                  //que para duplicar el valor de la celda solo se incrementa la potencia
                  valorCelda[posCel + posCelDest]++;

                  //Luego se marca la celda de destino como combinada y se marca la bandera de
                  //combinacion para animar posteriormente
                  celdaCombinada[posCel + posCelDest] = true;
                  bCombinar = true;
                }

                //La celda ya se desplazo, asi que se desmarca
                celdaDesplazada[posCel] = false;

                //Asimismo su ubicacion ahora esta libre
                valorCelda[posCel] = 0;
              }
            }
          }

          //Tras terminar la consolidacion se desmarca la bandera
          bConsolidar = false;
        }

        //A continuacion se verifica si es posible comenzar o continuar moviendo celdas en la direccion
        //seleccionada

        //Se recorre el arreglo de acuerdo a los parametros de iteracion establecidos segun la direccion
        //de movimiento
        for (i = i_ini; i != i_fin; i += i_inc) {
          for (j = j_ini; j != j_fin; j += j_inc) {
            //Se calcula la posicion de la celda de origen en el arreglo
            posCel = j * 4 + i;

            //Se determina si la celda de origen se puede desplazar. Para que ocurra, la misma debe estar
            //ocupada y su destino debe cumplir cualquiera de 3 condiciones:
            // - La posicion de destino esta libre
            // - La posicion de destino esta por moverse (esta marcada para desplazamiento), asi se moveran a la vez
            // - La posicion de destino tiene el mismo valor que la de origen (se combinaran), pero ninguna de las
            //   dos celdas esta marcada como combinada previamente
            if (valorCelda[posCel] && (!valorCelda[posCel + posCelDest] || celdaDesplazada[posCel + posCelDest] ||
                                       (valorCelda[posCel + posCelDest] == valorCelda[posCel] && !celdaCombinada[posCel] &&
                                        !celdaCombinada[posCel + posCelDest])))
            {
              //Si se puede desplazar, se marca para desplazamiento
              celdaDesplazada[posCel] = true;
              //Asimismo debera consolidarse este cambio en el arreglo tras terminar la animacion
              bConsolidar = true;
            }
            else
              //Si la celda no puede desplazarse, simplemente se desmarca
              celdaDesplazada[posCel] = false;
          }
        }

        //Se determina el siguiente proceso en base a las banderas
        if (bConsolidar) {
          //Si en este proceso se programo una consolidacion (se desplazo al menos una celda), se activa
          //la bandera de animacion
          bAnimando = true;

          //Al mismo tiempo se activa la bandera de creacion, ya que mover celdas implica que se creara otra
          bCrear = true;
        }
        else if (bCombinar)
          //Sino se desplazaron celdas, pero se marco la bandera de combinacion, se pasa a ese estado
          estadoJuego = EJ_COMBINAR;
        else if (bCrear)
          //Si no se marco la bandera de combinacion, pero si la de crear, se creara una celda nueva
          estadoJuego = EJ_CREAR;
        else
          //Si no se desplazan celdas, ni se combinan, ni se crean, entonces no se pudo hacer nada,
          //asi que se pasa a inactividad
          estadoJuego = EJ_INACTIVO;
      }

      break;
    case EJ_COMBINAR:
      //La logica de juego espera a que las animaciones terminen
      if (!bAnimando) {
        //Si no se esta animando, se verifica si esta activada la bandera de combinar
        if (bCombinar) {
          //Si esta activada, se realizara la animacion
          bAnimando = true;

          //Acto seguido se desmarca la bandera de combinar
          bCombinar = false;
        }
        else {
          //Si la bandera de combinar esta desactivada, se limpian todas las celdas marcadas
          //para combinar
          for (i = 0; i < 16; i++)
            celdaCombinada[i] = false;

          //A continuacion se crea una nueva celda
          estadoJuego = EJ_CREAR;
        }
      }

      break;
    case EJ_CREAR:
      //La logica de juego espera a que las animaciones terminen
      if (!bAnimando) {
        //Si no se esta animando, se verifica si esta activada la bandera de crear
        if (bCrear) {
          //Si esta activada, se procedera a crear una celda nueva en un espacio vacio
          crearCelda();

          //Se marca la bandera de animacion
          bAnimando = true;

          //Se desmarca la bandera de crear
          bCrear = false;
        }
        else {
          //Si la bandera de crear esta desactivada, se limpian todas las celdas marcadas
          //para crear
          for (i = 0; i < 16; i++)
            celdaCreada[i] = false;

          //A continuacion se pasa a inactividad, pues crear celdas es siempre el ultimo paso
          estadoJuego = EJ_INACTIVO;
        }
      }

      break;
  }

  //Como paso siguiente de la logica, se efectuan las animaciones
  hacerAnimacion();
}

//----------------------------------------------------------------------------------------------------

static void reiniciarJuego() {
  uint8_t i;

  //Se limpian todos los arreglos que contienen los datos del tablero
  for (i = 0; i < 16; i++) {
    valorCelda[i] = 0;
    celdaDesplazada[i] = 0;
    celdaCombinada[i] = 0;
    celdaCreada[i] = 0;
  }

  //Limpia todas las variables del automata finito del juego
  bJuegoNuevo = true;
  estadoJuego = EJ_INACTIVO;
  bConsolidar = false;
  bCombinar = false;
  bCrear = false;
  bAnimando = false;
  pasoAnimacion = 0;
}

//----------------------------------------------------------------------------------------------------

static void hacerAnimacion() {
  int8_t i, i_ini = 0, i_fin = 0, i_inc = 0;
  int8_t j, j_ini = 0, j_fin = 0, j_inc = 0;
  uint8_t posCel;
  uint16_t posX, posY;
  uint16_t offsetPos;
  uint32_t posSprDib;
  uint32_t posSprBor;

  switch (estadoJuego) {
    case EJ_INACTIVO:
      //De momento no se hace nada, pero se reserva para animaciones latentes mientras no ocurre entrada de usuario
      break;
    case EJ_DESPLAZAR:
      //En este estado se desplazan las celdas una a una

      //Las siguientes constantes establecen los parametros de iteracion para controlar el orden de
      //dibujado en las siguientes rutinas
      switch (dirMovimiento) {
        case DM_IZQ:
          i_ini = 0; i_fin = 4; i_inc = 1;    //Al mover hacia la izquierda se dibuja de izquierda a derecha
          j_ini = 0; j_fin = 4; j_inc = 1;
          break;
        case DM_DER:
          i_ini = 3; i_fin = -1; i_inc = -1;  //Al mover hacia la derecha se dibuja de derecha a izquierda
          j_ini = 0; j_fin = 4; j_inc = 1;
          break;
        case DM_ARR:
          i_ini = 0; i_fin = 4; i_inc = 1;    //Al mover hacia arriba se dibuja de arriba para abajo
          j_ini = 0; j_fin = 4; j_inc = 1;
          break;
        case DM_ABA:
          i_ini = 0; i_fin = 4; i_inc = 1;    //Al mover hacia abajo se dibuja de abajo hacia arriba
          j_ini = 3; j_fin = -1; j_inc = -1;
          break;
      }

      for (i = i_ini; i != i_fin; i += i_inc) {
        for (j = j_ini; j != j_fin; j += j_inc) {
          //Pre calcula las variables con que se operaran en la iteracion
          posCel = j * 4 + i;                       //Indice de la celda que se esta desplazando
          posX = i * 60 + 6;                        //Posicion en X de la celda (sin desplazar)
          posY = j * 60 + 6;                        //Posicion en Y de la celda (sin desplazar)
          offsetPos = (pasoAnimacion + 1) * 15;     //Desplazamiento en pixeles
          posSprDib = valorCelda[posCel] * 48 * 48; //Posicion de inicio del sprite a dibujar

          //Solo se animan aquellas celdas que estan marcadas para desplazamiento
          if (celdaDesplazada[posCel]) {
            //Se dibuja la celda desplazada de acuerdo a la direccion de movimiento
            switch (dirMovimiento) {
              case DM_IZQ:
                //Se calcula el indice del sprite usado para borrar
                posSprBor = pasoAnimacion * 15 * 48;
                //Coloca el sprite de la celda en una posicion desplazada
                lcd.blitImg(posX - offsetPos, posY, 48, 48, &spr_numeros[posSprDib]);
                //Borra el rastro dejado por el sprite redibujando un segmento del fondo
                lcd.blitImg(posX - offsetPos + 48, posY, 15, 48, &spr_borrado_hr[posSprBor]);
                break;
              case DM_DER:
                posSprBor = (pasoAnimacion + 4) * 15 * 48;
                lcd.blitImg(posX + offsetPos, posY, 48, 48, &spr_numeros[posSprDib]);
                lcd.blitImg(posX + offsetPos - 15, posY, 15, 48, &spr_borrado_hr[posSprBor]);
                break;
              case DM_ARR:
                posSprBor = pasoAnimacion * 48 * 15;
                lcd.blitImg(posX, posY - offsetPos, 48, 48, &spr_numeros[posSprDib]);
                lcd.blitImg(posX, posY - offsetPos + 48, 48, 15, &spr_borrado_vr[posSprBor]);
                break;
              case DM_ABA:
                posSprBor = (pasoAnimacion + 4) * 48 * 15;
                lcd.blitImg(posX, posY + offsetPos, 48, 48, &spr_numeros[posSprDib]);
                lcd.blitImg(posX, posY + offsetPos - 15, 48, 15, &spr_borrado_vr[posSprBor]);
                break;
            }
          }
        }
      }

      //Se avanza la animacion un paso
      pasoAnimacion++;
      if (pasoAnimacion >= 4) {
        //Si la animacion alcanzo la cuenta total de pasos se termina la misma
        bAnimando = false;
        pasoAnimacion = 0;  //Se limpia la variable para la proxima vez
      }

      break;
    case EJ_COMBINAR:
      //En esta etapa se animan las celdas una a una
      for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
          //Pre calcula las variables con que se operaran en la iteracion
          posCel = j * 4 + i; //Indice de la celda que se esta animando
          posX = i * 60 + 6;                        //Posicion en X de la celda animada
          posY = j * 60 + 6;                        //Posicion en Y de la celda animada
          posSprDib = valorCelda[posCel] * 48 * 48; //Posicion de inicio del sprite a dibujar

          //Solo se animan aquellas celdas que estan marcadas para combinacion
          if (celdaCombinada[posCel]) {
            //La animacion simplemente dibuja sprites en diferentes niveles de zoom. Notese que para
            //el zoom mas pequeño se uza el mismo sprite gris (visualmente no se percibe)
            switch (pasoAnimacion) {
              case 0: lcd.blitImg(posX, posY, 48, 48, &spr_numeros_z1[posSprDib - 48 * 48]); break;
              case 1: lcd.blitImg(posX, posY, 48, 48, &spr_numeros_z2[0]);                   break;
              case 2: lcd.blitImg(posX, posY, 48, 48, &spr_numeros_z1[posSprDib]);           break;
              case 3: lcd.blitImg(posX, posY, 48, 48, &spr_numeros[posSprDib]);              break;
            }
          }
        }
      }

      //Se incrementa la cuenta de pasos de animacion
      pasoAnimacion++;
      if (pasoAnimacion >= 4) {
        //Cuando la cuenta llega al final, la animacion se detiene
        bAnimando = false;
        pasoAnimacion = 0;
      }

      break;
    case EJ_CREAR:
      //En esta etapa se animan las celdas una a una
      for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
          //Pre calcula las variables con que se operaran en la iteracion
          posCel = j * 4 + i;                       //Indice de la celda que se esta animando
          posX = i * 60 + 6;                        //Posicion en X de la celda animada
          posY = j * 60 + 6;                        //Posicion en Y de la celda animada
          posSprDib = valorCelda[posCel] * 48 * 48; //Posicion de inicio del sprite a dibujar

          //Solo se animan aquellas celdas que esten marcadas para animacion
          if (celdaCreada[posCel]) {
            //La animacion simplemente dibuja sprites en diferentes niveles de zoom. Notese que para
            //el zoom mas pequeño se uza el mismo sprite gris (visualmente no se percibe)
            switch (pasoAnimacion/2) {
              case 0: lcd.blitImg(posX, posY, 48, 48, &spr_numeros_z2[0]);         break;
              case 1: lcd.blitImg(posX, posY, 48, 48, &spr_numeros_z1[posSprDib]); break;
              case 2: lcd.blitImg(posX, posY, 48, 48, &spr_numeros[posSprDib]);    break;
            }
          }
        }
      }

      //Incrementa la cuenta de pasos de animacion
      pasoAnimacion++;
      if (pasoAnimacion >= 6) {
        //Si la cuenta llega al final, se termina la animacion
        bAnimando = false;
        pasoAnimacion = 0;
      }

      break;
  }
}

//----------------------------------------------------------------------------------------------------

static void crearCelda() {
  uint8_t i;
  uint8_t nCeldasVacias;
  uint8_t posNuevaCelda;

  //Primero se cuenta la cantidad de celdas vacias
  nCeldasVacias = 0;
  for (i = 0; i < 16; i++)
    if (!valorCelda[i])
      nCeldasVacias++;

  //Se elige un numero al azar segun la cantidad de celdas vacias
  posNuevaCelda = random(nCeldasVacias);

  //Se recorre el tablero buscando la celda vacia indicada por el numero
  //generado al azar
  nCeldasVacias = 0;
  for (i = 0; i < 16; i++) {
    //Si la celda esta ocupada se ignora, solo cuentan las vacias
    if (!valorCelda[i]) {
      if (nCeldasVacias == posNuevaCelda) {
        //Si se esta en la posicion de celda vacia indicada por el numero
        //aleatorio, se marca la misma para creacion y se le asigna un 2
        //o 4 al azar (en realidad un 1 o un 2, dado que se usan potencias)
        celdaCreada[i] = true;
        //La probabilidad del 2 es mas alta que para el 4. En el juego
        //original la probabilidad de un 4 es de 10%, mientras que para 2 es 90%
        valorCelda[i] = (random(10) == 0) ? 2 : 1;
        return;
      }
      else
        //Si aun no se llega a la posicion, simplemente incrementa la cantidad
        //de celdas vacias recorridas
        nCeldasVacias++;
    }
  }
}

//----------------------------------------------------------------------------------------------------

static void dibujarTableroJuegoNuevo() {
  uint8_t i, j;
  uint16_t posX, posY;

  //Limpia de blanco el area de juego
  lcd.blitSol(0,   0, 240, 60, COLOR_RGB(31, 63, 31));
  lcd.blitSol(0,  60, 240, 60, COLOR_RGB(31, 63, 31));
  lcd.blitSol(0, 120, 240, 60, COLOR_RGB(31, 63, 31));
  lcd.blitSol(0, 180, 240, 60, COLOR_RGB(31, 63, 31));

  //Dibuja los marcadores de posicion de las celdas
  for (j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++) {
      posX = i * 60 + 6; //Posicion en X de la celda
      posY = j * 60 + 6; //Posicion en Y de la celda
      lcd.blitImg(posX, posY, 48, 48, &spr_numeros[0]);
    }
  }

  //Limpia de amarillo el area no usada
  lcd.blitSol(0, 240, 240, 40, COLOR_RGB(31, 63, 16));
  lcd.blitSol(0, 280, 240, 40, COLOR_RGB(31, 63, 16));
}

//----------------------------------------------------------------------------------------------------
/*
  static void dibujarTablero() {
  uint8_t i, j;
  uint8_t posCel;
  uint16_t posX, posY;
  uint32_t posSpr;

  for (j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++) {
      posCel = j * 4 + i;       //Indice de la celda que se esta dibujando
      if (valorCelda[posCel] > 0) {
        posX = i * 60 + 6;      //Posicion en X de la celda
        posY = j * 60 + 6;      //Posicion en Y de la celda
        posSpr = valorCelda[posCel] * 48 * 48;
        lcd.blitImg(posX, posY, 48, 48, &spr_numeros[posSpr]);
      }
    }
  }
  }
*/
//----------------------------------------------------------------------------------------------------
/*
  static void imprimirTablero() {
  uint8_t i, j;

  for (j = 0; j < 4; j++) {
    for (i = 0; i < 4; i++) {
      Serial.print(valorCelda[j * 4 + i]);
      Serial.print(" ");
    }
    Serial.print("| ");
    for (i = 0; i < 4; i++) {
      Serial.print(celdaDesplazada[j * 4 + i]);
      Serial.print(" ");
    }
    Serial.println();
  }
  }
*/
