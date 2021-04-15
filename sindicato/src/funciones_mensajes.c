#include "funciones_mesajes.h"

char *f_confirmar_pedido(char *nombre_restaurante, int pedido_ID)
{
	// ---------------------------MUTEX LOCK--------------------------//
	pthread_mutex_lock(&mutex);
	char *ret = string_new();
	if (verificar_restaurante(nombre_restaurante) == 1)
	{
		char *pedido = string_from_format("/Files/Restaurantes/%s/Pedido%d.AFIP", nombre_restaurante, pedido_ID);
		if (verificar_archivo(pedido))
		{			
			char *datos_pedido = obtener_datos_pedido(nombre_restaurante, pedido_ID);
			char **array_datos_pedido = string_split(datos_pedido, " ");

			if (strcmp(array_datos_pedido[0], "ESTADO_PEDIDO=Pendiente") == 0)
			{

				int pos_bloque_pedido = obtener_initial_block(pedido);

				int length_estado_pedido = string_length(array_datos_pedido[0]);

				char *nuevos_datos_pedido = string_new();
				string_append(&nuevos_datos_pedido, "ESTADO_PEDIDO=Confirmado");
				char *otros_datos_pedido = string_substring_from(datos_pedido, length_estado_pedido);
				string_append(&nuevos_datos_pedido, otros_datos_pedido);

				char *bloques_ocupados = obtener_bloques_usados(pos_bloque_pedido);

				eliminar_bloques(bloques_ocupados);

				int tamanio_texto = string_length(nuevos_datos_pedido);
				int bloque_inicial = escribir_bloque(nuevos_datos_pedido);

				char *tamanio_datos = string_itoa(tamanio_texto);
				char *bloque = string_itoa(bloque_inicial);

				agregar_datos_config(pedido, "SIZE", tamanio_datos);
				agregar_datos_config(pedido, "INITIAL_BLOCK", bloque);

				free(bloques_ocupados);
				free(tamanio_datos);
				free(bloque);

				free(otros_datos_pedido);
				free(nuevos_datos_pedido);

				string_append(&ret, "OK");
			}
			else
			{
				error_show("EL ESTADO DEL PEDIDO NO ES PENDIENTE\n");
				string_append(&ret, "FAIL");
			}
			free(datos_pedido);
			liberar_recibido(array_datos_pedido);
		}
		else
		{
			error_show("PEDIDO NO EXISTE\n");
			string_append(&ret, "FAIL");
		}
		free(pedido);
	}
	else
	{
		error_show("RESTAURANTE NO EXISTE\n");
		string_append(&ret, "FAIL");
	}
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK--------------------------//
	return ret;
}

char *f_obtener_restaurante(char *restaurante)
{
	// ---------------------------MUTEX LOCK--------------------------//
	pthread_mutex_lock(&mutex);
	char *datos_a_devolver = string_new();
	if (verificar_restaurante(restaurante) == 1)
	{
		char *datos_obtenidos;
		char **datos_restaurante;
		char **datos_cortados;
		datos_obtenidos = obtener_datos_restaurante(restaurante);
		datos_restaurante = string_split(datos_obtenidos, " ");
		for (int i = 0; i <= 6; i++)
		{
			datos_cortados = string_split(datos_restaurante[i], "=");
			string_append(&datos_a_devolver, datos_cortados[1]);
			if (i < 6)
			{
				string_append(&datos_a_devolver, " ");
			}
			liberar_recibido(datos_cortados);
		}
		liberar_recibido(datos_restaurante);
		free(datos_obtenidos);
	}
	else
	{
		error_show("NO EXISTE EL RESTAURANTE INDICADO\n");
		string_append(&datos_a_devolver, "FAIL");
	}
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK--------------------------//
	return datos_a_devolver;
}

