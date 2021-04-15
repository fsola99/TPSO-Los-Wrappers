#include "restaurante.h"
#define CONFIG_PATH "./cfg/restaurante.config"
#define CONEXIONES_PATH "../shared/cfg/conexiones.config"
int main(int argc, char **argv)
{
	signal(SIGINT, cerrar_restaurante);
	t_config *config_inicial = leer_config(CONFIG_PATH);
	char *nombre_restaurante = config_get_string_value(config_inicial, "NOMBRE_RESTAURANTE");
	char *restaurante_path = string_from_format("./cfg/%s.config", nombre_restaurante);
	restaurante_config = leer_config(restaurante_path);
	conexiones_config = leer_config(CONEXIONES_PATH);
	IP = config_get_string_value(conexiones_config, "IP_RESTAURANTE");

	logger = log_create(config_get_string_value(restaurante_config, "ARCHIVO_LOG"), "restaurante", 1, LOG_LEVEL_INFO);
	algoritmo = config_get_string_value(restaurante_config, "ALGORITMO_PLANIFICACION");
	quantum = config_get_int_value(restaurante_config, "QUANTUM");
	retardo_cpu = config_get_int_value(restaurante_config, "RETARDO_CICLO_CPU");

	char *datos_resto = mensaje_sindicato(nombre_restaurante, obtener_restaurante);
	resto = obtener_metadata(datos_resto);
	pthread_mutex_init(&mx_resto, NULL);
	log_info(logger, "DATOS OBTENIDOS: Restaurante [%s], CantCocineros [%d], CantPedidos [%d]", resto->nombre, resto->cantidad_cocineros, resto->cantidad_pedidos);

	procesadores = resto->cantidad_cocineros;
	cantidad_afinidades = string_array_length(resto->afinidades);
	inicializar_planificador();
	inicializar_colas(resto->afinidades);
	log_info(logger, "Colas de READY inicializadas");

	int puerto_restaurante = config_get_int_value(conexiones_config, "PUERTO_RESTAURANTE");
	int server_fd = iniciar_servidor(IP, puerto_restaurante); //creamos el servidor para que se conecte un cliente

	char *ip_app = config_get_string_value(conexiones_config, "IP_APP");
	int puerto_app = config_get_int_value(conexiones_config, "PUERTO_APP");
	int socket_app = crear_conexion(ip_app, puerto_app); //creamos la conexion para conectarnos a APP
	if (socket_app < 0)
	{
		printf("No se conecto APP, esperando cliente\n");
		socket_conexion = esperar_cliente(server_fd);
		modulo = handshake(RESTAURANTE, socket_conexion);
		while (1)
			atender_cliente(socket_conexion, modulo);
	}
	close(server_fd);

	modulo = handshake(RESTAURANTE, socket_app);
	char *reconocimiento_restaurante = string_from_format("%s %d %d", resto->nombre, resto->pos_x, resto->pos_y);
	a_enviar(reconocimiento_restaurante, socket_app, APP, RECONOCIMIENTO);
	free(reconocimiento_restaurante);
	socket_conexion = socket_app;
	while (1)
		atender_cliente(socket_conexion, modulo);
	free(datos_resto);
	free(restaurante_path);
	config_destroy(config_inicial);
	return EXIT_SUCCESS;
}

t_rest *obtener_metadata(char *str_datos)
{
	t_rest *metadata = malloc(sizeof(t_rest));
	char **datos = string_split(str_datos, " ");
	// 0 CANT_COCINEROS		1 POSICION-XY	2 AFINIDAD	3 PLATOS	4 PRECIOS	5 CANT_HORNOS	6 CANT_PEDIDOS
	//	int
	metadata->nombre = config_get_string_value(restaurante_config, "NOMBRE_RESTAURANTE");
	metadata->cantidad_cocineros = atoi(datos[0]);
	char **posiciones = string_get_string_as_array(datos[1]);
	metadata->pos_x = atoi(posiciones[0]);
	metadata->pos_y = atoi(posiciones[1]);
	metadata->afinidades = string_get_string_as_array(datos[2]); // "[s, da, asd]" -> ["s", "da", "asd"]
	metadata->platos = string_get_string_as_array(datos[3]);
	metadata->precios = string_get_string_as_array(datos[4]);
	metadata->cantidad_hornos = atoi(datos[5]);
	metadata->cantidad_pedidos = atoi(datos[6]);

	liberar_recibido(datos);
	liberar_recibido(posiciones);
	return metadata;
}

