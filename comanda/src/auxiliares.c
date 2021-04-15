#include "comanda.h"

//Funciones para buscar
t_restaurante *buscar_restaurante(char *nombre_restaurante)
{
	pthread_mutex_lock(&mutex_lista_restaurantes);
	t_restaurante *r = NULL;
	for (int i = 0; i < list_size(restaurantes); i++)
	{
		r = list_get(restaurantes, i);
		if (!strcmp(nombre_restaurante, r->nombre))
			break;
		r = NULL;
	}
	pthread_mutex_unlock(&mutex_lista_restaurantes);
	return r;
}

t_pedido *buscar_pedido(t_list *pedidos, uint32_t id_pedido)
{
	pthread_mutex_lock(&mutex_lista_pedidos);
	t_pedido *p = NULL;
	for (int i = 0; i < list_size(pedidos); i++)
	{
		p = list_get(pedidos, i);
		uint32_t id_pedido_2 = p->id_pedido;
		if (id_pedido == id_pedido_2)
			break;
		p = NULL;
	}
	pthread_mutex_unlock(&mutex_lista_pedidos);
	return p;
}
int buscar_posicion_pedido(t_list *pedidos, t_pedido *pedido)
{
	int resultado = -1;
	pthread_mutex_lock(&mutex_lista_pedidos);
	t_pedido *p = NULL;
	for (int i = 0; i < list_size(pedidos); i++)
	{
		p = list_get(pedidos, i);
		if (p->id_pedido == pedido->id_pedido)
		{
			resultado = i;
			break;
		}
		p = NULL;
	}
	pthread_mutex_unlock(&mutex_lista_pedidos);
	return resultado;
}
int buscar_auxiliar(int pos_swap)
{
	int resultado = -1;
	pthread_mutex_lock(&mutex_auxiliar);
	t_auxiliar *a = NULL;
	for (int i = 0; i < list_size(lista_aux); i++)
	{
		a = list_get(lista_aux, i);
		if (a->frame_swap == pos_swap)
		{
			resultado = i;
			break;
		}
		a = NULL;
	}
	pthread_mutex_unlock(&mutex_auxiliar);
	return resultado;
}
t_auxiliar *buscar_auxiliar_2(int pos_memoria)
{
	pthread_mutex_lock(&mutex_auxiliar);
	t_auxiliar *a = NULL;
	for (int i = 0; i < list_size(lista_aux); i++)
	{
		a = list_get(lista_aux, i);
		if (a->frame_memoria == pos_memoria && a->presente)
		{
			break;
		}
		a = NULL;
	}
	pthread_mutex_unlock(&mutex_auxiliar);
	return a;
}

int buscar_plato_vacio(int bitarray[], int tamanio)
{
	int i = 0;
	while (i < tamanio)
	{
		if (bitarray[i] == 0)
		{
			return i;
		}
		i++;
	}
	return -1;
}

int buscar_plato_vacio_memoria()
{
	pthread_mutex_lock(&mutex_memoria);
	int q = buscar_plato_vacio(bitarray_memoria, tamanio_memoria);
	pthread_mutex_unlock(&mutex_memoria);
	return q;
}

int buscar_plato_vacio_swap()
{
	pthread_mutex_lock(&mutex_swap);
	int q = buscar_plato_vacio(bitarray_swap, tamanio_swap);
	pthread_mutex_unlock(&mutex_swap);
	return q;
}

//Funciones de agregar/verificar/modificar paginas y obtener pedido