char *f_consultar_platos(char *nombre_restaurante)
{
	// ---------------------------MUTEX LOCK----------------------------//
	pthread_mutex_lock(&mutex);
	char *platos = string_new();
	char *datos_restaurante;
	char *ruta = string_from_format("/Files/Restaurantes/%s/info.AFIP", nombre_restaurante);
	char **datos_cortados;
	char **array_datos;
	if (verificar_restaurante(nombre_restaurante) == 1)
	{
		datos_restaurante = obtener_datos_restaurante(nombre_restaurante);
		array_datos = string_split(datos_restaurante, " ");
		datos_cortados = string_split(array_datos[3], "=");
		string_append(&platos, datos_cortados[1]);
	}
	else
	{
		string_append(&platos, "FAIL");
		error_show("No existe carpeta de ese restaurante\n");
	}
	free(datos_restaurante);
	free(ruta);
	liberar_recibido(array_datos);
	liberar_recibido(datos_cortados);
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK--------------------------//
	return platos;
}

char *f_guardar_pedido(char *nombre_restaurante, int pedido_ID)
{
	// ---------------------------MUTEX LOCK--------------------------//
	pthread_mutex_lock(&mutex);
	char *ret = string_new();

	if (verificar_restaurante(nombre_restaurante) == 1)
	{
		char *nuevo_pedido = string_from_format("/Files/Restaurantes/%s/Pedido%d.AFIP", nombre_restaurante, pedido_ID);
		if (!verificar_archivo(nuevo_pedido))
		{

				char *datos_agregar = string_from_format("ESTADO_PEDIDO=Pendiente LISTA_PLATOS=[] CANTIDAD_PLATOS=[] CANTIDAD_LISTA=[] PRECIO_TOTAL=0");

				int size_datos = string_length(datos_agregar);

				nuevo_archivo_config(nuevo_pedido);
				log_info(logger, "Creado el archivo .AFIP del pedido %d",pedido_ID);
				
				int bloque_inicial_pedido = escribir_bloque(datos_agregar);
				char *tamanio_datos_pedido = string_itoa(size_datos);
				char *bloque_pedido = string_itoa(bloque_inicial_pedido);

				agregar_datos_config(nuevo_pedido, "SIZE", tamanio_datos_pedido);
				agregar_datos_config(nuevo_pedido, "INITIAL_BLOCK", bloque_pedido);
				log_info(logger, "Agrega los campos SIZE e INITIAL_BLOCK al archivo Pedido%d.afip",pedido_ID);				

				char *ruta_restaurante = string_from_format("/Files/Restaurantes/%s/info.AFIP", nombre_restaurante);
				
				
				int pos_bloque_restaurante = obtener_initial_block(ruta_restaurante);
				int size_datos_restaurante = obtener_size_archivo(ruta_restaurante);
				char *datos_restaurante = obtener_datos_restaurante(nombre_restaurante);
				
				char **array_datos_restaurante = string_split(datos_restaurante, " ");
				char **array_cantidad_pedidos = string_split(array_datos_restaurante[6], "=");
				int cant_pedidos = (atoi(array_cantidad_pedidos[1])) + 1;

				int length_cant_pedidos = string_length(array_cantidad_pedidos[1]);
				char *new_datos_restaurante = string_substring_until(datos_restaurante, size_datos_restaurante - length_cant_pedidos);
				char *cantidad_pedidos = string_itoa(cant_pedidos);
				string_append(&new_datos_restaurante, cantidad_pedidos);

				char *bloques_ocupados = obtener_bloques_usados(pos_bloque_restaurante);

				eliminar_bloques(bloques_ocupados);

				int tamanio_texto = string_length(new_datos_restaurante);
				int bloque_inicial = escribir_bloque(new_datos_restaurante);

				char *tamanio_datos = string_itoa(tamanio_texto);
				char *bloque = string_itoa(bloque_inicial);

				agregar_datos_config(ruta_restaurante, "SIZE", tamanio_datos);
				agregar_datos_config(ruta_restaurante, "INITIAL_BLOCK", bloque);



				free(cantidad_pedidos);
				free(ruta_restaurante);
				free(datos_restaurante);
				liberar_recibido(array_datos_restaurante);
				liberar_recibido(array_cantidad_pedidos);
				free(new_datos_restaurante);
				free(bloques_ocupados);
				free(bloque_pedido);
				free(tamanio_datos_pedido);
				free(tamanio_datos);
				free(bloque);
				free(datos_agregar);
				string_append(&ret, "OK");
		}
		else
		{
			error_show("PEDIDO YA EXISTE\n");
			string_append(&ret, "FAIL");
		}
		free(nuevo_pedido);
	}
	else
	{
		error_show("RESTAURANTE NO EXISTE\n");
		string_append(&ret, "FAIL");
	}
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK--------------------------//
	return ret;
}

