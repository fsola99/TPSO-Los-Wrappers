#include "app.h"
#include "planificacion.h"
datos_t *datos_aux, *datos_cliente;


int main(int argc, char **argv)
{
	signal(SIGINT, cerrar);
	//config y logg
	app_config = leer_config(CONFIG_PATH);
	conexiones_config = leer_config(CONEXIONES_PATH);
	IP = config_get_string_value(conexiones_config,"IP_APP");
	char *logger_path = config_get_string_value(app_config, "ARCHIVO_LOG");
	logger = iniciar_logger(logger_path);
	free(logger_path);
	//Conexion App -> Comanda

	ip_comanda = config_get_string_value(conexiones_config, "IP_COMANDA");
	puerto_comanda = config_get_int_value(conexiones_config, "PUERTO_COMANDA");

	int conexion_comanda = crear_conexion(ip_comanda, puerto_comanda);
	if (conexion_comanda < 0)
		error_show("Comanda no conectada\n");
	else
	{
		handshake(APP, conexion_comanda);
		log_info(logger, "Handshake con comanda realizado correctamente\n");
		close(conexion_comanda);
	}

	int puerto_escucha = config_get_int_value(conexiones_config, "PUERTO_APP");
	int server_fd = iniciar_servidor(IP, puerto_escucha);

	inicializacion_planificacion();

	int maxfd, cliente_fd, result;
	FD_ZERO(&readset);
	FD_ZERO(&tempset);
	FD_SET(server_fd, &readset);
	maxfd = server_fd;
	datos_cliente = malloc(sizeof(datos_t));
	

	lista_cliente = list_create();
	lista_restaurante = list_create();

	do
	{
		memcpy(&tempset, &readset, sizeof(tempset));
		result = select(maxfd + 1, &tempset, NULL, NULL, NULL);

		if (result < 0)
		{
			printf("Error in select()\n");
			return 0;
		}
		if (FD_ISSET(server_fd, &tempset))
		{
			cliente_fd = esperar_cliente(server_fd);

			if (cliente_fd < 0)
				printf("Error in accept()\n");
			else
			{
				FD_SET(cliente_fd, &readset);
				if (maxfd < cliente_fd)
					maxfd = cliente_fd;
				handshake(APP, cliente_fd);
			}
			FD_CLR(server_fd, &tempset);
		}

		for (int cliente = 0; cliente < maxfd + 1; cliente++)
		{
			if (FD_ISSET(cliente, &tempset))
			{
				t_paquete *paquete = recibir_y_desempaquetar(cliente);
				void *stream = paquete->buffer->stream;

				switch (paquete->codigo_operacion)
				{
				case RECONOCIMIENTO:;
					char **recibido = deserializar_paquete(paquete);
					f_reconocimiento(recibido[0], atoi(recibido[1]), atoi(recibido[2]), cliente);
					liberar_recibido(recibido);
					break;
				case consultar_restaurantes:;
					f_consultar_restaurantes(cliente);
					break;
				case seleccionar_restaurante:;
					char **recibido1 = deserializar_paquete(paquete);
					f_seleccionar_restaurante(recibido1[0], cliente);
					liberar_recibido(recibido1);
					break;
				case consultar_platos:;
					f_consultar_platos(cliente);
					break;
				case crear_pedido:;
					if (conexion_comanda < 0)
						error_show("Comanda no conectada\n");
					else
						f_crear_pedido(cliente);
					break;
				case aniadir_plato:;
					if (conexion_comanda < 0)
						error_show("Comanda no conectada\n");
					else
					{
						char **recibido2 = deserializar_paquete(paquete);
						f_aniadir_plato(recibido2[0], recibido2[1], cliente);
						liberar_recibido(recibido2);
					}
					break;
				case confirmar_pedido:;
					char *ID = deserializar_palabra(paquete->buffer);
					f_confirmar_pedido(ID, cliente);
					free(ID);
					break;
				case consultar_pedido:;
					char *recibido4 = deserializar_palabra(paquete->buffer);
					datos_cliente = obtener_cliente(cliente);
					f_consultar_pedido(datos_cliente->restaurante->ID, recibido4, cliente);
					free(recibido4);
					datos_cliente = NULL;
					break;
				case plato_listo:;
					char **recibido3 = deserializar_paquete(paquete);
					f_plato_listo(recibido3[0], recibido3[1], recibido3[2], cliente_fd);
					liberar_recibido(recibido3);
					break;
				case cerrar_conexion:;
					cerrar_cliente(cliente);
					break;
				}

				liberar_paquete(paquete, stream);
			}
		}

	} while (1);
}