int verificar_plato_en_pagina(t_pedido *pedido, char *nombre_plato)
{
	int resultado = -1;
	bool encontrado = false;
	int i = 0;
	pthread_mutex_lock(&mutex_lista_platos);
	t_pagina *pagina = list_get(pedido->lista_platos, i);
	while (i < list_size(pedido->lista_platos))
	{
		if (pagina->auxiliar->presente && !pagina->auxiliar->comparado)
		{
			pthread_mutex_lock(&mutex_memoria);
			int comparar = strcmp(memoria_principal[pagina->frame_memoria].nombre, nombre_plato);
			pthread_mutex_unlock(&mutex_memoria);
			modificar_auxiliar(pagina->auxiliar, pagina->auxiliar->frame_memoria, pagina->auxiliar->frame_swap, pagina->auxiliar->presente, pagina->auxiliar->modificado, true, true);
			if (!comparar)
			{
				resultado = i;
				encontrado = true;
				break;
			}
		}
		i++;
		pagina = list_get(pedido->lista_platos, i);
	}
	pthread_mutex_unlock(&mutex_lista_platos);

	int j = 0;
	pthread_mutex_lock(&mutex_lista_platos);
	pagina = list_get(pedido->lista_platos, j);
	while (j < list_size(pedido->lista_platos) && !encontrado)
	{
		if (!pagina->auxiliar->presente && !pagina->auxiliar->comparado)
		{
			int pos_plato_vacio = buscar_plato_vacio_memoria();
			if (pos_plato_vacio != -1)
			{
				pthread_mutex_lock(&mutex_swap);
				t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
				pthread_mutex_unlock(&mutex_swap);
				agregar_plato_memoria(plato, pos_plato_vacio);
				ocupar_frame_memoria(pos_plato_vacio);
				log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_vacio);
				pthread_mutex_lock(&mutex_memoria);
				int comparar = strcmp(memoria_principal[pos_plato_vacio].nombre, nombre_plato);
				pthread_mutex_unlock(&mutex_memoria);

				modificar_auxiliar(pagina->auxiliar, pos_plato_vacio, pagina->auxiliar->frame_swap, true, pagina->auxiliar->modificado, true, true);
				pagina->frame_memoria = pos_plato_vacio;
				if (!comparar)
				{
					resultado = j;
					break;
				}
			}
			else
			{
				int pos_plato_movido = mover_plato_a_swap();
				pthread_mutex_lock(&mutex_swap);
				t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
				pthread_mutex_unlock(&mutex_swap);
				agregar_plato_memoria(plato, pos_plato_movido);
				log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_movido);
				pthread_mutex_lock(&mutex_memoria);
				int comparar = strcmp(memoria_principal[pos_plato_movido].nombre, nombre_plato);
				pthread_mutex_unlock(&mutex_memoria);
				modificar_auxiliar(pagina->auxiliar, pos_plato_movido, pagina->auxiliar->frame_swap, true, pagina->auxiliar->modificado, true, true);
				pagina->frame_memoria = pos_plato_movido;
				if (!comparar)
				{
					resultado = j;
					break;
				}
			}
		}
		j++;
		pagina = list_get(pedido->lista_platos, j);
	}
	reiniciar_comparado(pedido->lista_platos);
	pthread_mutex_unlock(&mutex_lista_platos);
	return resultado;
}

int agregar_pagina(char *nombre_plato, t_pedido *pedido, int cantidad)
{
	int resultado = 1;
	int pos_plato_vacio_swap = buscar_plato_vacio_swap();
	if (pos_plato_vacio_swap != -1)
	{ //Si hay lugar en swap,
		//Agrego el nuevo plato a memoria swap,
		t_plato nuevo_plato;
		strcpy(nuevo_plato.nombre, nombre_plato);
		nuevo_plato.cantidad = cantidad;
		nuevo_plato.cantidad_lista = 0;

		agregar_plato_swap(nuevo_plato, pos_plato_vacio_swap);
		ocupar_frame_swap(pos_plato_vacio_swap);

		t_pagina *pagina = malloc(sizeof(t_pagina));
		pthread_mutex_lock(&mutex_lista_platos);
		pagina->indice = list_size(pedido->lista_platos);
		pthread_mutex_unlock(&mutex_lista_platos);

		t_auxiliar *auxiliar = malloc(sizeof(t_auxiliar));
		int pos_plato_vacio_memoria = buscar_plato_vacio_memoria();

		if (pos_plato_vacio_memoria != -1)
		{ //Si hay lugar en memoria principal,
			//nuevo_plato.cantidad = cantidad;

			agregar_plato_memoria(nuevo_plato, pos_plato_vacio_memoria);
			ocupar_frame_memoria(pos_plato_vacio_memoria);
			log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", nuevo_plato.nombre, pos_plato_vacio_memoria);

			pagina->frame_memoria = pos_plato_vacio_memoria;
			list_add(lista_aux, auxiliar);
			
			if (!strcmp(algoritmo_reemplazo, "LRU")) {
				modificar_auxiliar(auxiliar, pos_plato_vacio_memoria, pos_plato_vacio_swap, true, true, auxiliar->comparado, true);
			} else {
				modificar_auxiliar(auxiliar, pos_plato_vacio_memoria, pos_plato_vacio_swap, true, false, auxiliar->comparado, true);
			}
		}
		else
		{ //Si no hay lugar en memoria principal,
			int pos_plato_movido = mover_plato_a_swap();
			//nuevo_plato.cantidad = cantidad;

			agregar_plato_memoria(nuevo_plato, pos_plato_movido);
			log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", nuevo_plato.nombre, pos_plato_movido);
			pagina->frame_memoria = pos_plato_movido;
			list_add(lista_aux, auxiliar);
			if (!strcmp(algoritmo_reemplazo, "LRU")) {
				modificar_auxiliar(auxiliar, pos_plato_movido, pos_plato_vacio_swap, true, true, auxiliar->comparado, true);
			} else {
				modificar_auxiliar(auxiliar, pos_plato_movido, pos_plato_vacio_swap, true, false, auxiliar->comparado, true);
			}
		}
		//Agrego la pagina a la tabla de paginas
		/*if (!strcmp(algoritmo_reemplazo, "LRU"))
		{
			pthread_mutex_lock(&mutex_auxiliar);
			list_add(lista_aux, auxiliar);
			pthread_mutex_unlock(&mutex_auxiliar);
		}
		else
		{
			if (pos_plato_vacio_memoria != -1)
			{
				pthread_mutex_lock(&mutex_auxiliar);
				list_add(lista_aux, auxiliar);
				pthread_mutex_unlock(&mutex_auxiliar);
			}
			else
			{
				pthread_mutex_lock(&mutex_auxiliar);
				list_add_in_index(lista_aux, aguja_contador - 1, auxiliar);
				pthread_mutex_unlock(&mutex_auxiliar);
			}
		}*/
		pagina->auxiliar = auxiliar;
		pthread_mutex_lock(&mutex_lista_platos);
		list_add(pedido->lista_platos, pagina);
		pthread_mutex_unlock(&mutex_lista_platos);
	}
	else
	{ //Si no hay lugar en swap,
		log_info(logger, "No hay mas lugar en SWAP");
		resultado = -1;
	}
	return resultado;
}