void atender_cliente(int socket, int modulo)
{
	t_paquete *paquete = recibir_y_desempaquetar(socket);
	void *stream = paquete->buffer->stream;

	switch (paquete->codigo_operacion)
	{
	case consultar_platos:;
		log_info(logger, "RECIBI [consultar_platos]");
		char *respuesta_platos = mensaje_sindicato(resto->nombre, consultar_platos);
		a_enviar(respuesta_platos, socket, modulo, RETORNO);
		free(respuesta_platos);
		break;
	case crear_pedido:;
		log_info(logger, "RECIBI [crear_pedido]");
		pthread_mutex_lock(&mx_resto);
		resto->cantidad_pedidos++;
		char *id_pedido = string_itoa(resto->cantidad_pedidos);
		pthread_mutex_unlock(&mx_resto);
		char *mensaje_0 = string_from_format("%s %s", resto->nombre, id_pedido);
		char *ok_fail_1 = mensaje_sindicato(mensaje_0, guardar_pedido);
		if (string_equals_ignore_case(ok_fail_1, "OK"))
		{
			log_info(logger, "Nuevo pedido creado y guardado. ID [%s]", id_pedido);
			a_enviar(id_pedido, socket, modulo, RETORNO);
		}
		else
		{
			log_error(logger, "SINDICATO no pudo guardar el pedido");
			a_enviar(ok_fail_1, socket, modulo, RETORNO);
		}
		free(ok_fail_1);
		free(mensaje_0);
		free(id_pedido);
		break;
	case aniadir_plato:;
		char **recibido1 = deserializar_paquete(paquete); //id, plato
		log_info(logger, "RECIBI [aniadir_plato] PLATO [%s] al PEDIDO [%s]", recibido1[1], recibido1[0]);
		if (!string_in_array(recibido1[1], resto->platos))
		{
			log_error(logger, "[%s] no contiene el plato [%s]", resto->nombre, recibido1[1]);
			a_enviar("FAIL", socket, modulo, RETORNO);
		}
		else
		{
			char *mensaje_1 = string_from_format("%s %s %s 1", resto->nombre, recibido1[0], recibido1[1]);
			char *ok_fail_2 = mensaje_sindicato(mensaje_1, guardar_plato);
			if (string_equals_ignore_case(ok_fail_2, "FAIL"))
			{
				log_error(logger, "SINDICATO no puedo guardar el plato [%s] en el pedido [%s]", recibido1[1], recibido1[0]);
			}
			a_enviar(ok_fail_2, socket, modulo, RETORNO);
			free(ok_fail_2);
			free(mensaje_1);
		}
		liberar_recibido(recibido1);
		break;
	case confirmar_pedido:;
		char *id_pedido_recibido = deserializar_palabra(paquete->buffer);
		log_info(logger, "Recibi [confirmar_pedido %s]", id_pedido_recibido);
		pthread_t hilo_confirmar_pedido;
		pthread_create(&hilo_confirmar_pedido, NULL,(void*)r_confirmar_pedido, id_pedido_recibido);
		break;
	case consultar_pedido:;
		char *id_pedido_recibido1 = deserializar_palabra(paquete->buffer);
		char *mensaje_3 = string_from_format("%s %s", resto->nombre, id_pedido_recibido1);
		char *respuesta_pedido_2 = mensaje_sindicato(mensaje_3, obtener_pedido);
		if (string_equals_ignore_case(respuesta_pedido_2, "FAIL"))
		{
			log_error(logger, "SINDICATO no pudo obtener datos del pedido [%s]", id_pedido_recibido1);
		}
		a_enviar(respuesta_pedido_2, socket, modulo, RETORNO);
		free(respuesta_pedido_2);
		free(mensaje_3);
		free(id_pedido_recibido1);
		break;
	case cerrar_conexion:;
		cerrar_restaurante();
	}
	liberar_paquete(paquete, stream);
}

