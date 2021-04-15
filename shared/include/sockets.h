#ifndef SOCKETS_H
#define SOCKETS_H

#include "config.h"
#include "logger.h"
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/string.h>
#include<unistd.h>
#include<commons/error.h>



int iniciar_servidor(char*,int);
int esperar_cliente(int);
int crear_conexion(char*, int);
void terminar_programa(int socket, t_log* logger, t_config* config);
void terminar_servidor(int socket, t_log* logger, t_config* config);
int handshake(int modulo_entrant, int socket);

#endif /* SOCKETS_H */