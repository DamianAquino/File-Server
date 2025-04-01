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

    if (chdir(new_dir) != 0)
        return 1;

    // Verificar el directorio de trabajo actual
    char cwd[5];
    if (getcwd(cwd, sizeof(cwd)) == new_dir)
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

int set_cliente(int server_fd, Network_dir* cliente_dir, socklen_t* cliente_len, int* n, int* descriptores)
{
    cliente_dir->fd = accept(server_fd, (struct sockaddr *)&cliente_dir->address, cliente_len);
    struct tm* hora = que_hora_es();
    char log[100];

    sprintf(log, "%02d:%02d:%02d CONECTADO: %d", hora->tm_hour, hora->tm_min, hora->tm_sec, cliente_dir->fd);
    logear(log);

    (*n) ++;
    printf("%d", *n);
    descriptores[*n] = cliente_dir->fd;

    return 0;
}

struct tm* que_hora_es(){
    time_t tiempo_actual;
    struct tm *tiempo_info;

    time(&tiempo_actual);
    tiempo_info = localtime(&tiempo_actual);

    return tiempo_info;
}

void SETDESCRIPTORES(int* descriptores, int cant, fd_set* readfds)
{
    for(int i = 1; i <= cant; i++)
    {
        if(descriptores[i] != 0)
            FD_SET(descriptores[i], readfds);
    }
}

void cerrar_conexion(fd_set* readfds, int descriptor, int* descriptores, int* cant)
{
    FD_CLR(descriptor, readfds);
    close(descriptor);
    char log[30];
    struct tm* hora = que_hora_es();

    for(int i = 1; i <= *cant; i++)
    {
        if(descriptores[i] == descriptor)
        {
            (*cant)--;
            sprintf(log, "%02d:%02d:%02d DESCONECTADO: %d", hora->tm_hour, hora->tm_min, hora->tm_sec, descriptor);
            logear(log);
            descriptores[i] = 0;
        }
    }
}

void leer_datos(fd_set* readfds, int* descriptores, int* cant)
{
    int bytesRead;
    char solicitud[MAX_SOLICITUD];
    char log[100];
    struct tm* tiempo = que_hora_es();

    for(int i = 1; i <= *cant; i++)
    {
        if(descriptores[i] != 0)
        {
            if(FD_ISSET(descriptores[i], readfds))
            {
                bytesRead = read(descriptores[i], solicitud, sizeof(solicitud));
                if (bytesRead <= 0)
                    cerrar_conexion(readfds, descriptores[i], descriptores, cant);
                else {
                    solicitud[bytesRead - 2] = '\0';
                    sprintf(log, "%02d:%02d:%02d SOLICITUD: %s", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, solicitud);
                    logear(log);

                    handler(descriptores[i], solicitud);
                }
            }
        }
    }
}

void handler(int descriptor, char* solicitud)
{
    struct tm* tiempo = que_hora_es();
    char log[100];

    switch(solicitud[0])
    {
        case(1):
            puts("Enviardo %s to %d");
            //enviar(descriptor, solicitud, (strlen(solicitud) - 1));
            break;
        case(2):
            puts("reibir");
            //recibir(descriptor, solicitud, (strlen(solicitud) - 1));
            break;
        default:
            sprintf(log, "%02d:%02d:%02d ERROR %d %s", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, descriptor, solicitud);
            logear(log);
            break;
    }
}