void cerrar_cliente(int cliente)
{	
	restaurante_t *restaurante;
	for (int i = 0; i < list_size(lista_cliente); i++)
	{
		datos_aux = list_get(lista_cliente, i);
		if (datos_aux->socket == cliente)
		{
			printf("Cerramos el %s\n", datos_aux->ID);
			list_remove(lista_cliente, i);
			break;
		}
	}
	for (int i = 0; i < list_size(lista_restaurante); i++)
	{
		restaurante = list_get(lista_restaurante, i);
		if (restaurante->socket == cliente)
		{
			list_remove(lista_restaurante, i);
			break;
		}
	}
	send(cliente, NULL, 0, 0);

	close(cliente);
	FD_CLR(cliente, &readset);
}

bool verificar(t_list *lista, char *ID, int pos_x, int pos_y)
{
	datos_t *cliente;
	for (int i = 0; i < list_size(lista); i++)
	{
		cliente = list_get(lista, i);
		if (!strcmp(ID, cliente->ID) || (cliente->pos_x == pos_x && cliente->pos_y == pos_y))
			return true;
	}
	return false;
}

void f_reconocimiento(char *ID, int pos_x, int pos_y, int cliente)
{	
	uint32_t validacion;
	restaurante_t *restaurante;
	restaurante = malloc(sizeof(restaurante_t));
	if (list_is_empty(lista_restaurante))
	{
		restaurante->ID = string_duplicate("restaurante_default");
		restaurante->pos_x = config_get_int_value(app_config, "POSICION_REST_DEFAULT_X");
		restaurante->pos_y = config_get_int_value(app_config, "POSICION_REST_DEFAULT_Y");
		restaurante->socket = 0;
		restaurante->ID_pedido = 0;
		list_add(lista_restaurante, restaurante);
	}

	if (!strncmp(ID,"Cliente",7))
	{
		datos_t *cliente_datos = malloc(sizeof(datos_t));
		cliente_datos->ID = string_duplicate(ID);
		cliente_datos->pos_x = pos_x;
		cliente_datos->pos_y = pos_y;
		cliente_datos->socket = cliente;
		cliente_datos->restaurante = NULL;

		if (list_is_empty(lista_cliente))
		{
			list_add(lista_cliente, cliente_datos);
			validacion = 1;
			send(cliente, &validacion, sizeof(uint32_t), 0);
		}
		else
		{
			if (verificar(lista_cliente, cliente_datos->ID, cliente_datos->pos_x, cliente_datos->pos_y))
			{
				printf("clientes iguales, cerrando conexion\n");
				validacion = 0;
				send(cliente, &validacion, sizeof(uint32_t), 0);
				close(cliente);
				FD_CLR(cliente, &readset);
			}
			else
			{
				list_add(lista_cliente, cliente_datos);
				validacion = 1;
				send(cliente, &validacion, sizeof(uint32_t), 0);
			}
		}
	}
	else
	{	
		restaurante_t *restaurante1;
		restaurante1 = malloc(sizeof(restaurante_t));
		restaurante1->ID = string_duplicate(ID);
		restaurante1->pos_x = pos_x;
		restaurante1->pos_y = pos_y;
		restaurante1->socket = cliente;
		restaurante1->ID_pedido = 0;
		list_add(lista_restaurante, restaurante1);
	}
	for (int i = 0; i < list_size(lista_cliente); i++)
	{
		datos_t * datos_aux1 = list_get(lista_cliente, i);
		printf("-----------------------%s--------------------------------\n", datos_aux1->ID);
		printf("POSX: %d\n", datos_aux1->pos_x);
		printf("POSY: %d\n", datos_aux1->pos_y);
		printf("SOCKET: %d\n", datos_aux1->socket);
	}
	for(int i = 0; i < list_size(lista_restaurante); i++)
	{
		restaurante_t * restaurante = list_get(lista_restaurante, i);
		printf("-----------------------%s-----------------------\n", restaurante->ID);
		printf("POSX: %d\n", restaurante->pos_x);
		printf("POSY: %d\n", restaurante->pos_y);
		printf("SOCKET: %d\n", restaurante->socket);
	}

}