void modificar_cantidad(int plato_en_pagina, int cantidad, t_pedido *pedido)
{
	pthread_mutex_lock(&mutex_lista_platos);
	t_pagina *pagina = list_get(pedido->lista_platos, plato_en_pagina);
	pthread_mutex_unlock(&mutex_lista_platos);
	if (pagina->auxiliar->presente)
	{
		pthread_mutex_lock(&mutex_memoria);
		t_plato plato = memoria_principal[pagina->frame_memoria];
		pthread_mutex_unlock(&mutex_memoria);
		plato.cantidad += cantidad;

		agregar_plato_memoria(plato, pagina->frame_memoria);
		//log_info(logger, "Se modifico el plato %s en Memoria Principal en la posicion %d\n", plato.nombre, pagina->frame_memoria);

		modificar_auxiliar(pagina->auxiliar, pagina->auxiliar->frame_memoria, pagina->auxiliar->frame_swap, pagina->auxiliar->presente, true, pagina->auxiliar->comparado, true);
	}
	else
	{

		int pos_plato_vacio_memoria = buscar_plato_vacio_memoria();

		pthread_mutex_lock(&mutex_swap);
		t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
		pthread_mutex_lock(&mutex_swap);

		plato.cantidad += cantidad;
		if (pos_plato_vacio_memoria != -1)
		{

			agregar_plato_memoria(plato, pos_plato_vacio_memoria);
			ocupar_frame_memoria(pos_plato_vacio_memoria);
			log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_vacio_memoria);
		}
		else
		{
			int pos_plato_movido = mover_plato_a_swap();
			agregar_plato_memoria(plato, pos_plato_movido);
			log_info(logger, "Se agrego el plato %s en Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_movido);
		}
		modificar_auxiliar(pagina->auxiliar, pagina->auxiliar->frame_memoria, pagina->auxiliar->frame_swap, true, true, pagina->auxiliar->comparado, true);
	}
}

