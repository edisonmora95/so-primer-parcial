main: ./src/main.c
	gcc -o ./bin/main ./src/main.c

lector_sensores: ./src/lector_sensores.c
	gcc -o ./bin/lector_sensores ./src/lector_sensores.c -lpthread -lm

sensores: sensorL sensorR sensorC

sensorL: ./src/sensorL.c
	gcc -o ./bin/sensorL ./src/sensorL.c -lm

sensorR: ./src/sensorR.c
	gcc -o ./bin/sensorR ./src/sensorR.c -lm

sensorC: ./src/sensorC.c
	gcc -o ./bin/sensorC ./src/sensorC.c -lm

printer: ./src/printer.c
	gcc -o ./bin/printer ./src/printer.c -lm

clean:
	rm -rf ./bin/*
