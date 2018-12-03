main: ./src/main.c
	gcc -o ./bin/main ./src/main.c

lector_sensores: ./src/lector_sensores.c
	gcc -o ./bin/lector_sensores ./src/lector_sensores.c -lpthread -lm

sensores: ./src/sensorSO.c
	gcc -o ./bin/sensores ./src/sensorSO.c -lm

printer: ./src/printer.c
	gcc -o ./bin/printer ./src/printer.c -lm

clean:
	rm -rf ./bin/*