void modificar_cantidad_lista(int plato_en_pagina, int cantidad_lista, t_pedido *pedido)
{
	pthread_mutex_lock(&mutex_lista_platos);
	t_pagina *pagina = list_get(pedido->lista_platos, plato_en_pagina);
	pthread_mutex_unlock(&mutex_lista_platos);
	if (pagina->auxiliar->presente)
	{
		pthread_mutex_lock(&mutex_memoria);
		t_plato plato = memoria_principal[pagina->frame_memoria];
		pthread_mutex_unlock(&mutex_memoria);
		if (plato.cantidad_lista < plato.cantidad)
		{
			plato.cantidad_lista += cantidad_lista;
		}
		else
		{
			//printf("Cantidad tope\n");
		}
		agregar_plato_memoria(plato, pagina->frame_memoria);
		//log_info(logger, "Se modifico el plato %s en Memoria Principal en la posicion %d\n", plato.nombre, pagina->frame_memoria);
		modificar_auxiliar(pagina->auxiliar, pagina->auxiliar->frame_memoria, pagina->auxiliar->frame_swap, true, true, pagina->auxiliar->comparado, true);
	}
	else
	{
		int pos_plato_vacio_memoria = buscar_plato_vacio_memoria();
		pthread_mutex_lock(&mutex_swap);
		t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
		pthread_mutex_unlock(&mutex_swap);
		if (plato.cantidad_lista < plato.cantidad)
		{
			plato.cantidad_lista += cantidad_lista;
		}
		else
		{
			//printf("Cantidad tope\n");
		}
		if (pos_plato_vacio_memoria != -1)
		{
			agregar_plato_memoria(plato, pos_plato_vacio_memoria);
			log_info(logger, "Se agrego el plato %s en Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_vacio_memoria);
			ocupar_frame_memoria(pos_plato_vacio_memoria);
		}
		else
		{
			int pos_plato_movido = mover_plato_a_swap();
			agregar_plato_memoria(plato, pos_plato_movido);
			log_info(logger, "Se agrego el plato %s en Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_movido);
		}
		/*if (strcmp(algoritmo_reemplazo, "LRU"))
		{
			if (pos_plato_vacio_memoria == -1)
			{
				int pos_auxiliar = buscar_auxiliar(pagina->auxiliar->frame_swap);
				pagina->auxiliar = NULL;
				t_auxiliar *aux = list_remove(lista_aux, pos_auxiliar);
				list_add_in_index(lista_aux, aguja_contador - 1, aux);
				pagina->auxiliar = aux;
			}
		}*/
		modificar_auxiliar(pagina->auxiliar, pagina->auxiliar->frame_memoria, pagina->auxiliar->frame_swap, true, true, pagina->auxiliar->comparado, true);
	}
}

bool pedido_terminado(char *nombre_restaurante, t_pedido *pedido)
{
	int i = 0;
	bool resultado = true;
	bool encontrado = false;
	pthread_mutex_lock(&mutex_lista_platos);
	t_pagina *pagina = list_get(pedido->lista_platos, i);
	while (i < list_size(pedido->lista_platos))
	{
		if (pagina->auxiliar->presente && !pagina->auxiliar->comparado)
		{
			pthread_mutex_lock(&mutex_memoria);
			t_plato plato = memoria_principal[pagina->frame_memoria];
			pthread_mutex_unlock(&mutex_memoria);
			if (plato.cantidad != plato.cantidad_lista)
			{
				resultado = false;
				encontrado = true;
				break;
			}
			modificar_auxiliar(pagina->auxiliar, pagina->auxiliar->frame_memoria, pagina->auxiliar->frame_swap, pagina->auxiliar->presente, pagina->auxiliar->modificado, true, true);
		}
		i++;
		pagina = list_get(pedido->lista_platos, i);
	}
	pthread_mutex_unlock(&mutex_lista_platos);

	int j = 0;
	pthread_mutex_lock(&mutex_lista_platos);
	pagina = list_get(pedido->lista_platos, j);
	while (j < list_size(pedido->lista_platos) && !encontrado)
	{
		if (!pagina->auxiliar->presente && !pagina->auxiliar->comparado)
		{
			int pos_plato_vacio = buscar_plato_vacio_memoria();
			if (pos_plato_vacio != -1)
			{
				pthread_mutex_lock(&mutex_swap);
				t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
				pthread_mutex_unlock(&mutex_swap);
				agregar_plato_memoria(plato, pos_plato_vacio);
				ocupar_frame_memoria(pos_plato_vacio);
				log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_vacio);
				if (plato.cantidad != plato.cantidad_lista)
				{
					resultado = false;
					break;
				}
				modificar_auxiliar(pagina->auxiliar, pos_plato_vacio, pagina->auxiliar->frame_swap, true, pagina->auxiliar->modificado, true, true);
				pagina->frame_memoria = pos_plato_vacio;
			}
			else
			{
				int pos_plato_movido = mover_plato_a_swap();
				pthread_mutex_lock(&mutex_swap);
				t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
				pthread_mutex_unlock(&mutex_swap);
				agregar_plato_memoria(plato, pos_plato_movido);
				log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_movido);
				if (plato.cantidad != plato.cantidad_lista)
				{
					resultado = false;
					break;
				}
				modificar_auxiliar(pagina->auxiliar, pos_plato_movido, pagina->auxiliar->frame_swap, true, pagina->auxiliar->modificado, true, true);
				pagina->frame_memoria = pos_plato_movido;
			}
			/*if (strcmp(algoritmo_reemplazo, "LRU"))
			{
				if (pos_plato_vacio == -1)
				{
					int pos_auxiliar = buscar_auxiliar(pagina->auxiliar->frame_swap);
					pagina->auxiliar = NULL;
					t_auxiliar *aux = list_remove(lista_aux, pos_auxiliar);
					list_add_in_index(lista_aux, aguja_contador - 1, aux);
					pagina->auxiliar = aux;
				}
			}*/
		}
		j++;
		pagina = list_get(pedido->lista_platos, j);
	}
	reiniciar_comparado(pedido->lista_platos);
	pthread_mutex_unlock(&mutex_lista_platos);
	return resultado;
}

