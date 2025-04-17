#ifndef HEADERS_H
#define HEADERS_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>

#define PORT 8080
#define MAX_CLIENTS 4 + 4
#define ERR_LECTURA 5
#define MAX_SOLICITUD 50
#define ARCHIVO_NO_ENCONTRADO "ARCHIVO_NO_ENCONTRADO"

#define SOLICITUD '1'
#define SUBIDA '2'
#define LISTADO '3'
#define FINALIZA_ENVIO '4'
#define ERROR '5'
#define DATOS '6'

typedef struct{
    int fd;
    struct sockaddr_in address;
}Network_dir;

typedef struct{
    int flag;
    int tam;
    char* data;
}Data;

typedef struct{
    char op;
    int bytes;
    char* buffer;
}Cabecera;

int set_root();
int crear_server(Network_dir*);
int set_cliente(int, Network_dir*, socklen_t*, int*, int*);
void set_fds(int*, int, fd_set*);
void leer_datos(fd_set*, int*, int*);
void handler(int, char*);
void cerrar_conexion(fd_set*, int, int*, int*);
void logear(char*);
struct tm* que_hora_es();
void enviar(int,char*);
void recibir(int,char*);
void listar(int,char*);

#endif // HEADERS_H
