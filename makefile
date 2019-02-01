main: ./src/main.c
	gcc -o ./bin/main ./src/main.c

lectores: lectorL lectorR lectorC

lectorL: ./src/lectorL.c
	gcc -o ./bin/lectorL ./src/lectorL.c -lpthread -lm

lectorR: ./src/lectorR.c
	gcc -o ./bin/lectorR ./src/lectorR.c -lpthread -lm

lectorC: ./src/lectorC.c
	gcc -o ./bin/lectorC ./src/lectorC.c -lpthread -lm

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