void f_consultar_restaurantes(int cliente)
{
	printf("entramos en consultar_restaurante\n");
	char *nombres = string_new();
	restaurante_t *restaurante;
	for (int i = 0; i < list_size(lista_restaurante); i++)
	{
		restaurante = list_get(lista_restaurante, i);
		string_append(&nombres, restaurante->ID);
		if (i != list_size(lista_restaurante) - 1)
			string_append(&nombres, ", ");
	}
	a_enviar(nombres, cliente, CLIENTE, RETORNO);
	free(nombres);
	return;
}

void f_seleccionar_restaurante(char *nombre_restaurante, int cliente)
{
	printf("entramos en seleccionar_restaurante\n");
	char *resultado = string_new();
	string_append(&resultado, "FAIL");
	restaurante_t *restaurante;
	for (int i = 0; i < list_size(lista_restaurante); i++)
	{
		restaurante = list_get(lista_restaurante, i);
		if (!strcmp(nombre_restaurante, restaurante->ID))
		{
			datos_cliente = obtener_cliente(cliente);
			datos_cliente->restaurante = restaurante;
			memset(resultado, 0x00, 4);
			string_append(&resultado, "OK");
			a_enviar(resultado, cliente, CLIENTE, RETORNO);
			free(resultado);
			datos_cliente = NULL;
			return;
		}
	}

	a_enviar(resultado, cliente, CLIENTE, RETORNO);
	free(resultado);
	return;
}

void f_consultar_platos(int cliente)
{
	printf("entramos en consultar_platos\n");
	datos_cliente = obtener_cliente(cliente);

	if (datos_cliente->restaurante == NULL)
	{
		a_enviar("[ERROR] No se tiene asociado ningun restaurante", cliente, CLIENTE, RETORNO);
		return;
	}

	if (!strcmp(datos_cliente->restaurante->ID, "restaurante_default"))
	{
		char *platos = config_get_string_value(app_config, "PLATOS_DEFAULT");
		a_enviar(platos, cliente, CLIENTE, RETORNO);
		datos_cliente = NULL;
		free(platos);
		return;
	}
	else
	{
		char *platos = mensaje_restaurante("", consultar_platos, datos_cliente->restaurante->socket);
		a_enviar(platos, cliente, CLIENTE, RETORNO);
		datos_cliente = NULL;
		free(platos);
		return;
	}
}

void f_crear_pedido(int cliente)
{
	printf("entramos en crear_pedido\n");
	datos_cliente = obtener_cliente(cliente);

	if (datos_cliente->restaurante == NULL)
	{
		a_enviar("[ERROR] No se tiene asociado ningun restaurante", cliente, CLIENTE, RETORNO);
		return;
	}
	int ID_pedido = datos_cliente->restaurante->ID_pedido;
	char *respuesta = string_new();
	char *enviar = string_new();

	if (!strcmp(datos_cliente->restaurante->ID, "restaurante_default"))
	{
		ID_pedido++;
		enviar = string_from_format("restaurante_default %s", string_itoa(ID_pedido));

		respuesta = mensaje_comanda(enviar, guardar_pedido);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("se rechazo el pedido", cliente, CLIENTE, RETORNO);
			return;
		}
		datos_cliente->restaurante->ID_pedido++;
		memset(enviar, 0x00, strlen(enviar));
		enviar = string_from_format("El ID del pedido es: %s", string_itoa(ID_pedido));
		a_enviar(enviar, cliente, CLIENTE, RETORNO);
	}
	else
	{
		respuesta = mensaje_restaurante("", crear_pedido, datos_cliente->restaurante->socket);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("se rechazo el pedido", cliente, CLIENTE, RETORNO);
			return;
		}
		ID_pedido = atoi(respuesta);
		enviar = string_from_format("%s %d", datos_cliente->restaurante->ID, ID_pedido);
		memset(respuesta, 0x00, strlen(respuesta));
		respuesta = mensaje_comanda(enviar, guardar_pedido);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("se rechazo el pedido", cliente, CLIENTE, RETORNO);
			return;
		}
		memset(enviar, 0x00, strlen(enviar));
		enviar = string_from_format("El ID del pedido es: %s", string_itoa(ID_pedido));
		a_enviar(enviar, cliente, CLIENTE, RETORNO);
	}
	datos_cliente = NULL;
	return;
}

