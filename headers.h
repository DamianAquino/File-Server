#ifndef HEADERS_H
#define HEADERS_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 4 + 4
#define ERR_LECTURA 5
#define MAX_SOLICITUD 50

#define SOLICITAR_DATOS 1
#define SUBIR_DATOS 2
#define DATOS 3
#define FINALIZA_ENVIO 4

typedef struct
{
    int fd;
    struct sockaddr_in address;
}Network_dir;


int set_root();
int crear_server(Network_dir*);
int set_cliente(int, Network_dir*, socklen_t*, int*, int*);
void SETDESCRIPTORES(int*, int, fd_set*);
void leer_datos(fd_set*, int*, int*);
void handler(int, char*);
void cerrar_conexion(fd_set*, int, int*, int*);
void logear(char* log);
struct tm* que_hora_es();

#endif // HEADERS_H
