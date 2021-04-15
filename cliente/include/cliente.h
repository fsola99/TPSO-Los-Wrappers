#ifndef CLIENTE_H_
#define CLIENTE_H_

#include "sockets.h"
#include "consola.h"
#include<commons/string.h>
#include<commons/error.h>
#include <signal.h>

#define CONFIG_PATH "./cfg/cliente.config"
#define CONEXIONES_PATH "../shared/cfg/conexiones.config"
t_log *logger;
int socket_servidor;
uint32_t modulo;

void* mensajes_retorno();
void cerrar();


#endif /* CLIENTE_H_ */