void cerrar_restaurante()
{
	//destruir_colas_planificador();
	destruir_planificador();
	free_t_rest(resto);
	log_destroy(logger);
	config_destroy(restaurante_config);
	config_destroy(conexiones_config);
	if (modulo == APP)
	{
		a_enviar("", socket_conexion, APP, cerrar_conexion);
	}
	exit(1);
}

char *mensaje_sindicato(char *msj, int cOp)
{
	char *sindicato_ip = config_get_string_value(conexiones_config, "IP_SINDICATO");
	int sindicato_puerto = config_get_int_value(conexiones_config, "PUERTO_SINDICATO");
	int conexion_sindicato = crear_conexion(sindicato_ip, sindicato_puerto);
	handshake(RESTAURANTE, conexion_sindicato);
	a_enviar(msj, conexion_sindicato, SINDICATO, cOp);
	t_paquete *paquete = recibir_y_desempaquetar(conexion_sindicato);
	void *stream = paquete->buffer->stream;
	char *respuesta = deserializar_palabra(paquete->buffer);
	liberar_paquete(paquete, stream);
	close(conexion_sindicato);
	return respuesta;
}

void r_confirmar_pedido(char* id_pedido)
{
	char *mensaje = string_from_format("%s %s", resto->nombre, id_pedido);
	char *str_pedido = mensaje_sindicato(mensaje, obtener_pedido);
	if (string_equals_ignore_case(str_pedido, "FAIL"))
	{
		log_error(logger, "SINDICATO no pudo obtener el pedido [%s]", id_pedido);
		a_enviar(str_pedido, socket_conexion, modulo, RETORNO);
	}
	else
	{
		char *ok_fail_3 = mensaje_sindicato(mensaje, confirmar_pedido);
		a_enviar(ok_fail_3, socket_conexion, modulo, RETORNO);
		if (string_equals_ignore_case(ok_fail_3, "FAIL"))
		{
			log_error(logger, "SINDICATO no pudo confirmar el pedido [%s]", id_pedido);
		}
		else
		{
			char **datos = string_split(str_pedido, " ");
			// 0 ESTADO_PEDIDO		1 LISTA_PLATOS						2 CANTIDAD_PLATOS		3 CANTIDAD_LISTA 	4 PRECIO_TOTAL
			// Confirmado			[Milanesa,Empanadas,Ensalada]		[2,12,1]				[1,6,0]				1150
			char **lista_platos = string_get_string_as_array(datos[1]);
			char **cantidades_platos = string_get_string_as_array(datos[2]);
			char **cantidades_lista = string_get_string_as_array(datos[3]);
			
			pthread_mutex_lock(&mx_pedidos);
			t_pedido *pedido = agregar_pedido(id_pedido, cantidades_platos, cantidades_lista);
			log_info(logger, "Agrego pedido [%d] a la lista de pedidos", pedido->id);
			list_add(pedidos, pedido);
			pthread_mutex_unlock(&mx_pedidos);

			for (int i = 0; lista_platos[i] != NULL; i++)
			{
				int cantidad_faltante = atoi(cantidades_platos[i]) - atoi(cantidades_lista[i]);
				for(int j=0; j < cantidad_faltante; j++)
				{
					t_receta *receta_plato = r_obtener_receta(lista_platos[i]);
					t_pcb *pcb = malloc(sizeof(t_pcb));
					pcb->id_pedido = atoi(id_pedido);
					pcb->id_plato = j + 1;
					pcb->prioridad = 3;
					pcb->datos = receta_plato;
					pcb->index_afinidad = asignar_afinidad(pcb->datos->nombre_plato);
					pcb->pc = 0;
					pcb->espera_horno = 0;
					pcb->estado = NEW;
					sem_init(&pcb->sem_listo, 0, 0);
					encolar_ready(pcb, false);
				}
			}
			liberar_recibido(cantidades_lista);
			liberar_recibido(cantidades_platos);
			liberar_recibido(lista_platos);
			liberar_recibido(datos);
			}
		free(ok_fail_3);
	}
	free(str_pedido);
	free(mensaje);
}

