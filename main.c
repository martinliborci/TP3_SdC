#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define TEMP_REG_FILE	"tempReg.txt"
#define HUM_REG_FILE  	"humReg.txt"
#define BUFSIZE 		512

void main(int argc, char const *argv[])
{
	
	int fd_TempReg, fd_Humedad;
	char buffer[BUFSIZE];
	int  nChars;

	// Registro de Temperatura
	// tempReg.txt: nombre del archivo de registro de temperatura
	// O_WRONLY: sólo escritura
	// O_CREAT: si no existe se crea
	// O_TRUNC: si el archivo ya existe lo trunca a longitud cero
	// 0751: propietario RWX, grupo RX, otros X
	fd_TempReg = open(TEMP_REG_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0751);
	if (fd_TempReg < 0){
		close(fd_TempReg);
		perror("Error en la apertura del archivo de registro de temperatura");
		exit(1);
	}

	// Registro de Humedad
	fd_Humedad = open(HUM_REG_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0751);
	if (fd_Humedad < 0){
		close(fd_Humedad);
		perror("Error en la apertura del archivo de registro de humedad");
		exit(1);
	}
	
	int i;
	float nRand;
	srand (time(NULL));
	for ( i= 10; i ; i-- ){
		
		// rand() % 100: n° aleatorio decimal entre 0 y 100
		nRand = rand() % 100;
		nChars = sprintf(buffer, "%1.2f ", nRand);
		if (write(fd_TempReg, buffer, nChars) < nChars){
			// write devuelve el # de bytes escritos y si es menor a los que se debieron escribir (n_read)
			// se considera que hubo un error.
			perror("Error en la escritura del archivo de registro de temperatura");
			close(fd_TempReg);
			exit(1);
		}

		nRand = rand() % 100;
		nChars = sprintf(buffer, "%1.2f ", nRand);
		if (write(fd_Humedad, buffer, nChars) < nChars){
			// write devuelve el # de bytes escritos y si es menor a los que se debieron escribir (n_read)
			// se considera que hubo un error.
			perror("Error en la escritura del archivo de registro de humedad");
			close(fd_Humedad);
			exit(1);
		}

		sleep(1); // Esperar 1 segundo

	}



}

/*
#define BUFSIZE 512
void main(int argc, char const *argv[])
{
	// Variables locales
	// Descriptor de entrada, salida y # de bytes leidos.
	int fd_in, fd_out, n_read;
	// Espacio en memoria para guardar lo leído
	char buffer[BUFSIZE];

	// Abre archivo de entrada
	fd_in = open(argv[1], O_RDONLY);
	if (fd_in < 0){
		perror("open source file");
		exit(1);
	}

	// Crea archivo de salida
	fd_out = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0751);
	if (fd_out <0){
		close(fd_out);
		perror("open destination file");
		exit(1);
	}

	// Lazo para transferir datos desde argv[1] a argv[2].
	while ((n_read = read(fd_in, buffer, BUFSIZE)) > 0) {
		// Lee BUFSIZE bytes del archivo vinculado al descriptor fd_in y los guarda en buffer.
		// n_read es la cantidad de bytes devueltos en la lectura. Puede ser menor que BUFSIZE 
		// también puede ser cero (EOF).-

		// printf("El numero de caracteres es= %d \nY el contenido es: %s \n",n_read, buffer);
		
		if (write(fd_out, buffer, n_read) < n_read){
			// write devuelve el # de bytes escritos y si es menor a los que se debieron escribir (n_read)
			// se considera que hubo un error.
			perror("write");
			close(fd_in);
			close(fd_out);
			exit(1);
		}
	}

	if (n_read < 0) {
		// Si el valor de bytes leídos en el read e -1, hugo algún error.
		perror("read");
		close(fd_in);
		close(fd_out);
		exit(0);
	}

}

*/