void f_aniadir_plato(char *ID, char *plato, int cliente)
{
	printf("entramos en aniadir_plato\n");
	datos_cliente = obtener_cliente(cliente);

	if (datos_cliente->restaurante == NULL)
	{
		a_enviar("[ERROR] No se tiene asociado ningun restaurante", cliente, CLIENTE, RETORNO);
		return;
	}
	char *enviar = string_new();
	char *respuesta = string_new();
	if (!strcmp(datos_cliente->restaurante->ID, "restaurante_default"))
	{
		enviar = string_from_format("restaurante_default %s %s 1", ID, plato);
		respuesta = mensaje_comanda(enviar, guardar_plato);
		free(enviar);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("Se rechazo el plato", cliente, CLIENTE, RETORNO);
			free(respuesta);
			return;
		}
		a_enviar("Aniadir plato = OK", cliente, CLIENTE, RETORNO);
		free(respuesta);
	}
	else
	{
		enviar = string_from_format("%s %s", ID, plato);
		respuesta = mensaje_restaurante(enviar, aniadir_plato, datos_cliente->restaurante->socket);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("Se rechazo el plato", cliente, CLIENTE, RETORNO);
			return;
		}
		memset(respuesta, 0x00, strlen(respuesta));
		memset(enviar, 0x00, strlen(enviar));
		enviar = string_from_format("%s %s %s 1", datos_cliente->restaurante->ID, ID, plato);
		respuesta = mensaje_comanda(enviar, guardar_plato);
		free(enviar);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("Se rechazo el plato", cliente, CLIENTE, RETORNO);
			free(respuesta);
			return;
		}
		a_enviar("Aniadir plato = OK", cliente, CLIENTE, RETORNO);
		free(respuesta);
	}
	datos_cliente = NULL;
	return;
}

void f_plato_listo(char *nombre_restaurante, char *id_pedido, char *plato, int cliente)
{
	char *enviar = string_new();
	enviar = string_from_format("%s %s %s", nombre_restaurante, id_pedido, plato);
	char *respuesta = mensaje_comanda(enviar, plato_listo);
	if (!strcmp(respuesta, "FAIL"))
	{
		a_enviar("Plato listo = FAIL", cliente, CLIENTE, RETORNO);
		free(respuesta);
		return;
	}
	//VERIFICAR LA CANTIDAD LISTA
	char *datos = string_new();
	datos = string_from_format("%s %s", nombre_restaurante, id_pedido);
	respuesta = mensaje_comanda(datos, obtener_pedido);
	char **estado = string_split(respuesta, ",");
	
	pedido_t *pedido = verificar_lista_block(id_pedido, nombre_restaurante,"buscar");
	if (!strcmp(estado[0], "Terminado") && pedido)
	{
		verificar_lista_block(id_pedido, nombre_restaurante,"borrar");
		log_info(logger,"El repartidor %c recibio el pedido del restaurante %s", pedido->repartidor->Identificador,pedido->restaurante);
		pedido->pedido_listo = 1;
		enviar_ready(pedido);
	}
	liberar_recibido(estado);
	free(enviar);
	free(datos);
	free(respuesta);
	return;
}

