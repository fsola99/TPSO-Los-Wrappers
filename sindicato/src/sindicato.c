#include "sindicato.h"
#define CONFIG_PATH "./cfg/sindicato.config"
#define CONEXION_PATH "../shared/cfg/conexiones.config"
int main(int argc, char **argv)
{
	t_config *sindicato_config;
	sindicato_config = leer_config(CONFIG_PATH);
	conexiones_config = leer_config(CONEXION_PATH);
	IP = config_get_string_value(conexiones_config,"IP_SINDICATO");
	char *logger_path_sindicato = config_get_string_value(sindicato_config, "ARCHIVO_LOG");
	logger = iniciar_logger(logger_path_sindicato);
	free(logger_path_sindicato);
	
	pthread_mutex_init(&mutex,NULL);

	punto_montaje = config_get_string_value(sindicato_config, "PUNTO_MONTAJE");

	//Conexion Sindicato -> Restaurant
	int puerto_sindicato = config_get_int_value(conexiones_config, "PUERTO_SINDICATO");
	int blocks = config_get_int_value(sindicato_config, "BLOCKS");
	int block_size = config_get_int_value(sindicato_config, "BLOCK_SIZE");
	int server_sindicato = iniciar_servidor(IP, puerto_sindicato);

	//INICIALIZAMOS
	inicializacion(blocks, block_size);


	

	pthread_t hilo;
	pthread_t consola;
	pthread_create(&consola, NULL, consola_sindicato, NULL);
	//administacion de mensajes!~
	while (1)
	{
		int cliente_fd = esperar_cliente(server_sindicato);
		modulo = handshake(SINDICATO, cliente_fd);
		if (modulo == RESTAURANTE)
		{
			if (cliente_fd < 0)
			{
				error_show("No se pudo establecer la conexion");
				exit(1);
			}
			log_info(logger, "Se conecto un restaurante");
			pthread_create(&hilo, NULL, (void *)atender_cliente, (void*)cliente_fd);
		}
		else
		{
			if (cliente_fd < 0)
			{
				error_show("No se pudo establecer la conexion con el cliente");
				exit(1);
			}
			
			log_info(logger, "Se conecto un cliente");
			atender_cliente((void *)cliente_fd);
		}
	}
	
	bitarray_destroy(bitmap);
	log_destroy(logger);
	config_destroy(sindicato_config);
	config_destroy(conexiones_config);
	return EXIT_SUCCESS;
}
void atender_cliente(void* cliente)
{
	int cliente_fd = (int) cliente;
	t_paquete *paquete = recibir_y_desempaquetar(cliente_fd);
	void *stream = paquete->buffer->stream;

	switch (paquete->codigo_operacion)
	{
	case consultar_platos:;
		log_info(logger,"Se recibe mensaje 'Consultar Platos'");
		char *recibido_0 = deserializar_palabra(paquete->buffer);
		char *platos = f_consultar_platos(recibido_0);
		a_enviar(platos, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Consultar Platos' fue: %s",platos);
		free(recibido_0);
		free(platos);
		break;
	case guardar_pedido:;
		log_info(logger,"Se recibe mensaje 'Guardar Pedido'");
		char **recibido_1 = deserializar_paquete(paquete);
		char *ok_fail_1 = f_guardar_pedido(recibido_1[0], atoi(recibido_1[1]));
		a_enviar(ok_fail_1, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Guardar Pedido' fue: %s",ok_fail_1);
		liberar_recibido(recibido_1);
		free(ok_fail_1);
		break;
	case guardar_plato:;
		log_info(logger,"Se recibe mensaje 'Guardar Plato'");
		char **recibido_2 = deserializar_paquete(paquete);
		char *ok_fail_2 = f_guardar_plato(recibido_2[0], atoi(recibido_2[1]), recibido_2[2], atoi(recibido_2[3]));
		a_enviar(ok_fail_2, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Guardar Plato' fue: %s",ok_fail_2);
		liberar_recibido(recibido_2);
		free(ok_fail_2);
		break;
	case confirmar_pedido:;
		log_info(logger,"Se recibe mensaje 'Confirmar Pedido'");
		char **recibido_3 = deserializar_paquete(paquete);
		char *ok_fail_3 = f_confirmar_pedido(recibido_3[0], atoi(recibido_3[1]));
		a_enviar(ok_fail_3, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Confirmar Pedido' fue: %s",ok_fail_3);
		liberar_recibido(recibido_3);
		free(ok_fail_3);
		break;
	case obtener_pedido:;
		log_info(logger,"Se recibe mensaje 'Obtener Pedido'");
		char **recibido_4 = deserializar_paquete(paquete);
		char *pedido = f_obtener_pedido(recibido_4[0], atoi(recibido_4[1]));
		a_enviar(pedido, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Obtener Pedido' fue: %s",pedido);
		liberar_recibido(recibido_4);
		free(pedido);
		break;
	case obtener_restaurante:;
		log_info(logger,"Se recibe mensaje 'Obtener Restaurante'");
		char *recibido_5 = deserializar_palabra(paquete->buffer);
		char *restaurante_obtenido = f_obtener_restaurante(recibido_5);
		a_enviar(restaurante_obtenido, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Obtener Restaurante' fue: %s",restaurante_obtenido);
		free(recibido_5);
		free(restaurante_obtenido);
		break;
	case plato_listo:;
		log_info(logger,"Se recibe mensaje 'Plato Listo'");
		char **recibido_6 = deserializar_paquete(paquete);
		char *ok_fail_6 = f_plato_listo(recibido_6[0], atoi(recibido_6[1]), recibido_6[2]);
		a_enviar(ok_fail_6, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Plato Listo' fue: %s",ok_fail_6);
		liberar_recibido(recibido_6);
		free(ok_fail_6);
		break;
	case terminar_pedido:;
		log_info(logger,"Se recibe mensaje 'Terminar Pedido'");
		char **recibido_7 = deserializar_paquete(paquete);
		char *ok_fail_7 = f_terminar_pedido(recibido_7[0], atoi(recibido_7[1]));
		a_enviar(ok_fail_7, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Terminar Pedido' fue: %s",ok_fail_7);
		liberar_recibido(recibido_7);
		free(ok_fail_7);
		break;
	case obtener_receta:;
		log_info(logger,"Se recibe mensaje 'Obtener Receta'");
		char *recibido_8 = deserializar_palabra(paquete->buffer);
		char *receta = f_obtener_receta(recibido_8);
		a_enviar(receta, cliente_fd, modulo, RETORNO);
		log_info(logger,"La respuesta al mensaje 'Obtener Receta' fue: %s",receta);
		free(recibido_8);
		free(receta);
		break;
	}
	liberar_paquete(paquete, stream);
}

void menu_sindicato()
{
	printf("-------Elija una opcion-------- \n");
	printf("A- Crear un restaurante\n");
	printf("B- Crear una receta\n");
	printf("C- Para salir\n");
}

void *consola_sindicato()
{
	while (1)
	{
		menu_sindicato();
		char *opcion = readline("");
		string_to_lower(opcion);

		t_dictionary *diccionario_consola = dictionary_create();
		dictionary_put(diccionario_consola, "a", (void *)0);
		dictionary_put(diccionario_consola, "b", (void *)1);
		dictionary_put(diccionario_consola, "c", (void *)2);

		//Switch de opciones
		switch ((int)dictionary_get(diccionario_consola, opcion))
		{
		case 0:
			crear_restaurante();
			break;
		case 1:
			crear_receta();
			break;
		case 2:
			printf("cerrar consola\n");
			free(opcion);
			dictionary_destroy(diccionario_consola);

			return 0;
			break;
		default:
			printf("No se selecciono ningun opcion valida\n");
			break;
		}
		free (opcion);
		dictionary_destroy(diccionario_consola);
	}
}

int verificar_archivo(char *ruta)
{
	FILE *file;
	char *ruta_final = string_new();
	string_append(&ruta_final, punto_montaje);
	string_append(&ruta_final, ruta);
	int ret = 0;
	if ((file = fopen(ruta_final, "r")))
	{
		ret = 1;
		fclose(file);
	}
	free(ruta_final);
	return ret;
}

int verificar_restaurante(char *nombre_restaurante)
{
	char *restaurante = string_from_format("%s/Files/Restaurantes/%s", punto_montaje, nombre_restaurante);
	int existe = 0;
	DIR *dir = opendir(restaurante);
	if (dir)
	{
		existe = 1;
	}
	free(restaurante);
	closedir(dir);
	return existe;
}

void crear_restaurante()
{
	// ---------------------------MUTEX LOCK----------------------------//
	pthread_mutex_lock(&mutex);
	char **datos_restaurante;
	char *datos_ingresados;
	datos_ingresados = readline("Ingrese los datos para crear restaurante: \n");
	datos_restaurante = string_split(datos_ingresados, " ");
	if (parametros_check(datos_restaurante) == 7)
	{
		if (!verificar_restaurante(datos_restaurante[0]))
		{
			char *nuevo_restaurante = string_from_format("/Files/Restaurantes/%s", datos_restaurante[0]);
			nueva_carpeta(nuevo_restaurante);
			log_info(logger, "Creada carpeta del restaurante");

			// CREO ARRAY DE LOS DATOS PARA METERLOS EN LOS BLOQUES

			char *datos_agregar = string_from_format("CANTIDAD_COCINEROS=%s POSICION=%s AFINIDAD_PLATOS=%s PLATOS=%s PRECIO_PLATOS=%s CANTIDAD_HORNOS=%s CANTIDAD_PEDIDOS=0",
													 datos_restaurante[1], datos_restaurante[2], datos_restaurante[3], datos_restaurante[4], datos_restaurante[5], datos_restaurante[6]);

			int size_datos = string_length(datos_agregar);

			//Crear archivo info.AFIP
			string_append(&nuevo_restaurante, "/info.AFIP");
			
			nuevo_archivo_config(nuevo_restaurante);
			log_info(logger, "Creado el archivo info.AFIP del restaurante: %s",datos_restaurante[0]);

			int bloque_inicial = escribir_bloque(datos_agregar);

			char *tamanio_datos = string_itoa(size_datos);
			char *bloque = string_itoa(bloque_inicial);

			agregar_datos_config(nuevo_restaurante, "SIZE", tamanio_datos);
			agregar_datos_config(nuevo_restaurante, "INITIAL_BLOCK", bloque);
			log_info(logger, "Agrega los campos SIZE e INITIAL_BLOCK al archivo info.afip del restaurante: %s",datos_restaurante[0]);

			free(nuevo_restaurante);
			free(bloque);
			free(tamanio_datos);
			free(datos_agregar);
		}
		else
		{
			error_show("El restaurante ya existe\n");
		}
	}
	else
	{
		error_show("Cantidad de argumentos incorrecta\n");
	}
	free(datos_ingresados);
	liberar_recibido(datos_restaurante);
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK----------------------------//
}

void crear_receta()
{
	// ---------------------------MUTEX LOCK----------------------------//
	pthread_mutex_lock(&mutex);
	char **datos_receta;
	char *datos_ingresados;
	datos_ingresados = readline("Ingrese los datos para crear una receta: \n");
	datos_receta = string_split(datos_ingresados, " ");

	if (parametros_check(datos_receta) == 3)
	{
		char *nueva_receta = string_from_format("/Files/Recetas/%s.AFIP", datos_receta[0]);

		if (!verificar_archivo(nueva_receta))
		{
			char *datos_agregar = string_from_format("PASOS=%s TIEMPO_PASOS=%s", datos_receta[1], datos_receta[2]);
			int size_datos = string_length(datos_agregar);

			nuevo_archivo_config(nueva_receta);
			log_info(logger, "Creado el archivo .AFIP de la receta %s", datos_receta[0]);

			int bloque_inicial = escribir_bloque(datos_agregar);

			char *tamanio_datos = string_itoa(size_datos);
			char *bloque = string_itoa(bloque_inicial);

			agregar_datos_config(nueva_receta, "SIZE", tamanio_datos);
			agregar_datos_config(nueva_receta, "INITIAL_BLOCK", bloque);
			log_info(logger, "Agrega los campos SIZE e INITIAL_BLOCK al archivo info.afip");

			free(tamanio_datos);
			free(bloque);
			free(datos_agregar);
		}
		else
		{
			error_show("La receta ya existe\n");
		}

		free(nueva_receta);
	}
	else
	{
		error_show("Cantidad de argumentos incorrecta\n");
	}
	free(datos_ingresados);
	liberar_recibido(datos_receta);
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK----------------------------//
}

int obtener_size_archivo(char *archivo)
{
	int size;
	char *ruta = string_new();
	string_append(&ruta, punto_montaje);
	string_append(&ruta, archivo);
	t_config *config;
	config = leer_config(ruta);
	size = config_get_int_value(config, "SIZE");
	config_destroy(config);
	free(ruta);
	return size;
}

char *obtener_datos_restaurante(char *restaurante)
{
	char *path = string_from_format("/Files/Restaurantes/%s/info.AFIP", restaurante);
	int bloque_inicial = obtener_initial_block(path);
	char *datos;
	datos = obtener_datos_bloques(bloque_inicial);
	free(path);
	return datos;
}

char *obtener_datos_receta(char *receta)
{
	char *path = string_from_format("/Files/Recetas/%s.AFIP", receta);
	int bloque_inicial = obtener_initial_block(path);
	char *datos;
	datos = obtener_datos_bloques(bloque_inicial);
	free(path);
	return datos;
}

char *obtener_datos_pedido(char *restaurante, int pedido_ID)
{
	char *path = string_from_format("/Files/Restaurantes/%s/Pedido%d.AFIP",restaurante,pedido_ID);
	int bloque_inicial = obtener_initial_block(path);
	char *datos;
	datos = obtener_datos_bloques(bloque_inicial);
	free(path);
	return datos;
}


void eliminar_caracteres(char *x, int a, int b)
{
	if ((a + b - 1) <= strlen(x))
	{
		strcpy(&x[b - 1], &x[a + b - 1]);
	}
}


int obtener_precio_plato(char *nombre_restaurante, char *nombre_plato)
{
	int precio = -1;
	if (verificar_restaurante(nombre_restaurante) == 1)
	{
		char *datos_obtenidos;
		char **datos_restaurante;
		datos_obtenidos = obtener_datos_restaurante(nombre_restaurante);
		datos_restaurante = string_split(datos_obtenidos, " ");
		char **platos = string_split(datos_restaurante[3],"=");
		char **precios = string_split(datos_restaurante[4],"=");
		char **array_platos = string_get_string_as_array(platos[1]);
		char **array_precio = string_get_string_as_array(precios[1]);
		if (string_contains(platos[1], nombre_plato))
		{
			int i = 0;
			while (array_platos[i] != NULL)
			{
				if (strcmp(array_platos[i], nombre_plato) == 0)
				{
					precio = atoi(array_precio[i]);
				}
				i++;
			}
		}
		else
		{
			error_show("NO EXISTE EL PLATO INDICADO EN EL RESTAURANTE ESPECIFICADO\n");
			precio = -1;
		}
		liberar_recibido(platos);
		liberar_recibido(precios);
		liberar_recibido(array_platos);
		liberar_recibido(array_precio);
		liberar_recibido(datos_restaurante);
		free(datos_obtenidos);
	}
	else
	{
		error_show("NO EXISTE EL RESTAURANTE INDICADO\n");
		precio = -1;
	}
	return precio;
}



char *de_array_a_string(char **array)
{
	char *string = string_new();
	string_append(&string, "[");
	int cantidad_array = 0;
	while (array[cantidad_array] != NULL)
	{
		cantidad_array++;
	}
	for (int i = 0; i < cantidad_array; i++)
	{
		string_append(&string, array[i]);
		if (i < cantidad_array - 1)
		{
			string_append(&string, ",");
		}
	}
	string_append(&string, "]");
	return string;
}

int prox_pos_vacia(char **array)
{
	int i = 0;
	do
	{
		i++;
	} while (array[i] != NULL);
	return i;
}

char *agregar_elemento(char *array, char *nuevo_dato)
{
	int length = string_length(array);
	char *nuevo_array = string_substring_until(array, length - 1);
	if (length != 2)
	{
		string_append(&nuevo_array, ",");
		string_append(&nuevo_array, nuevo_dato);
		string_append(&nuevo_array, "]");
	}
	else
	{
		string_append(&nuevo_array, nuevo_dato);
		string_append(&nuevo_array, "]");
	}

	return nuevo_array;
}