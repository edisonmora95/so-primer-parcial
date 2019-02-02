Proyecto de Sistemas Operativos

El proyecto consta de 4 programas:
  *  Sensor, el cual escribe en bloques de memoria compartida valores de los sensores de distancia y giroscopio.
  *  Lector, el cual lee los valores de esos bloques de memoria y calcula la distancia.
  *  Printer, el cual utiliza las distancias creadas para verificar si es carro u obstáculo.
  *  Main, el cual crea y controla a los otros procesos.

Todos los procesos tienen sus archivos de configuración los cuales se encuentran en config
Estos archivos contienen los datos necesarios para correr cada programa, como por ejemplo:
  *  Valores de frecuencia
  *  Bloques de memoria compartida
  *  Direcciones de los programas a ejecutar (Solo para el main)
En el directorio config se encuentra un README indicando la organización de los config files de los programas.

Primero hay que compilar los programas
  *  make lectores
  *  make sensores
  *  make printer
  *  make main

Luego solo se ejecuta el main.


