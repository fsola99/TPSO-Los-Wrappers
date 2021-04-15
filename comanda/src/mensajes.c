#include "comanda.h"

char *f_guardar_pedido(char* nombre_restaurante, uint32_t id_pedido)
{

	char *resultado = string_new();
	t_restaurante* resto = buscar_restaurante(nombre_restaurante);

	if (resto != NULL) //Si existe el restaurante,
	{		
		t_pedido* pedido = buscar_pedido(resto->lista_pedidos, id_pedido);
		if (pedido != NULL) //Si existe el pedido,
		{
			printf("Ya existe el pedido\n");
			string_append(&resultado, "FAIL");
		}
		else //Si no existe el pedido,
		{
			t_pedido *nuevo_pedido = malloc(sizeof(t_pedido));
			t_list *paginas = list_create();
			nuevo_pedido->id_pedido = id_pedido;
			nuevo_pedido->lista_platos = paginas;
			nuevo_pedido->estado = string_new();
			string_append(&nuevo_pedido->estado,"Pendiente");
			
			pthread_mutex_lock(&mutex_lista_pedidos);
			list_add(resto->lista_pedidos, nuevo_pedido); //ZC
			pthread_mutex_unlock(&mutex_lista_pedidos);
			
			printf("Se agrego el nuevo pedido\n");
			string_append(&resultado, "OK");
		}
	}
	else //Si no existe el restaurante,
	{
		resto = malloc(sizeof(t_restaurante));
		t_list *paginas = list_create();
		t_list *pedidos = list_create();
		t_pedido *pedido = malloc(sizeof(t_pedido));
		pedido->id_pedido = id_pedido;
		
		pedido->lista_platos = paginas;
		
		pedido->estado = string_new();
		string_append(&pedido->estado,"Pendiente");
		
		pthread_mutex_lock(&mutex_lista_pedidos);
		list_add(pedidos, pedido); //ZC
		pthread_mutex_unlock(&mutex_lista_pedidos);
		
		resto->nombre = string_new();
		string_append(&resto->nombre,nombre_restaurante);
		
		resto->lista_pedidos = pedidos;

		pthread_mutex_lock(&mutex_lista_restaurantes);
		list_add(restaurantes, resto); //ZC
		pthread_mutex_unlock(&mutex_lista_restaurantes);
		
		if (resto->lista_pedidos != NULL)
		{
			printf("Se creo la tabla de segmentos\n");
			string_append(&resultado, "OK");
		}
		else
		{	
			string_append(&resultado, "FAIL");
		}
	}
	return resultado;
}

char *f_guardar_plato(char *nombre_restaurante, uint32_t id_pedido, char *nombre_plato, uint32_t cantidad)
{

	char *resultado = string_new();
	
	t_restaurante* resto = buscar_restaurante(nombre_restaurante);
	if (strlen(nombre_plato) > 24)
	{
		printf("El nombre del plato es demasiado largo\n");
		string_append(&resultado, "FAIL");
		return resultado;
	}
	if (resto != NULL)
	{ //Si existe el restaurante,
		
		if (resto->lista_pedidos == NULL)
		{ //Si no existe la tabla de segmentos,

			printf("No existe la tabla de segmentos\n");
			string_append(&resultado, "FAIL");
		}
		else
		{ //Si existe la tabla de segmentos,

			
			t_pedido* pedido = buscar_pedido(resto->lista_pedidos, id_pedido);
			
			if (pedido != NULL)
			{ //Si existe el pedido,

				if (!strcmp(pedido->estado, "Pendiente"))
				{	
					pthread_mutex_lock(&mutex_lista_platos);
					int size_lista_platos = list_size(pedido->lista_platos); //ZC
					pthread_mutex_unlock(&mutex_lista_platos);

					if (!size_lista_platos) //Si el pedido esta vacio,
					{
						int pagina_agregada = agregar_pagina(nombre_plato, pedido, cantidad);
						if (pagina_agregada == -1)
						{
							string_append(&resultado, "FAIL");
						} else {
							string_append(&resultado, "OK");
						}
					}
					else //Si el pedido no esta vacio,
					{
						int plato_en_pagina = verificar_plato_en_pagina(pedido, nombre_plato);
						if (plato_en_pagina == -1)
						{
							int pagina_agregada = agregar_pagina(nombre_plato, pedido, cantidad);
							if (pagina_agregada == -1)
							{
								string_append(&resultado, "FAIL");
							}
							else
							{
								string_append(&resultado, "OK");
							}
						}
						else
						{
							modificar_cantidad(plato_en_pagina, cantidad, pedido);
							string_append(&resultado, "OK");
						}
					}
				}
				else
				{
					printf("El pedido debe estar en estado pendiente\n");
					string_append(&resultado, "FAIL");
				}
			}
			else
			{ //Si no existe el pedido
				printf("No existe el pedido\n");
				string_append(&resultado, "FAIL");
			}
		}
	}
	else
	{ //Si no existe el restaurante,
		printf("No existe el restaurante\n");
		string_append(&resultado, "FAIL");
	}
	printf("Recorro lista\n");
	//recorrer_lista();
	return resultado;
}