char *obtener_datos_pedido(t_restaurante *restaurante, uint32_t id_pedido)
{
	char *resultado = string_new();
	t_pedido *pedido = buscar_pedido(restaurante->lista_pedidos, id_pedido);
	if (pedido != NULL)
	{
		string_append(&resultado, pedido->estado);
		string_append(&resultado, ",");
		int i = 0;
		pthread_mutex_lock(&mutex_lista_platos);
		t_pagina *pagina = list_get(pedido->lista_platos, i);
		while (i < list_size(pedido->lista_platos))
		{
			if (pagina->auxiliar->presente && !pagina->auxiliar->comparado)
			{
				pthread_mutex_lock(&mutex_memoria);
				t_plato plato = memoria_principal[pagina->frame_memoria];
				pthread_mutex_unlock(&mutex_memoria);
				char *cantidad = string_itoa(plato.cantidad);
				char *cantidad_lista = string_itoa(plato.cantidad_lista);
				string_append(&resultado, plato.nombre);
				string_append(&resultado, " ");
				string_append(&resultado, cantidad);
				free(cantidad);
				string_append(&resultado, " ");
				string_append(&resultado, cantidad_lista);
				free(cantidad_lista);
				string_append(&resultado, ",");
				modificar_auxiliar(pagina->auxiliar, pagina->auxiliar->frame_memoria, pagina->auxiliar->frame_swap, pagina->auxiliar->presente, pagina->auxiliar->modificado, true, true);
			}
			i++;
			pagina = list_get(pedido->lista_platos, i);
		}
		pthread_mutex_unlock(&mutex_lista_platos);

		int j = 0;
		pthread_mutex_lock(&mutex_lista_platos);
		pagina = list_get(pedido->lista_platos, j);
		while (j < list_size(pedido->lista_platos))
		{
			if (!pagina->auxiliar->presente && !pagina->auxiliar->comparado)
			{
				int pos_plato_vacio = buscar_plato_vacio_memoria();
				if (pos_plato_vacio != -1)
				{
					pthread_mutex_lock(&mutex_swap);
					t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
					pthread_mutex_unlock(&mutex_swap);
					agregar_plato_memoria(plato, pos_plato_vacio);
					log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_vacio);
					char *cantidad = string_itoa(plato.cantidad);
					char *cantidad_lista = string_itoa(plato.cantidad_lista);
					string_append(&resultado, plato.nombre);
					string_append(&resultado, " ");
					string_append(&resultado, cantidad);
					free(cantidad);
					string_append(&resultado, " ");
					string_append(&resultado, cantidad_lista);
					free(cantidad_lista);
					string_append(&resultado, ",");
					modificar_auxiliar(pagina->auxiliar, pos_plato_vacio, pagina->auxiliar->frame_swap, true, pagina->auxiliar->modificado, true, true);
					pagina->frame_memoria = pos_plato_vacio;
				}
				else
				{
					int pos_plato_movido = mover_plato_a_swap();
					pthread_mutex_lock(&mutex_swap);
					t_plato plato = memoria_swap[pagina->auxiliar->frame_swap];
					pthread_mutex_unlock(&mutex_swap);
					agregar_plato_memoria(plato, pos_plato_movido);
					log_info(logger, "Se agrego el plato %s a Memoria Principal en la posicion %d\n", plato.nombre, pos_plato_movido);
					char *cantidad = string_itoa(plato.cantidad);
					char *cantidad_lista = string_itoa(plato.cantidad_lista);
					string_append(&resultado, plato.nombre);
					string_append(&resultado, " ");
					string_append(&resultado, cantidad);
					free(cantidad);
					string_append(&resultado, " ");
					string_append(&resultado, cantidad_lista);
					free(cantidad_lista);
					string_append(&resultado, ",");
					modificar_auxiliar(pagina->auxiliar, pos_plato_movido, pagina->auxiliar->frame_swap, true, pagina->auxiliar->modificado, true, true);
					pagina->frame_memoria = pos_plato_movido;
				}
			}
			j++;
			pagina = list_get(pedido->lista_platos, j);
		}
		reiniciar_comparado(pedido->lista_platos);
		pthread_mutex_unlock(&mutex_lista_platos);
	}
	else
	{
		printf("No existe el pedido\n");
		string_append(&resultado, "FAIL");
	}
	//recorrer_lista();
	return resultado;
}

