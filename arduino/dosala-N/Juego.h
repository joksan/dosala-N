#ifndef Juego_h_Incluida
#define Juego_h_Incluida

//Enumeracion de los comandos de control de juego
enum COMANDO_CONTROL {
  CC_IZQUIERDA, CC_DERECHA, CC_ARRIBA, CC_ABAJO, CC_REINICIAR,
};

//Funciones exportadas por el modulo
void accion(COMANDO_CONTROL comando);
void hacerLogicaJuego();

#endif //Juego_h_Incluida
