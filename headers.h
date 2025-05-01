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
#define MAX_SOLICITUD 1024
#define ARCHIVO_NO_ENCONTRADO "5|21|ARCHIVO_NO_ENCONTRADO"
#define CARPETA_NO_ENCONTRADA "5|21|CARPETA_NO_ENCONTRADA"

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
    char op;
    size_t bytes;
    char* buffer;
}Solicitud;

int set_root();
int crear_server(Network_dir*);
int set_cliente(int, Network_dir*, socklen_t*, int*, int*);
void set_fds(int*, int, fd_set*);
void leer_stream(fd_set*, int*, int*);
void handler(int, char*);
void cerrar_conexion(fd_set*, int, int*, int*);
void logear(char*);
struct tm* que_hora_es();
void enviar(int,Solicitud*);
void recibir(int,char*);
void listar(int,char*);
void formatear_paquete(char*, Solicitud*);

#endif // HEADERS_H