//Funciones de algoritmos de reemplazo

int mover_plato_a_swap()
{
	log_info(logger, "Comienzo de reemplazo de pagina\n");
	int encontrado = -1;
	if (!strcmp(algoritmo_reemplazo, "LRU"))
	{
		int i = 0;
		pthread_mutex_lock(&mutex_auxiliar);
		list_sort(lista_aux, (*comparador));
		t_auxiliar *auxiliar = list_get(lista_aux, i);
		while (i < list_size(lista_aux))
		{
			if (auxiliar->presente)
			{
				if (auxiliar->modificado)
				{
					pthread_mutex_lock(&mutex_memoria);
					t_plato plato = memoria_principal[auxiliar->frame_memoria];
					pthread_mutex_unlock(&mutex_memoria);
					log_info(logger, "Victima seleccionada para reemplazo: %s\n", plato.nombre);
					pthread_mutex_lock(&mutex_swap);
					memoria_swap[auxiliar->frame_swap] = plato;
					pthread_mutex_unlock(&mutex_swap);
				}
				pthread_mutex_unlock(&mutex_auxiliar);
				modificar_auxiliar(auxiliar, auxiliar->frame_memoria, auxiliar->frame_swap, false, false, auxiliar->comparado, auxiliar->utilizado);
				pthread_mutex_lock(&mutex_auxiliar);
				encontrado = auxiliar->frame_memoria;
				break;
			}
			i++;
			auxiliar = list_get(lista_aux, i);
		}
		pthread_mutex_unlock(&mutex_auxiliar);
	}
	else
	{ //Si es clock mejorado,

		/*if (aguja_contador == 0)
		{
			pthread_mutex_lock(&mutex_auxiliar);
			//aguja = &memoria_principal[0];
			aguja_contador = 0;
			pthread_mutex_unlock(&mutex_auxiliar);
		}*/

		//Paso 1
		while (encontrado == -1)
		{
			encontrado = paso_uno_clock();
			if (encontrado == -1)
			{
				encontrado = paso_dos_clock();
			}
			else
			{
				break;
			}
		}
	}
	return encontrado;
}
bool comparador(void *aux1, void *aux2)
{
	t_auxiliar *auxiliar1 = (t_auxiliar *)aux1;
	t_auxiliar *auxiliar2 = (t_auxiliar *)aux2;
	return auxiliar1->timestamp < auxiliar2->timestamp;
}
void modificar_auxiliar(t_auxiliar *auxiliar, int pos_memoria, int pos_swap, bool presente, bool modificado, bool comparado, bool utilizado)
{
	pthread_mutex_lock(&mutex_auxiliar);
	auxiliar->timestamp = timestamp();
	auxiliar->presente = presente;
	auxiliar->modificado = modificado;
	auxiliar->comparado = comparado;
	auxiliar->frame_swap = pos_swap;
	auxiliar->frame_memoria = pos_memoria;
	auxiliar->utilizado = utilizado;
	pthread_mutex_unlock(&mutex_auxiliar);
}
uint64_t timestamp()
{
	/*struct timeval valor;
	gettimeofday(&valor, NULL);
	unsigned long long result = (((unsigned long long)valor.tv_sec) * 1000 + ((unsigned long)valor.tv_usec));
	uint64_t tiempo = result;*/
	
	struct timespec tms;
	/* POSIX.1-2008 way */
    if (clock_gettime(CLOCK_REALTIME,&tms)) {
        return -1;
    }
    /* seconds, multiplied with 1 million */
    uint64_t micros = tms.tv_sec * 1000000;
    /* Add full microseconds */
    micros += tms.tv_nsec/1000;
    /* round up if necessary */
    if (tms.tv_nsec % 1000 >= 500) {
        ++micros;
    }
	
	return micros;
}
int paso_uno_clock()
{
	int i = 0;
	int encontrado = -1;
	//pthread_mutex_lock(&mutex_auxiliar);
	while (i < tamanio_memoria)
	{
		//pthread_mutex_lock(&mutex_auxiliar);
		t_auxiliar *aux = buscar_auxiliar_2(aguja_contador);
		//pthread_mutex_unlock(&mutex_auxiliar);
		if (!aux->utilizado && !aux->modificado && aux->presente)
		{
			encontrado = aux->frame_memoria;
			pthread_mutex_lock(&mutex_memoria);
			t_plato plato = memoria_principal[aux->frame_memoria];
			pthread_mutex_unlock(&mutex_memoria);
			log_info(logger, "Victima seleccionada para reemplazo: %s\n", plato.nombre);
			printf("Victima %s\n", plato.nombre);
			//pthread_mutex_lock(&mutex_auxiliar);
			modificar_auxiliar(aux, aux->frame_memoria, aux->frame_swap, false, false, aux->comparado, false);
			//pthread_mutex_unlock(&mutex_auxiliar);
			if (tamanio_memoria == (aguja_contador + 1))
			{
				aguja_contador = -1;
			}
			aguja_contador++;
			//aguja = &memoria_principal[aguja_contador];

			break;
		}
		if (tamanio_memoria == (aguja_contador + 1))
		{
			aguja_contador = -1;
		}
		aguja_contador++;
		i++;
		//aguja = &memoria_principal[aguja_contador];
	}
	//pthread_mutex_unlock(&mutex_auxiliar);
	return encontrado;
}
int paso_dos_clock()
{
	int encontrado = -1;
	int k = 0;
	//pthread_mutex_lock(&mutex_auxiliar);
	while (k < tamanio_memoria)
	{
		//pthread_mutex_lock(&mutex_auxiliar);
		t_auxiliar *aux = buscar_auxiliar_2(aguja_contador);
		//pthread_mutex_unlock(&mutex_auxiliar);
		if (!aux->utilizado && aux->modificado && aux->presente)
		{
			pthread_mutex_lock(&mutex_memoria);
			t_plato plato = memoria_principal[aux->frame_memoria];
			pthread_mutex_unlock(&mutex_memoria);
			printf("Victima %s\n", plato.nombre);
			log_info(logger, "Victima seleccionada para reemplazo: %s\n", plato.nombre);
			pthread_mutex_lock(&mutex_swap);
			memoria_swap[aux->frame_swap] = plato;
			pthread_mutex_unlock(&mutex_swap);
			encontrado = aux->frame_memoria;
			//pthread_mutex_lock(&mutex_auxiliar);
			modificar_auxiliar(aux, aux->frame_memoria, aux->frame_swap, false, false, aux->comparado, false);
			//pthread_mutex_unlock(&mutex_auxiliar);
			if (tamanio_memoria == (aguja_contador + 1))
			{
				aguja_contador = -1;
			}
			aguja_contador++;
			//aguja = list_get(lista_aux, aguja_contador);
			break;
		}
		else
		{
			//pthread_mutex_lock(&mutex_auxiliar);
			modificar_auxiliar(aux, aux->frame_memoria, aux->frame_swap, aux->presente, aux->modificado, aux->comparado, false);
			//pthread_mutex_unlock(&mutex_auxiliar);
		}
		if (tamanio_memoria == (aguja_contador + 1))
		{
			aguja_contador = -1;
		}
		aguja_contador++;
		k++;
		//aguja = list_get(lista_aux, aguja_contador);
	}
	//pthread_mutex_unlock(&mutex_auxiliar);
	return encontrado;
}

