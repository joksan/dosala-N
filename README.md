# dosala-N
Juego portatil estilo "2048", con teensy, pantalla LCD y sensor de gestos.

Esta es una implementacion 100% portatil del juego 2048, inspirado en el famoso juego de Gabriele Cirulli:
https://gabrielecirulli.github.io/2048/

No esta demas señalar que el codigo operativo es de mi propia creacion, pero muchas gracias a Gabriele por ese genial concepto :-D.

Este mini juego puede jugarse de dos formas:
La primera es que envias mensajes a la teensy mediante la terminal serie de Arduino. Los siguientes mensajes de una letra realizan las siguientes acciones:
- 'w': Mueve las celdas hacia arriba.
- 'a': Mueve las celdas hacia la izquierda.
- 's': Mueve las celdas hacia abajo.
- 'd': Mueve las celdas hacia la derecha.
- 'r': Reinicia el juego.

La segunda forma es que usas el sensor de gestos de sparkfun (informacion mas abajo) para controlarlo:
- Desplazar la mano en cualquier direccion (gesto de deslizamiento) mueve las celdas en la misma direccion.
- Colocar la mano al frente del sensor y luego alejarla reinicia el juego.

Lamentablemente la memoria del teensy esta bastante agotada (mas del 75% usada) y no fue posible agregar mensajes de victoria ni derrota, asi que cuando te quedes sin movimientos simplemente reinicia el juego y cuando logres una celda con 2048 puedes celebrar \o/ y optar por seguir jugando o reiniciar.

Conexiones de circuito
----------------------
El diagrama de conexion es extremadamente simple y no vale la pena crear un esquematico, pero si vale la pena dar unas indicaciones generales:
- La pantalla LCD debe alimentarse de 5V (posee regulador de 3.3V integrado) y sus lineas de bus SPI deben conectarse a las lineas por defecto de la teensy.
- La linea de reset de la LCD debe conectarse al pin 14 de la teensy.
- Por si no es suficientemente obvio :-), el chip select de la LCD va al pin 10 de la teensy.
- En total la LCD solo requiere 7 lineas para operar (4 de SPI, 2 de suministro mas reset), el resto no se conectan.
- El sensor de gestos va alimentado a 3.3V (no posee regulador) y sus lineas de bus I2C deben conectarse tambien a las lineas por defecto de la teensy.
- El sensor de gestos requiere tan solo 4 lineas para operar (2 de I2C y 2 de suministro), el resto no se conectan (el pin de interrupcion no es usado por la libreria de sparkfun, muy a pesar que indican que se debe conectar).
- En general no se requieren convertidores de nivel. Tanto la LCD como el sensor de gestos operan a 3.3V al igual que la teensy.

Partes usadas y librerias
-------------------------
Tarjeta de microcontrolador teensy (use la 3.1, pero la 3.2 deberia funcionar):
- http://tienda.teubi.co/productos/ver/162
- https://www.pjrc.com/teensy/

Sensor de gestos de sparkfun:
- http://tienda.teubi.co/productos/ver/519
- https://www.sparkfun.com/products/12787

Liberia del sensor de gestos:
- https://github.com/sparkfun/SparkFun_APDS-9960_Sensor_Arduino_Library

Pantalla LCD (HY28A-LCDB con controlador ILI9320):
- http://tienda.teubi.co/productos/ver/242
- http://www.hotmcu.com/28-touch-screen-tft-lcd-with-spi-interface-p-42.html

La libreria de la LCD no existe para arduino ni teensy, pero yo mismo la cree y viene con el codigo :-).

Modificando las imagenes
------------------------
Si te interesa agregar texturas diferentes para las celdas del juego, puedes editar las imagenes con el editor de imagenes Gimp o crear las tuyas propias! Tan solo asegurate de respetar las dimensiones de las mismas.

Una vez tengas tus nuevas texturas, exportalas desde gimp usando el formato de codigo C (C source code), y en las opciones que aparecen haz los siguientes cambios:
- En "prefixed name" usa el mismo nombre del archivo (letras minusculas, separados con guiones bajos). Procura no cambiar los nombres de los archivos para no tener problemas con el codigo.
- "Save comment to file" debe estar apagado.
- "Use GLib types (guint8*)" debe estar apagado.
- "Use macros instead of struct" debe estar ENCENDIDO.
- "Use 1 byte Run-Length Encoding" debe estar apagado.
- "Save alpha channel (RGBA/RGB)" debe estar apagado.
- "Save as RGB565 (16-bit)" debe estar apagado.
- La opacidad dejala al 100%.

Nota: La version de gimp usada a la fecha de este escrito era la 2.8.10.

Luego, tras exportar las imagenes, corre el makefile incluido en el folder de imagenes usando el comando "make" (si no sabes que es un makefile, te recomiendo leer un tutorial de bash primero y de ahi estudiar make, veras es super simple). El makefile compilara el programa que lee las imagenes y acto seguido lo ejecutara, actualizando las imagenes directamente en el folder de sprites del sketch de Arduino.

En este momento puedes correr "make clean" si quieres eliminar el programa convertidor (siempre puedes volver a generarlo).

Todo listo! ahora desde Arduino IDE sube el sketch a tu teensy y prueba tus nuevas texturas.

Si tienes un juego de texturas genial que te gustaria compartir, acepto pull requests :-). En cuyo caso asegurate de colocar tus imagenes en un directorio separado. La idea es correr make desde cada folder con imagenes para que actualice las imagenes en el sketch y asi solo se suben.

Licencias
---------
Todo el codigo esta publicado bajo licencia GPL version 3 (ver archivo adjunto). Las imagenes estan bajo creative commons Reconocimiento-CompartirIgual version 4.0: https://creativecommons.org/licenses/by-sa/4.0/deed.es_ES
