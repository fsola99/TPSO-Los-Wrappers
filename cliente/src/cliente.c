#include "cliente.h"

int main(int argc, char **argv)
{
	signal(SIGINT, cerrar);
	//Leemos la config e iniciamos el logger.

	t_config *config;
	t_config *conexiones_config;
	config = leer_config(CONFIG_PATH);
	conexiones_config = leer_config(CONEXIONES_PATH);
	//ID
	char *ID = config_get_string_value(config, "ID_CLIENTE");

	char *logger_path = string_new();
	string_append(&logger_path, config_get_string_value(config, "ARCHIVO_LOG"));
	string_append(&logger_path, ID);
	string_append(&logger_path, ".log");

	logger = log_create(logger_path, "cliente", 1, LOG_LEVEL_INFO);

	//Obtenemos los valores de la config.
	//IP's
	char *ip;
	//Puertos
	int puerto = config_get_int_value(config, "PUERTO");
	switch (puerto)
	{
	case 5001:
		ip = config_get_string_value(conexiones_config,"IP_COMANDA");
		break;
	case 5002:
		ip = config_get_string_value(conexiones_config,"IP_RESTAURANTE");
		break;
	case 5003:
		ip = config_get_string_value(conexiones_config,"IP_COMANDA");
		break;
	case 5004:
		ip = config_get_string_value(conexiones_config,"IP_APP");
		break;
	}

	socket_servidor = crear_conexion(ip, puerto);
	//printf("%d\n",socket_servidor);
	if (socket_servidor == -1)
	{
		log_info(logger,"ERROR AL CREAR LA CONEXION\n");
		terminar_programa(socket_servidor, logger, config);
		return 0;
	}

	//Posiciones
	char *posicion_x = config_get_string_value(config, "POSICION_X");
	char *posicion_y = config_get_string_value(config, "POSICION_Y");\

	uint32_t modulo, validacion;
	
	modulo = handshake(CLIENTE,socket_servidor);

	
	pthread_t retorno;
	pthread_create(&retorno, NULL, mensajes_retorno, NULL);
	switch (modulo)
	{
	case APP:
		log_info(logger, "Se conecto al modulo app");

		char *unaPalabra = string_new();

		string_append(&unaPalabra, ID);
		string_append(&unaPalabra, " ");
		string_append(&unaPalabra, posicion_x);
		string_append(&unaPalabra, " ");
		string_append(&unaPalabra, posicion_y);

		a_enviar(unaPalabra, socket_servidor, APP, RECONOCIMIENTO);

		recv(socket_servidor, &validacion, sizeof(uint32_t), 0);
		if (!validacion)
		{
			error_show("Cerrando conexion, ID del cliente ya registrada\n");
			terminar_programa(socket_servidor, logger, config);
			return 0;
		}
		consola(socket_servidor, modulo);
		a_enviar("", socket_servidor, APP, cerrar_conexion);

		break;

	case COMANDA:
		log_info(logger, "Se conecto al modulo comanda");
		consola(socket_servidor, modulo);

		break;

	case RESTAURANTE:
		log_info(logger, "Se conecto al modulo restaurante");
		consola(socket_servidor, modulo);
		a_enviar("", socket_servidor, RESTAURANTE, cerrar_conexion);
		break;

	case SINDICATO:
		log_info(logger, "Se conecto al modulo sindicato");
		consola(socket_servidor, modulo);

		break;
	}
	config_destroy(conexiones_config);
	terminar_programa(socket_servidor, logger, config);
	return 0;
}

void *mensajes_retorno()
{
	char *l;
	while (1)
	{
		int result = recv(socket_servidor, &l, 2, 0);
		if (result > 0)
		{
			t_paquete *paquete = recibir_y_desempaquetar(socket_servidor);
			char *retorno = deserializar_palabra(paquete->buffer);
			log_info(logger, retorno);
		}
	}
}

void cerrar()
{
	if (modulo == APP)
	{
		a_enviar("", socket_servidor, APP, cerrar_conexion);
		printf(" \n");
		exit(1);
	}
	if(modulo == RESTAURANTE)
	{
		a_enviar("", socket_servidor, RESTAURANTE, cerrar_conexion);
		printf(" \n");
		exit(1);
	}

}
