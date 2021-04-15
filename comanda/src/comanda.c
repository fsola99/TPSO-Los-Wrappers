#include "comanda.h"

int main(int argc, char **argv)
{
	t_config *comanda_config = leer_config(CONFIG_PATH);
	conexiones_config = leer_config(CONEXIONES_PATH);
	IP = config_get_string_value(conexiones_config,"IP_COMANDA");
	char *logger_path = config_get_string_value(comanda_config, "ARCHIVO_LOG");

	logger = iniciar_logger(logger_path);

	algoritmo_reemplazo = config_get_string_value(comanda_config, "ALGORITMO_REEMPLAZO");

	tamanio_memoria = config_get_int_value(comanda_config, "TAMANIO_MEMORIA") / sizeof(t_plato);
	tamanio_swap = config_get_int_value(comanda_config, "TAMANIO_SWAP") / sizeof(t_plato);

	memoria_principal = calloc(tamanio_memoria, sizeof(t_plato));
	memoria_swap = calloc(tamanio_swap, sizeof(t_plato));

	bitarray_memoria = calloc(tamanio_memoria, sizeof(int));
	bitarray_swap = calloc(tamanio_swap, sizeof(int));
	
	int puerto_comanda = config_get_int_value(conexiones_config, "PUERTO_COMANDA");
	int server_fd = iniciar_servidor(IP, puerto_comanda);
	int cliente_fd = esperar_cliente(server_fd);
	if (cliente_fd < 0)
	{
		error_show("No se pudo establecer la conexion");
		exit(1);
	}
	modulo = handshake(COMANDA, cliente_fd);

	restaurantes = list_create();
	lista_aux = list_create();
	aguja_contador = 0;
	pthread_t hilo;
	while (1)
	{
		if (modulo == APP)
		{
			cliente_fd = esperar_cliente(server_fd);
			if (cliente_fd < 0)
			{
				error_show("No se pudo establecer la conexion con la app");
				exit(1);
			}
			modulo = handshake(COMANDA, cliente_fd);
			pthread_create(&hilo,NULL,(void*)atender_cliente,(void*)cliente_fd);
		} else {
			if (cliente_fd < 0)
			{
				error_show("No se pudo establecer la conexion con el cliente");
				exit(1);
			}
			atender_cliente((void*)cliente_fd);
		}
	}

	liberar_memoria(memoria_principal);
	liberar_memoria(memoria_swap);
	liberar_bitarray(bitarray_memoria);
	liberar_bitarray(bitarray_swap);
	liberar_estructura();
	log_destroy(logger);
	config_destroy(comanda_config);
	config_destroy(conexiones_config);
	free(algoritmo_reemplazo);

	return EXIT_SUCCESS;
}
void atender_cliente(void* cliente)
{
	int cliente_fd = (int) cliente;
	t_paquete *paquete = recibir_y_desempaquetar(cliente_fd);
	void *stream = paquete->buffer->stream;
	switch (paquete->codigo_operacion)
	{
	case guardar_pedido:;
		char **recibido = deserializar_paquete(paquete);
		char *ok_fail_1 = f_guardar_pedido(recibido[0],atoi(recibido[1]));
		a_enviar(ok_fail_1, cliente_fd, modulo, RETORNO);
		free(ok_fail_1);
		liberar_recibido(recibido);
		break;
	case guardar_plato:;
		char **recibido_2 = deserializar_paquete(paquete);
		char *ok_fail_2 = f_guardar_plato(recibido_2[0],atoi(recibido_2[1]),recibido_2[2],atoi(recibido_2[3]));
		printf("%s\n",ok_fail_2);
		a_enviar(ok_fail_2, cliente_fd, modulo, RETORNO);
		free(ok_fail_2);
		liberar_recibido(recibido_2);
		break;
	case obtener_pedido:;
		char **recibido_3 = deserializar_paquete(paquete);
		char *pedido = f_obtener_pedido(recibido_3[0], atoi(recibido_3[1]));
		printf("PEDIDO:%s\n", pedido);
		a_enviar(pedido, cliente_fd, modulo, RETORNO);
		free(pedido);
		liberar_recibido(recibido_3);
		break;
	case confirmar_pedido:;
		char **recibido_4 = deserializar_paquete(paquete);
		char *ok_fail_4 = f_confirmar_pedido(recibido_4[0], atoi(recibido_4[1]));
		printf("%s\n", ok_fail_4);
		a_enviar(ok_fail_4, cliente_fd, modulo, RETORNO);
		free(ok_fail_4);
		liberar_recibido(recibido_4);
		break;
	case plato_listo:;
		char **recibido_5 = deserializar_paquete(paquete);
		char *ok_fail_5 = f_plato_listo(recibido_5[0], recibido_5[2], atoi(recibido_5[1]));
		a_enviar(ok_fail_5, cliente_fd, modulo, RETORNO);
		free(ok_fail_5);
		liberar_recibido(recibido_5);
		break;
	case finalizar_pedido:;
		char **recibido_6 = deserializar_paquete(paquete);
		char *ok_fail_6 = f_finalizar_pedido(recibido_6[0], atoi(recibido_6[1]));
		printf("%s\n", ok_fail_6);
		a_enviar(ok_fail_6, cliente_fd, modulo, RETORNO);
		free(ok_fail_6);
		liberar_recibido(recibido_6);
		break;
	}

	liberar_paquete(paquete, stream);
}