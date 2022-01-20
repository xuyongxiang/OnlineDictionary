server = ser_func.o serve.o
ser:$(server)
	gcc $(server) -o ser -Wall -pthread -lsqlite3
.PYTHON:clean
clean:
	rm $(server) ser 