void f_confirmar_pedido(char *ID_pedido, int cliente)
{
	printf("entramos en confirmar_pedido\n");

	datos_cliente = obtener_cliente(cliente);

	char *respuesta = string_new();
	char *datos = string_new();
	if (!strcmp(datos_cliente->restaurante->ID, "restaurante_default"))
	{
		crear_PCB(datos_cliente, ID_pedido);

		datos = string_from_format("%s %s", datos_cliente->restaurante->ID, ID_pedido);
		respuesta = mensaje_comanda(datos, confirmar_pedido);
		free(datos);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("se rechazo el pedido", cliente, CLIENTE, RETORNO);
			free(respuesta);
			return;
		}

		a_enviar("Pedido confirmado", cliente, CLIENTE, RETORNO);
		free(respuesta);
	}
	else
	{
		respuesta = mensaje_restaurante(ID_pedido, confirmar_pedido, datos_cliente->restaurante->socket);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("Se rechazo la confirmacion del pedido en restaurante", cliente, CLIENTE, RETORNO);
			free(respuesta);
			return;
		}

		crear_PCB(datos_cliente, ID_pedido);

		datos = string_from_format("%s %s", datos_cliente->restaurante->ID, ID_pedido);
		memset(respuesta, 0x00, strlen(respuesta));
		respuesta = mensaje_comanda(datos, confirmar_pedido);
		free(datos);
		if (!strcmp(respuesta, "FAIL"))
		{
			a_enviar("se rechazo el pedido", cliente, CLIENTE, RETORNO);
			free(respuesta);
			return;
		}

		a_enviar("Pedido confirmado", cliente, CLIENTE, RETORNO);
		free(respuesta);
	}
	datos_cliente = NULL;
	return;
}

void f_consultar_pedido(char *restaurante, char *ID_pedido, int cliente)
{
	char *datos = string_new();
	char *respuesta = string_new();
	datos = string_from_format("%s %s", restaurante, ID_pedido);
	respuesta = mensaje_comanda(datos, obtener_pedido);
	free(datos);
	a_enviar(respuesta, cliente, CLIENTE, RETORNO);
	free(respuesta);
	return;
}

char *mensaje_comanda(char *mensaje, int codigo_operacion)
{
	int socket_comanda = crear_conexion(ip_comanda, puerto_comanda);
	handshake(APP, socket_comanda);

	a_enviar(mensaje, socket_comanda, COMANDA, codigo_operacion);

	t_paquete *paquete = recibir_y_desempaquetar(socket_comanda);
	void*stream = paquete->buffer->stream;
	char *respuesta = deserializar_palabra(paquete->buffer);
	
	close(socket_comanda);
	liberar_paquete(paquete,stream);
	return respuesta;
}

char *mensaje_restaurante(char *mensaje, int codigo_operacion, int socket_restaurante)
{
	a_enviar(mensaje, socket_restaurante, RESTAURANTE, codigo_operacion);
	t_paquete *paquete = recibir_y_desempaquetar(datos_cliente->restaurante->socket);
	char *respuesta = deserializar_palabra(paquete->buffer);
	return respuesta;
}

datos_t *obtener_cliente(int cliente)
{
	for (int i = 0; i < list_size(lista_cliente); i++)
	{
		datos_cliente = list_get(lista_cliente, i);
		if (datos_cliente->socket == cliente)
			return datos_cliente;
	}
	return 0;
}

void crear_PCB(datos_t *datos_cliente, char *ID_pedido)
{
	pedido_t *pedido = malloc(sizeof(pedido_t));
	pedido->ID_pedido = atoi(ID_pedido);
	pedido->cliente = datos_cliente->socket;
	pedido->pos_restaurante[0] = datos_cliente->restaurante->pos_x;
	pedido->pos_restaurante[1] = datos_cliente->restaurante->pos_y;
	pedido->pos_cliente[0] = datos_cliente->pos_x;
	pedido->pos_cliente[1] = datos_cliente->pos_y;
	pedido->pedido_listo = 0;
	pedido->restaurante = datos_cliente->restaurante->ID;
	queue_push(new, pedido);
	pthread_mutex_unlock(&mutex_new);
	return;
}

pedido_t *verificar_lista_block(char *ID_pedido, char *nombre_restaurante,char *condicion)
{
	
	pthread_mutex_lock(&mutex_espera);
	for (int i = 0; i < list_size(lista_bloqueados); i++)
	{
		pedido_t *pedido = list_get(lista_bloqueados, i);
		if (pedido->ID_pedido == atoi(ID_pedido) && !strcmp(pedido->restaurante, nombre_restaurante))
		{
			if(!strcmp(condicion,"borrar"))
				list_remove(lista_bloqueados,i);
			pthread_mutex_unlock(&mutex_espera);
			return pedido;
		}
	}
	pthread_mutex_unlock(&mutex_espera);
	return NULL;
}

void cerrar()
{
	config_destroy(conexiones_config);
	config_destroy(app_config);
	log_destroy(logger);
	printf("\n");
	exit(1);
}
