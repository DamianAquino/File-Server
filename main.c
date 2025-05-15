#include "headers.h"
//#include "funciones.c"
int main(void) {
    Network_dir server_fd;
    Network_dir cliente_fd;

    if(crear_server(&server_fd))
        return -1;

    if(!set_root())
        return -2;

    int n = 0, descriptores[MAX_FDS];
    socklen_t cliente_len = sizeof(cliente_fd.address);
    fd_set readfds;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_fd.fd, &readfds);
        set_fds(descriptores, n, &readfds);
        select(MAX_FDS, &readfds, NULL, NULL, NULL);

        // Verifica si se recibio una conexion o un cliente envio datos
        if(FD_ISSET(server_fd.fd, &readfds))
            set_cliente(server_fd.fd, &cliente_fd, &cliente_len, &n, descriptores);
        else
            leer_stream(&readfds, descriptores, &n);
    }

    close(server_fd.fd);
    return 0;
}
