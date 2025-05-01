#include "headers.h"

void logear(char* log)
{
    FILE * archivo = fopen("/home/damian/Escritorio/telecomunicaciones/Internet/File Server3/logs.txt", "a");

    if (archivo == NULL)
        perror("Error al abrir el archivo");

    fprintf(archivo, "%s\n", log);
    fclose(archivo);
}

int set_root()
{
    const char *raiz = "/";

    /// Cambia la carpets
    if(chdir(raiz) != 0)
        return 1;

    /// Verificar la carpeta actual
    char cwd[5];
    if(getcwd(cwd, sizeof(cwd)) == raiz)
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
        if(fds[i] == fd)
        {
            (*cant)--;
            sprintf(log, "%02d:%02d:%02d DESCONECTADO: %d", hora->tm_hour, hora->tm_min, hora->tm_sec, fd);
            logear(log);
            fds[i] = 0;
        }
}

void leer_stream(fd_set* readfds, int* fds, int* cant)
{
    int bytesRead;
    char solicitud[MAX_SOLICITUD];

    for(int i = 1; i <= *cant; i++)
        if(fds[i] != 0)
            if(FD_ISSET(fds[i], readfds))
            {
                bytesRead = recv(fds[i], solicitud, MAX_SOLICITUD, 0);
                if (bytesRead <= 0)
                    cerrar_conexion(readfds, fds[i], fds, cant);
                else {
                    solicitud[bytesRead] = '\0';
                    handler(fds[i], solicitud);
                }
            }
}

void formatear_paquete(char* paquete, Solicitud* solicitud)
{
    int cont = 2;
    char* num = (char*)malloc(5);
    num[0] = '\0';

    while(paquete[cont] != '|')
    {
        num[cont - 2] = paquete[cont];
        cont ++;
        //num[cont - 2] = '\0';
    }

    num[cont - 2] = '\0';

    solicitud->op = paquete[0];
    solicitud->bytes = atoi(num);

    if(solicitud->bytes < 100)
    {
        solicitud->buffer = (char*)malloc(solicitud->bytes + 1);
        strncpy(solicitud->buffer, (paquete + cont + 1), solicitud->bytes);
        solicitud->buffer[solicitud->bytes] = '\0';

        char* buffer = malloc(100);
        logear(buffer);
        free(buffer);
    }
    else
        solicitud->op = 0;

    free(num);
}

void handler(int fd, char* paquete)
{
    struct tm* tiempo = que_hora_es();
    char log[100];
    char* rec = "Recibiendo archivo...";

    Solicitud solicitud;

    formatear_paquete(paquete, &solicitud);

    switch(solicitud.op)
    {
        case SOLICITUD:
            sprintf(log, "%02d:%02d:%02d FD:%d BAJADA : -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, paquete);
            logear(log);
            enviar(fd, &solicitud);
            break;
        case SUBIDA:
            sprintf(log, "%02d:%02d:%02d FD:%d SUBIDA : -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, paquete);
            logear(log);
            recibir(fd, rec);
            break;
        case LISTADO:
            sprintf(log, "%02d:%02d:%02d FD:%d LISTADO : -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, paquete);
            logear(log);
            listar(fd, solicitud.buffer);
            break;
        default:
            sprintf(log, "%02d:%02d:%02d FD:%d ERROR MSG: -SOLICITUD INVALIDA.-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd);
            logear(log);
            //enviar(fd, err);
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
        send(fd, CARPETA_NO_ENCONTRADA, strlen(CARPETA_NO_ENCONTRADA), 0);

        /// LOG
        char* err = malloc((int)strlen(CARPETA_NO_ENCONTRADA));
        snprintf(err, 28 + (int)strlen(CARPETA_NO_ENCONTRADA), "%02d:%02d:%02d ERROR FD:%d MSG: -%s-", tiempo->tm_hour, tiempo->tm_min, tiempo->tm_sec, fd, CARPETA_NO_ENCONTRADA);
        logear(err);
        free(err);
    }
    else
    {
        char* buffer_nombres = malloc(1024);
        buffer_nombres[0] = '\0';

        while((buffer_dirent = readdir(dir)) != NULL)
        {
           strcat(buffer_nombres, buffer_dirent->d_name);
           strcat(buffer_nombres, (char*)"\n");
        }
        buffer_nombres[(int)strlen(buffer_nombres)] = '\0';

        char* buffer = malloc((int)strlen(buffer_nombres) + 7);

        snprintf(buffer, 1024, "%c|%d|%s", DATOS, (int)strlen(buffer_nombres), buffer_nombres);
        write(fd, buffer, strlen(buffer));
        free(buffer);
        free(buffer_nombres);
    }
}

void enviar(int fd, Solicitud* solicitud)
{
    FILE * arch = fopen(solicitud->buffer, "r");
    if(!arch)
    {
        send(fd, ARCHIVO_NO_ENCONTRADO, strlen(ARCHIVO_NO_ENCONTRADO), 0);
        return;
    }

    free(solicitud->buffer);

    fseek(arch, 0 , SEEK_END);
    long tam_arch = ftell(arch);
    fseek(arch, 0 , SEEK_SET);

    char* paquete = malloc(1032);
    solicitud->buffer = malloc(tam_arch + 1);
    solicitud->buffer[0] = '\0';
    solicitud->op = 6;
    solicitud->bytes = (size_t)tam_arch;

    size_t bytes_leidos = fread(solicitud->buffer, 1, 1024, arch);

    while (!feof(arch)) {
        solicitud->buffer += bytes_leidos;
        bytes_leidos = fread(solicitud->buffer, 1, 1024, arch);
    }

    solicitud->buffer[tam_arch - 1] = '\0';

    int long_paquete = 7 + (int)strlen(solicitud->buffer);

    snprintf(paquete, long_paquete, "%c|%d|%s", DATOS, (int)strlen(solicitud->buffer), solicitud->buffer);

    send(fd, paquete, strlen(paquete), 0);

    free(solicitud->buffer);
    free(paquete);
}

void recibir(int fd, char* archivo)
{

}