//Funciones para liberar memoria

void liberar_memoria(t_plato *memoria)
{
	free(memoria);
}
void liberar_bitarray(int *bitarray)
{
	free(bitarray);
}
void liberar_pagina(void *pag)
{
	t_pagina *pagina = (t_pagina *)pag;
	free(pagina->auxiliar);
	free(pagina);
}
void liberar_restaurante(void *resto)
{
	t_restaurante *restaurante = (t_restaurante *)resto;
	free(restaurante->lista_pedidos);
	free(restaurante->nombre);
	free(restaurante);
}
void liberar_pedido(void *ped)
{
	t_pedido *pedido = (t_pedido *)ped;
	free(pedido->lista_platos);
	free(pedido->estado);
	free(pedido);
}
void liberar_estructura()
{
	int i = 0;
	t_restaurante *restaurante = list_get(restaurantes, i);
	while (i < list_size(restaurantes))
	{
		int k = 0;
		t_pedido *pedido = list_get(restaurante->lista_pedidos, k);
		while (k < list_size(restaurante->lista_pedidos))
		{
			int j = 0;
			while (j < list_size(pedido->lista_platos))
			{
				list_remove_and_destroy_element(pedido->lista_platos, j, liberar_pagina);
				j++;
			}
			list_remove_and_destroy_element(restaurante->lista_pedidos, k, liberar_pedido);
			k++;
			pedido = list_get(restaurante->lista_pedidos, k);
		}
		i++;
		restaurante = list_get(restaurantes, i);
	}
	list_clean_and_destroy_elements(restaurantes, liberar_restaurante);
	free(lista_aux);
}