char *f_obtener_pedido(char *nombre_restaurante, uint32_t id_pedido)
{	
	t_restaurante *restaurante = buscar_restaurante(nombre_restaurante);
	char *resultado = string_new();
	if (restaurante != NULL)
	{
		if (restaurante->lista_pedidos == NULL)
		{
			printf("No existe la tabla de segmentos del restaurante: %s\n", nombre_restaurante);
			string_append(&resultado, "FAIL");
		}
		else
		{
			free(resultado);
			return obtener_datos_pedido(restaurante, id_pedido);
		}
	}
	else
	{
		printf("No existe el restaurante\n");
		string_append(&resultado, "FAIL");
	}
	//recorrer_lista();
	return resultado;
}

char *f_confirmar_pedido(char *nombre_restaurante, uint32_t id_pedido)
{
	t_restaurante *resto = buscar_restaurante(nombre_restaurante);
	char *resultado = string_new();

	//Verifico si existe el restaurante
	if (resto != NULL)
	{
		//Verifico si existe la tabla de segmentos
		if (resto->lista_pedidos == NULL)
		{
			printf("No existe la tabla de segmentos\n");
			string_append(&resultado, "FAIL");
		}
		else
		{
			t_pedido *pedido = buscar_pedido(resto->lista_pedidos, id_pedido);
			
			//Verifico si existe el segmento/pedido
			if (pedido != NULL)
			{
				if (!strcmp(pedido->estado, "Pendiente"))
				{
					pedido->estado = realloc(pedido->estado, strlen("Confirmado") + 1);
					strcpy(pedido->estado, "Confirmado");
					printf("Pedido %s\n", pedido->estado);
					string_append(&resultado, "OK");
				}
				else
				{
					printf("El pedido ya esta en estado Confirmado\n");
					string_append(&resultado, "FAIL");
				}
			}
			else
			{
				printf("No existe el pedido\n");
				string_append(&resultado, "FAIL");
			}
		}
	}
	else
	{
		printf("No se encontro el restaurante\n");
		string_append(&resultado, "FAIL");
	}
	return resultado;
}

char *f_plato_listo(char *nombre_restaurante, char *nombre_plato, uint32_t id_pedido)
{
	t_restaurante *resto = buscar_restaurante(nombre_restaurante);
	
	char *resultado = string_new();

	//Verifico si existe el restaurante
	if (resto != NULL)
	{
		//Verifico si existe la tabla de segmentos
		if (resto->lista_pedidos == NULL)
		{
			printf("No existe la tabla de segmentos\n");
			string_append(&resultado, "FAIL");
		}
		else
		{
			t_pedido *pedido = buscar_pedido(resto->lista_pedidos, id_pedido);
			//Verifico si existe el segmento/pedido
			if (pedido != NULL)
			{
				if (!strcmp(pedido->estado, "Confirmado"))
				{
					int plato_en_pagina = verificar_plato_en_pagina(pedido, nombre_plato);
					if (plato_en_pagina != -1)
					{
						modificar_cantidad_lista(plato_en_pagina, 1, pedido);
						bool ped_terminado = pedido_terminado(nombre_restaurante, pedido);
						if (ped_terminado)
						{
							memset(pedido->estado, 0, strlen(pedido->estado));
							string_append(&pedido->estado, "Terminado");
							printf("Pedido %s\n", pedido->estado);
						}
						string_append(&resultado, "OK");
					}
					else
					{
						printf("No existe el plato en este pedido\n");
						string_append(&resultado, "FAIL");
					}
				}
				else
				{
					printf("El pedido no esta confirmado\n");
					string_append(&resultado, "FAIL");
				}
			}
			else
			{
				printf("No existe el pedido\n");
				string_append(&resultado, "FAIL");
			}
		}
	}
	else
	{
		printf("No se encontro el restaurante\n");
		string_append(&resultado, "FAIL");
	}
	//recorrer_lista();
	return resultado;
}

char *f_finalizar_pedido(char *nombre_restaurante, uint32_t id_pedido)
{
	t_restaurante *resto = buscar_restaurante(nombre_restaurante);
	char *resultado = string_new();

	//Verifico si existe el restaurante
	if (resto != NULL)
	{
		//Verifico si existe la tabla de segmentos
		if (resto->lista_pedidos == NULL)
		{
			printf("No existe la tabla de segmentos\n");
			string_append(&resultado, "FAIL");
		}
		else
		{
			t_pedido *pedido = buscar_pedido(resto->lista_pedidos, id_pedido);
			//Verifico si existe el segmento/pedido
			if (pedido != NULL)
			{
				eliminar_paginas_memoria(pedido->lista_platos);
				eliminar_paginas_lista(pedido->lista_platos);
				int pos_pedido = buscar_posicion_pedido(resto->lista_pedidos,pedido);
				list_remove_and_destroy_element(resto->lista_pedidos, pos_pedido, liberar_pedido);
				printf("Pedido finalizado\n");
				string_append(&resultado, "OK");
			}
			else
			{
				printf("No existe el pedido\n");
				string_append(&resultado, "FAIL");
			}
		}
	}
	else
	{
		printf("No se encontro el restaurante\n");
		string_append(&resultado, "FAIL");
	}
	return resultado;
}