t_pedido *agregar_pedido(char *id, char **cant_total, char **cant_lista)
{
	t_pedido *pedido = malloc(sizeof(t_pedido));
	pedido->id = atoi(id);
	int sum_platos = 0;
	int cantidad_platos = string_array_length(cant_total);
	for(int i = 0; i < cantidad_platos; i++){
		int faltan = atoi(cant_total[i]) - atoi(cant_lista[i]);
		sum_platos += faltan;
	}
	pedido->cantidad_platos = sum_platos;
	sem_init(&pedido->sem_plato_listo, 0, 0);
	pthread_create(&pedido->hilo_pedido, NULL, (void *)r_terminar_pedido, pedido);
	return pedido;
}

void r_terminar_pedido(t_pedido *pedido)
{
	while (1)
	{
		for (int i = 0; i < pedido->cantidad_platos; i++)
		{
			sem_wait(&pedido->sem_plato_listo);
		}
		char *mensaje = string_from_format("%s %d", resto->nombre, pedido->id);
		char *respuesta = mensaje_sindicato(mensaje, terminar_pedido);
		if (string_equals_ignore_case(respuesta, "ok"))
		{
			log_info(logger, "PEDIDO[%d] TERMINADO", pedido->id);
		}
		else
		{
			log_error(logger, "SINDICATO terminar_pedido: %s", respuesta);
		}
		a_enviar(respuesta, socket_conexion, modulo, RETORNO);
		free(respuesta);
		free(mensaje);
	}
}

void free_t_pedido(t_pedido *pedido)
{
	free(pedido);
}

t_pedido *encontrar_pedido(int id)
{
	pthread_mutex_lock(&mx_pedidos);
	for (int i = 0; i < list_size(pedidos); i++)
	{
		t_pedido *pedido = list_get(pedidos, i);
		if (pedido->id == id)
		{
			pthread_mutex_unlock(&mx_pedidos);
			return pedido;
		}
	}
	pthread_mutex_unlock(&mx_pedidos);
	return NULL;
}

void r_plato_listo(t_pcb *plato)
{
	plato->estado = FIN;
	char *info_plato = string_from_format("%s %d %s", resto->nombre, plato->id_pedido, plato->datos->nombre_plato);
	char *respuesta = mensaje_sindicato(info_plato, plato_listo);
	if (string_equals_ignore_case(respuesta, "ok"))
	{
		log_info(logger, "PLATO LISTO [%d][%s-%d] terminado (EXIT)", plato->id_pedido, plato->datos->nombre_plato, plato->id_plato);
		a_enviar(info_plato, socket_conexion, modulo, plato_listo);
		t_pedido *pedido = encontrar_pedido(plato->id_pedido);
		sem_post(&pedido->sem_plato_listo);
	}
	else
	{
		log_error(logger, "SINDICATO plato_listo: %s", respuesta);
		a_enviar(respuesta, socket_conexion, modulo, RETORNO);
	}
	free(respuesta);
	free(info_plato);
}

t_receta *r_obtener_receta(char *plato)
{
	char *respuesta_receta = mensaje_sindicato(plato, obtener_receta);

	if (string_equals_ignore_case(respuesta_receta, "FAIL"))
	{
		log_error(logger,"RECETA [%s] NO EXISTE CAPO",plato);
		return NULL;
	}
	char **datos = string_split(respuesta_receta, " ");
	// [Cortar,Hornear,Reposar,servir] [2,1,3,2]
	t_receta *receta_plato = malloc(sizeof(t_receta));
	receta_plato->nombre_plato = string_duplicate(plato);
	receta_plato->pasos = string_get_string_as_array(datos[0]);
	receta_plato->cantidad_pasos = string_get_string_as_array(datos[1]);
	receta_plato->total_pasos = string_array_length(receta_plato->pasos);

	liberar_recibido(datos);
	free(respuesta_receta);

	return receta_plato;
}

//-------------LIBERAR DATOS------------------
void free_t_rest(t_rest *rest)
{
	liberar_recibido(rest->platos);
	liberar_recibido(rest->precios);
	free(rest->afinidades);
	free(rest);
}