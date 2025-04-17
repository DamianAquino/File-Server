#include "headers.h"

void logear(char* log)
{
    FILE * archivo = fopen("/home/damian/Escritorio/telecomunicaciones/Internet/File Server3/logs.txt", "a");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
    }
    fprintf(archivo, "%s\n", log);
    fclose(archivo);
}

int set_root()
{
    const char *new_dir = "/mnt";

    if(chdir(new_dir) != 0)
        return 1;

    // Verificar el directorio de trabajo actual
    char cwd[5];
    if(getcwd(cwd, sizeof(cwd)) == new_dir)
        return 0;
    else
        return 1;
}

int crear_server(Network_dir* server_dir){
    int opt = 1;
    server_dir->fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_dir->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    server_dir->address.sin_family = AF_INET;
    server_dir->address.sin_addr.s_addr = inet_addr("127.0.0.1"); ///address.sin_addr.s_addr = INADDR_ANY;  // Aceptar conexiones de cualquier Interfaz
    server_dir->address.sin_port = htons(PORT);

    bind(server_dir->fd, (struct sockaddr *)&server_dir->address, sizeof(server_dir->address));
    listen(server_dir->fd, MAX_CLIENTS);
    printf("Servidor escuchando en puerto %d...\n", PORT);

    return 0;
}

int set_cliente(int server_fd, Network_dir* cliente_dir, socklen_t* cliente_len, int* n, int* fds)
{
    cliente_dir->fd = accept(server_fd, (struct sockaddr *)&cliente_dir->address, cliente_len);
    struct tm* hora = que_hora_es();
    char log[100];

    sprintf(log, "%02d:%02d:%02d CONECTADO: %d", hora->tm_hour, hora->tm_min, hora->tm_sec, cliente_dir->fd);
    logear(log);

    (*n) ++;
    fds[*n] = cliente_dir->fd;

    return 0;
}

struct tm* que_hora_es(){
    time_t tiempo_actual;
    struct tm *tiempo_info;

    time(&tiempo_actual);
    tiempo_info = localtime(&tiempo_actual);

    return tiempo_info;
}

void set_fds(int* fds, int cant, fd_set* readfds)
{
    for(int i = 1; i <= cant; i++)
        if(fds[i] != 0)
            FD_SET(fds[i], readfds);
}

void cerrar_conexion(fd_set* readfds, int fd, int* fds, int* cant)
{
    FD_CLR(fd, readfds);
    close(fd);
    char log[30];
    struct tm* hora = que_hora_es();

    for(int i = 1; i <= *cant; i++)
    {
        if(fds[i] == fd)
        {
            (*cant)--;
            sprintf(log, "%02d:%02d:%02d DESCONECTADO: %d", hora->tm_hour, hora->tm_min, hora->tm_sec, fd);
            logear(log);
            fds[i] = 0;
        }
    }
}

void leer_datos(fd_set* readfds, int* fds, int* cant)
{
    int bytesRead;
    char solicitud[MAX_SOLICITUD];

    for(int i = 1; i <= *cant; i++)
    {
        if(fds[i] != 0)
        {
            if(FD_ISSET(fds[i], readfds))
            {
                bytesRead = recv(fds[i], solicitud, sizeof(solicitud), 0);
                if (bytesRead <= 0)
                    cerrar_conexion(readfds, fds[i], fds, cant);
                else {
                    solicitud[bytesRead] = '\0';
                    handler(fds[i], solicitud);
                }
            }
        }
    }
}

void handler(int fd, char* solicitud)
{
    struct tm* tiempo = que_hora_es();
    char log[100];
    char* env = "Enviando archivo...";
    char* err = "Error al recibir flag de operacion...";
    char* rec = "Recibiendo archivo...";

    switch(solicitud[0])
    {
        case '1':
            sprintf(log, "%02d:%02d:%02d FD:%d BAJADA : -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, solicitud);
            logear(log);
            enviar(fd, env);
            break;
        case '2':
            sprintf(log, "%02d:%02d:%02d FD:%d SUBIDA : -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, solicitud);
            logear(log);
            recibir(fd, rec);
            break;
        case '3':
            sprintf(log, "%02d:%02d:%02d FD:%d LISTADO : -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, solicitud);
            logear(log);
            listar(fd, solicitud);
            break;
        default:
            sprintf(log, "%02d:%02d:%02d FD:%d ERROR MSG:-%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, solicitud);
            logear(log);
            enviar(fd, err);
            break;
    }
}

void listar(int fd, char* carpeta)
{
    DIR* dir;
    struct dirent* buffer_dirent;
    struct tm* tiempo = que_hora_es();

    if((dir = opendir(carpeta)) == NULL)
    {
        int max_len_rta = 4 + strlen(ARCHIVO_NO_ENCONTRADO) + 1;
        char* rta = malloc(max_len_rta);
        snprintf(rta, max_len_rta, "%c|%d|%s", ERROR, (int)strlen(rta), ARCHIVO_NO_ENCONTRADO);
        send(fd, rta, strlen(rta), 0);

        /// LOG
        int max_len_log = 24 + (int)strlen(ARCHIVO_NO_ENCONTRADO) + 4;
        char* err = malloc(max_len_log);
        snprintf(err, max_len_log, "%02d:%02d:%02d ERROR FD:%d MSG: -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, ARCHIVO_NO_ENCONTRADO);
        logear(err);
    }

    else
    {
        char* buffer_nombres = malloc(1024);

        while((buffer_dirent = readdir(dir)) != NULL)
        {
           strcat(buffer_nombres, buffer_dirent->d_name);
           strcat(buffer_nombres, (char*)"\n");
        }
        char* buffer = malloc(strlen(buffer_nombres) + 7);

        snprintf(buffer, (int)strlen(buffer), "%c|%d|%s", DATOS, (int)strlen(buffer_nombres), buffer_nombres);
        write(fd, (void*)buffer, strlen(buffer));
        free(buffer);
        free(buffer_nombres);
    }
}

void enviar(int fd, char* archivo)
{/*
    char* buffer = (char*)malloc(2 + strlen(flag) + strlen(datos));
    sprintf(buffer, "%s|%ld|%s", flag, strlen(datos), datos);

    write(fd, buffer, strlen(buffer));
    free(buffer);
*/}

void recibir(int fd, char* archivo)
{

}
