#include "sockets.h"

// Funciones del cliente

int crear_conexion(char *ip, int port)
{
	struct addrinfo hints;
	struct addrinfo *server_info;
	char* puerto = string_itoa(port);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		{
			freeaddrinfo(server_info);
			free(puerto);
			return -1;
		}
		

	freeaddrinfo(server_info);
	free(puerto);

	return socket_cliente;
}

void terminar_programa(int socket, t_log* logger, t_config* config)
{

	if(logger != NULL)
		log_destroy(logger);

	if(config != NULL)
		config_destroy(config);
	
	close(socket);
	
	return;
}

// Funciones del servidor

int iniciar_servidor(char* ip, int port) {
	int socket_servidor;
	char* puerto = string_itoa(port);
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        int activado = 1;
        setsockopt(socket_servidor,SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado));

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }
	printf("Servidor corriendo correctamente\n");
	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

	free(puerto);

    //log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	printf("se conecto un cliente\n");

	return socket_cliente;
}
void terminar_servidor(int socket, t_log* logger, t_config* config)
{

	if(logger != NULL)
		log_destroy(logger);

	if(config != NULL)
		config_destroy(config);

	close(socket);
	return;
}

int handshake(int modulo_entrante, int socket)
{
	uint8_t modulo = modulo_entrante;
	send(socket,&modulo,sizeof(uint8_t),0);
	recv(socket,&modulo,sizeof(uint8_t),0);
	modulo_entrante = modulo;
	return modulo_entrante;
}