char *f_obtener_pedido(char *nombre_restaurante, int pedido_ID)
{
	// ---------------------------MUTEX LOCK----------------------------//
	pthread_mutex_lock(&mutex);
	char *datos_a_devolver = string_new();
	if (verificar_restaurante(nombre_restaurante))
	{
		char *path_pedido = string_from_format("/Files/Restaurantes/%s/Pedido%d.AFIP", nombre_restaurante, pedido_ID);
		if (verificar_archivo(path_pedido))
		{
			char *datos_obtenidos;
			char **datos_pedido;
			char **datos_cortados;
			int bloque_inicial = obtener_initial_block(path_pedido);
			datos_obtenidos = obtener_datos_bloques(bloque_inicial);
			datos_pedido = string_split(datos_obtenidos, " ");
			for (int i = 0; i <= 4; i++)
			{
				datos_cortados = string_split(datos_pedido[i], "=");
				string_append(&datos_a_devolver, datos_cortados[1]);
				if (i < 4)
				{
					string_append(&datos_a_devolver, " ");
				}
				liberar_recibido(datos_cortados);
			}
			liberar_recibido(datos_pedido);
			free(datos_obtenidos);
		}
		else
		{
			error_show("EL PEDIDO NO EXISTE\n");
			string_append(&datos_a_devolver, "FAIL");
		}
		free(path_pedido);
	}
	else
	{
		error_show("RESTAURANTE NO EXISTE\n");
		string_append(&datos_a_devolver, "FAIL");
	}
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK--------------------------//
	return datos_a_devolver;
}

char *f_obtener_receta(char *nombre_receta)
{
	// ---------------------------MUTEX LOCK------------------------------//
	pthread_mutex_lock(&mutex);
	char *datos_a_devolver = string_new();
	char *path_receta = string_from_format("/Files/Recetas/%s.AFIP", nombre_receta);
	if (verificar_archivo(path_receta))
	{
		char *datos_obtenidos;
		char **datos_receta;
		char **datos_cortados;
		datos_obtenidos = obtener_datos_receta(nombre_receta);
		datos_receta = string_split(datos_obtenidos, " ");
		for (int i = 0; i <= 1; i++)
		{
			datos_cortados = string_split(datos_receta[i], "=");
			string_append(&datos_a_devolver, datos_cortados[1]);
			if (i < 1)
			{
				string_append(&datos_a_devolver, " ");
			}
			liberar_recibido(datos_cortados);
		}
		liberar_recibido(datos_receta);
		free(datos_obtenidos);
	}
	else
	{
		error_show("LA RECETA NO EXISTE\n");
		string_append(&datos_a_devolver, "FAIL");
	}
	free(path_receta);
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK----------------------------//
	return datos_a_devolver;
}

char *f_terminar_pedido(char *nombre_restaurante, int pedido_ID)
{
	// ---------------------------MUTEX LOCK--------------------------------//
	pthread_mutex_lock(&mutex);
	char *ret = string_new();
	if (verificar_restaurante(nombre_restaurante) == 1)
	{
		char *pedido = string_from_format("/Files/Restaurantes/%s/Pedido%d.AFIP", nombre_restaurante, pedido_ID);
		if (verificar_archivo(pedido))
		{
			char *datos_pedido = obtener_datos_pedido(nombre_restaurante, pedido_ID);
			char **array_datos_pedido = string_split(datos_pedido, " ");

			if (strcmp(array_datos_pedido[0], "ESTADO_PEDIDO=Confirmado") == 0)
			{
				int pos_bloque_pedido = obtener_initial_block(pedido);

				int length_estado_pedido = string_length(array_datos_pedido[0]);

				char *nuevos_datos_pedido = string_new();
				string_append(&nuevos_datos_pedido, "ESTADO_PEDIDO=Terminado");
				char *otros_datos_pedido = string_substring_from(datos_pedido, length_estado_pedido);
				string_append(&nuevos_datos_pedido, otros_datos_pedido);

				char *bloques_ocupados = obtener_bloques_usados(pos_bloque_pedido);

				eliminar_bloques(bloques_ocupados);

				int tamanio_texto = string_length(nuevos_datos_pedido);
				int bloque_inicial = escribir_bloque(nuevos_datos_pedido);

				char *tamanio_datos = string_itoa(tamanio_texto);
				char *bloque = string_itoa(bloque_inicial);

				agregar_datos_config(pedido, "SIZE", tamanio_datos);
				agregar_datos_config(pedido, "INITIAL_BLOCK", bloque);

				free(bloques_ocupados);
				free(tamanio_datos);
				free(bloque);

				free(otros_datos_pedido);
				free(nuevos_datos_pedido);

				string_append(&ret, "OK");
			}
			else
			{
				error_show("EL PEDIDO NO ESTA CONFIRMADO\n");
				string_append(&ret, "FAIL");
			}			
			free(datos_pedido);
			liberar_recibido(array_datos_pedido);
		}
		else
		{
			error_show("PEDIDO NO EXISTE\n");
			string_append(&ret, "FAIL");
		}
		free(pedido);
	}
	else
	{
		error_show("RESTAURANTE NO EXISTE\n");
		string_append(&ret, "FAIL");
	}
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK----------------------------//
	return ret;
}