void eliminar_paginas_lista(t_list *lista_platos)
{	
	int i = 0;
	pthread_mutex_lock(&mutex_lista_platos);
	while (i < list_size(lista_platos))
	{
		list_remove_and_destroy_element(lista_platos, i, liberar_pagina);
		i++;
	}
	pthread_mutex_unlock(&mutex_lista_platos);
}
void eliminar_paginas_memoria(t_list *lista_platos)
{
	pthread_mutex_lock(&mutex_lista_platos);
	for (int i = 0; i < list_size(lista_platos); i++)
	{
		t_pagina *pagina = list_get(lista_platos, i);
		if (bitarray_memoria[pagina->auxiliar->frame_memoria] == 1)
		{
			log_info(logger, "Eliminando posicion %d en Memoria Principal\n", pagina->auxiliar->frame_memoria);
			desocupar_frame_memoria(pagina->auxiliar->frame_memoria);
		}
		desocupar_frame_swap(pagina->auxiliar->frame_swap);
	}
	pthread_mutex_unlock(&mutex_lista_platos);
}

//Otros

void agregar_plato(t_plato plato, t_plato *plato_array, int tamanio, int pos_array)
{
	if (pos_array < tamanio)
		plato_array[pos_array] = plato;
}

void agregar_plato_memoria(t_plato plato, int pos_array)
{
	pthread_mutex_lock(&mutex_memoria);
	agregar_plato(plato, memoria_principal, tamanio_memoria, pos_array);
	pthread_mutex_unlock(&mutex_memoria);
}

void agregar_plato_swap(t_plato plato, int pos_array)
{
	pthread_mutex_lock(&mutex_swap);
	agregar_plato(plato, memoria_swap, tamanio_swap, pos_array);
	pthread_mutex_unlock(&mutex_swap);
}

void recorrer_array(t_plato array[], int tamanio)
{
	for (int i = 0; i < tamanio; i++)
	{
		printf("%s %d\n", array[i].nombre, array[i].cantidad);
	}
	return;
}

void ocupar_frame(int bitarray[], int pos)
{
	bitarray[pos] = 1;
}

void ocupar_frame_swap(int pos)
{
	pthread_mutex_lock(&mutex_swap);
	ocupar_frame(bitarray_swap, pos);
	pthread_mutex_unlock(&mutex_swap);
}
void ocupar_frame_memoria(int pos)
{
	pthread_mutex_lock(&mutex_memoria);
	ocupar_frame(bitarray_memoria, pos);
	pthread_mutex_unlock(&mutex_memoria);
}

void desocupar_frame(int bitarray[], int pos)
{
	bitarray[pos] = 0;
}

void desocupar_frame_swap(int pos)
{
	pthread_mutex_lock(&mutex_swap);
	desocupar_frame(bitarray_swap, pos);
	pthread_mutex_unlock(&mutex_swap);
}
void desocupar_frame_memoria(int pos)
{
	pthread_mutex_lock(&mutex_memoria);
	desocupar_frame(bitarray_memoria, pos);
	pthread_mutex_unlock(&mutex_memoria);
}

void reiniciar_comparado(t_list *paginas)
{
	for (int k = 0; k < list_size(paginas); k++)
	{
		t_pagina *pagina = list_get(paginas, k);
		if (pagina->auxiliar->comparado)
		{
			pagina->auxiliar->comparado = false;
		}
	}
}

void recorrer_memorias()
{
	printf("-----------------------------\n");
	printf("Memoria Principal\n");
	for (int i = 0; i < tamanio_memoria; i++)
	{
		if (bitarray_memoria[i] == 1)
		{
			t_auxiliar* aux = buscar_auxiliar_2(i);
			printf("%s U: %d M: %d\n", memoria_principal[aux->frame_memoria].nombre,aux->utilizado,aux->modificado);
		}
	}
	printf("-----------------------------\n");
	/*
	printf("Memoria SWAP\n");
	for (int i = 0; i < tamanio_swap; i++)
	{
		if (bitarray_swap[i] == 1)
		{
			printf("%s\n", memoria_swap[i].nombre);
		}
	}
	printf("-----------------------------\n");
	*/
}
void recorrer_lista()
{
	printf("-----------------------------\n");
	for (int i = 0; i < list_size(lista_aux); i++)
	{
		char* p = ' ';
		t_auxiliar *auxiliar = list_get(lista_aux, i);
		if(auxiliar->presente) {
			if (aguja_contador == auxiliar->frame_memoria) {
				p = 'p';
			}
			printf("Plato: %s U: %d M: %d %c\n", memoria_swap[auxiliar->frame_swap].nombre, auxiliar->utilizado, auxiliar->modificado,p);
		}
		//printf("Timestamp: %lld\n", auxiliar->timestamp);
	}
	printf("-----------------------------\n");
}