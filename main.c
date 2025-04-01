#include "headers.h"

int main(int argc, char *argv[]) {
    Network_dir server_dir;
    Network_dir cliente_dir;

    if(crear_server(&server_dir))
        return -1;

    if(!set_root())
        return -2;

    int n = 0, descriptores[MAX_CLIENTS];
    socklen_t cliente_len = sizeof(cliente_dir.address);
    fd_set readfds;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_dir.fd, &readfds);
        SETDESCRIPTORES(descriptores, n, &readfds);
        select(MAX_CLIENTS, &readfds, NULL, NULL, NULL);

        // Verifica si se recibio una conexion o un cliente envio datos
        if(FD_ISSET(server_dir.fd, &readfds))
            set_cliente(server_dir.fd, &cliente_dir, &cliente_len, &n, descriptores);

        else
            leer_datos(&readfds, descriptores, &n);
    }

    close(server_dir.fd);
    return 0;
}