char *f_guardar_plato(char *nombre_restaurante, int pedido_ID, char *nombre_plato, int cantidad_plato_agregar)
{
	// ---------------------------MUTEX LOCK--------------------------------//
	pthread_mutex_lock(&mutex);
	char *ret = string_new();
	char *pedido = string_from_format("/Files/Restaurantes/%s/Pedido%d.AFIP", nombre_restaurante, pedido_ID);
	if (verificar_restaurante(nombre_restaurante))
	{
		if (verificar_archivo(pedido))
		{
			char *datos_pedido = obtener_datos_pedido(nombre_restaurante, pedido_ID);
			char **array_datos_pedido = string_split(datos_pedido, " ");
			if (strcmp(array_datos_pedido[0], "ESTADO_PEDIDO=Pendiente") == 0)
			{
				char **array_platos = string_split(array_datos_pedido[1], "=");
				char **array_cantidad_platos = string_split(array_datos_pedido[2], "=");
				char **array_precio = string_split(array_datos_pedido[4], "=");
				char **array_cant_lista = string_split(array_datos_pedido[3], "=");

				char *platos_string = string_new();
				string_append(&platos_string, array_platos[1]);

				char *cantidad_string = string_new();
				string_append(&cantidad_string, array_cantidad_platos[1]);

				char *cant_lista = string_new();
				string_append(&cant_lista, array_cant_lista[1]);

				int precio_total = atoi(array_precio[1]);

				char **platos = string_get_string_as_array(platos_string);
				char **cantidad_platos = string_get_string_as_array(cantidad_string);

				if (string_contains(platos_string, nombre_plato))
				{

					//El plato esta en el pedido
					precio_total += (obtener_precio_plato(nombre_restaurante, nombre_plato) * cantidad_plato_agregar);
					int i = 0;
					int posicion_array = -1;
					while (platos[i] != NULL)
					{
						if (strcmp(platos[i], nombre_plato) == 0)
						{
							posicion_array = i;
						}
						i++;
					}
					int cantidad = (atoi(cantidad_platos[posicion_array])) + cantidad_plato_agregar;
					free(cantidad_platos[posicion_array]);
					cantidad_platos[posicion_array] = string_itoa(cantidad);

					char *nueva_cantidad = de_array_a_string(cantidad_platos);
					char *nuevos_datos = string_from_format("%s %s CANTIDAD_PLATOS=%s %s PRECIO_TOTAL=%d", array_datos_pedido[0], array_datos_pedido[1], nueva_cantidad, array_datos_pedido[3], precio_total);

					int pos_bloque_pedido = obtener_initial_block(pedido);
					char *bloques_ocupados = obtener_bloques_usados(pos_bloque_pedido);
					eliminar_bloques(bloques_ocupados);

					int tamanio_texto = string_length(nuevos_datos);
					int bloque_inicial = escribir_bloque(nuevos_datos);

					char *tamanio_datos = string_itoa(tamanio_texto);
					char *bloque = string_itoa(bloque_inicial);

					agregar_datos_config(pedido, "SIZE", tamanio_datos);
					agregar_datos_config(pedido, "INITIAL_BLOCK", bloque);

					free(bloque);
					free(tamanio_datos);
					free(bloques_ocupados);
					free(nueva_cantidad);
					free(nuevos_datos);
					string_append(&ret, "OK");
				}
				else
				{
					//El plato NO ESTA en el pedido

					int precio_plato = obtener_precio_plato(nombre_restaurante, nombre_plato);
					if (precio_plato > -1)
					{

						char *cantidad_plato_agr = string_itoa(cantidad_plato_agregar);

						precio_total += (obtener_precio_plato(nombre_restaurante, nombre_plato) * cantidad_plato_agregar);

						char *nueva_cant_listos = agregar_elemento(cant_lista, "0");
						char *nueva_cantidad_platos = agregar_elemento(cantidad_string, cantidad_plato_agr);
						char *nuevos_platos = agregar_elemento(platos_string, nombre_plato);

						char *nuevos_datos_pedido = string_from_format("%s LISTA_PLATOS=%s CANTIDAD_PLATOS=%s CANTIDAD_LISTA=%s PRECIO_TOTAL=%d", array_datos_pedido[0], nuevos_platos, nueva_cantidad_platos, nueva_cant_listos, precio_total);

						int pos_bloque_pedido = obtener_initial_block(pedido);
						char *bloques_ocupados_pedido = obtener_bloques_usados(pos_bloque_pedido);
						eliminar_bloques(bloques_ocupados_pedido);

						int tamanio_texto = string_length(nuevos_datos_pedido);
						int bloque_inicial = escribir_bloque(nuevos_datos_pedido);

						char *tamanio_datos_pedido = string_itoa(tamanio_texto);
						char *bloque_pedido = string_itoa(bloque_inicial);

						agregar_datos_config(pedido, "SIZE", tamanio_datos_pedido);
						agregar_datos_config(pedido, "INITIAL_BLOCK", bloque_pedido);

						free(nueva_cant_listos);
						free(cantidad_plato_agr);
						free(bloque_pedido);
						free(tamanio_datos_pedido);
						free(bloques_ocupados_pedido);
						free(nueva_cantidad_platos);
						free(nuevos_platos);
						free(nuevos_datos_pedido);
						string_append(&ret, "OK");
					}
					else
					{
						error_show("EL RESTAURANTE NO VENDE ESE PLATO\n");
						string_append(&ret, "FAIL");
					}
				}
				liberar_recibido(array_cant_lista);
				liberar_recibido(array_precio);
				liberar_recibido(cantidad_platos);
				liberar_recibido(platos);
				liberar_recibido(array_cantidad_platos);
				liberar_recibido(array_platos);
				free(cant_lista);
				free(cantidad_string);
				free(platos_string);
			}
			else
			{
				error_show("EL ESTADO DEL PEDIDO NO ES PENDIENTE\n");
				string_append(&ret, "FAIL");
			}
			free(datos_pedido);
			liberar_recibido(array_datos_pedido);
		}
		else
		{
			error_show("EL PEDIDO NO EXISTE\n");
			string_append(&ret, "FAIL");
		}
	}
	else
	{
		error_show("RESTAURANTE NO EXISTE\n");
		string_append(&ret, "FAIL");
	}
	free(pedido);
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK--------------------------------//
	return ret;
}

char *f_plato_listo(char *nombre_restaurante, int pedido_ID, char *nombre_plato)
{
	// ---------------------------MUTEX LOCK--------------------------------//
	pthread_mutex_lock(&mutex);
	char *ret = string_new();
	char *ruta = string_from_format("/Files/Restaurantes/%s/Pedido%d.AFIP", nombre_restaurante, pedido_ID);
	if (verificar_restaurante(nombre_restaurante))
	{
		if (verificar_archivo(ruta))
		{
			char *datos_pedido = obtener_datos_pedido(nombre_restaurante, pedido_ID);
			char **array_datos_pedido = string_split(datos_pedido, " ");
			char *nuevos_datos = string_new();

			if (strcmp(array_datos_pedido[0], "ESTADO_PEDIDO=Confirmado") == 0)
			{
				char **array_platos = string_split(array_datos_pedido[1], "=");
				char *platos_string = string_new();
				string_append(&platos_string,array_platos[1]);
				char **platos = string_get_string_as_array(platos_string);

				int pos_plato = 0;
				int contiene_plato = 0;
				while (strcmp(platos[pos_plato], nombre_plato) != 0)
				{
					pos_plato++;
				}
				if (strcmp(platos[pos_plato], nombre_plato) == 0)
				{
					contiene_plato = 1;
				}

				if (contiene_plato == 1)
				{
					char **array_cantidad_lista = string_split(array_datos_pedido[3], "=");
					char *cantidad_lista_string = string_new();
					string_append(&cantidad_lista_string,array_cantidad_lista[1]);
					char **cantidad_lista = string_get_string_as_array(cantidad_lista_string);

					int int_cantidad_lista = atoi(cantidad_lista[pos_plato]);
					int_cantidad_lista++;
					char *cantidad_actualizada = string_itoa(int_cantidad_lista);

					//cambiar cantidad_lista
					char *nueva_cantidad_lista = string_new();

					string_append(&nueva_cantidad_lista, "[");
					int contador = 0;
					while (contador < pos_plato)
					{
						string_append(&nueva_cantidad_lista, cantidad_lista[contador]);
						string_append(&nueva_cantidad_lista, ",");
						contador++;
					}
					string_append(&nueva_cantidad_lista, cantidad_actualizada);
					contador++;
					if (cantidad_lista[contador] != NULL)
					{
						string_append(&nueva_cantidad_lista, ",");
					}
					while (cantidad_lista[contador] != NULL)
					{
						string_append(&nueva_cantidad_lista, cantidad_lista[contador]);
						if (cantidad_lista[contador + 1] != NULL)
						{
							string_append(&nueva_cantidad_lista, ",");
						}
						contador++;
					}
					string_append(&nueva_cantidad_lista, "]");

					//cambiar bloques

					string_append(&nuevos_datos, "ESTADO_PEDIDO=Confirmado ");
					string_append(&nuevos_datos, array_datos_pedido[1]);
					string_append(&nuevos_datos, " ");
					string_append(&nuevos_datos, array_datos_pedido[2]);
					string_append(&nuevos_datos, " CANTIDAD_LISTA=");
					string_append(&nuevos_datos, nueva_cantidad_lista);
					string_append(&nuevos_datos, " ");
					string_append(&nuevos_datos, array_datos_pedido[4]);

					int pos_bloque_pedido = obtener_initial_block(ruta);

					char *bloques_ocupados = obtener_bloques_usados(pos_bloque_pedido);

					eliminar_bloques(bloques_ocupados);

					int tamanio_texto = string_length(nuevos_datos);
					int bloque_inicial = escribir_bloque(nuevos_datos);

					char *tamanio_datos = string_itoa(tamanio_texto);
					char *bloque = string_itoa(bloque_inicial);

					agregar_datos_config(ruta, "SIZE", tamanio_datos);
					agregar_datos_config(ruta, "INITIAL_BLOCK", bloque);

					free(cantidad_lista_string);
					free(cantidad_actualizada);
					free(nueva_cantidad_lista);
					free(bloques_ocupados);
					free(tamanio_datos);
					free(bloque);
					liberar_recibido(array_cantidad_lista);
					liberar_recibido(cantidad_lista);

					string_append(&ret, "OK");
				}
				else
				{
					error_show("NO EXISTE EL PLATO INDICADO\n");
					string_append(&ret, "FAIL");
				}
				liberar_recibido(array_platos);
				liberar_recibido(platos);
				free(platos_string);
			}
			else
			{
				error_show("EL PEDIDO NO ESTA CONFIRMADO\n");
				string_append(&ret, "FAIL");
			}

			liberar_recibido(array_datos_pedido);
			free(datos_pedido);
			free(nuevos_datos);
		}
		else
		{
			error_show("NO EXISTE EL PEDIDO\n");
			string_append(&ret, "FAIL");
		}
	}
	else
	{
		error_show("NO EXISTE EL RESTAURANTE");
		string_append(&ret, "FAIL");
	}
	free(ruta);
	pthread_mutex_unlock(&mutex);
	// ---------------------------MUTEX UNLOCK--------------------------------//
	return ret;
}