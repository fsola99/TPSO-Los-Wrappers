#include "planificador.h"

void inicializar_colas(char **afinidades)
{
	cola_fin = list_create();
	cola_horno = list_create();
	int total_afinidades = cantidad_afinidades + 1;
	array_ready = malloc(sizeof(q_afinidad) * total_afinidades); // 0 A1 	1 A2 	 2 A3 	3 NULL
	int i = 0;
	for (; afinidades[i] != NULL; i++)
	{
		array_ready[i] = malloc(sizeof(q_afinidad));
		array_ready[i]->index = i;
		array_ready[i]->afinidad = afinidades[i];
		sem_init(&array_ready[i]->cocinero_libre, 1, 1);
	}
	array_ready[i] = malloc(sizeof(q_afinidad));
	array_ready[i]->index = i;
	array_ready[i]->afinidad = string_duplicate("SinAfinidad");
	int cocineros_sin_afinidad = procesadores - cantidad_afinidades;
	sem_init(&array_ready[i]->cocinero_libre, 1, cocineros_sin_afinidad);
	for (int i = 0; i < total_afinidades; i++)
	{
		array_ready[i]->ciclo_actual = 0;
		array_ready[i]->cola_ready = list_create();
		pthread_mutex_init(&array_ready[i]->mx_ready, NULL);
		pthread_mutex_init(&array_ready[i]->mx_ciclo, NULL);
		sem_init(&array_ready[i]->sem_pcb_ready, 0, 0);
		pthread_create(&array_ready[i]->hilo_afinidad, NULL, (void *)planificador, array_ready[i]);
		pthread_detach(array_ready[i]->hilo_afinidad);
	}
}

void inicializar_planificador()
{
	pedidos = list_create();
	sem_init(&sem_planificador, 0, 1);
	sem_init(&horno_disponible, 0, resto->cantidad_hornos);
	sem_init(&sem_horno, 0, 0);
	pthread_mutex_init(&mx_fin, NULL);
	pthread_mutex_init(&mx_pedidos, NULL);
	pthread_create(&hilo_horno, NULL, (void *)hornear_plato, NULL);
}

void destruir_planificador()
{
	sem_destroy(&sem_planificador);
	sem_destroy(&horno_disponible);
	sem_destroy(&sem_horno);
	pthread_mutex_destroy(&mx_fin);
	pthread_mutex_destroy(&mx_pedidos);
	destruir_array_afinidad(array_ready);
}

int asignar_afinidad(char *nombre_plato)
{
	int i = 0;
	if (cantidad_afinidades > 0)
	{
		for (; i < cantidad_afinidades; i++)
		{
			char *nombre_afinidad = array_ready[i]->afinidad;
			if (string_equals_ignore_case(nombre_plato, nombre_afinidad))
			{
				return i;
			}
		}
	}
	return i;
}

void encolar_ready(t_pcb *pcb, bool replanificar)
{
	pcb->estado = READY;
	int i = pcb->index_afinidad;
	pthread_mutex_lock(&array_ready[i]->mx_ciclo);
	pcb->cpu_llegada = ciclo_mas_alto();
	pthread_mutex_unlock(&array_ready[i]->mx_ciclo);

	pthread_mutex_lock(&array_ready[i]->mx_ready);
	list_add(array_ready[i]->cola_ready, pcb);
	if (replanificar)
	{
		list_sort(array_ready[i]->cola_ready, orden_prioridad);
	}
	pthread_mutex_unlock(&array_ready[i]->mx_ready);

	log_info(logger, "[%d][%s-%d] en READY [%s]- instante [%d]", pcb->id_pedido, pcb->datos->nombre_plato, pcb->id_plato, array_ready[i]->afinidad, pcb->cpu_llegada);
	sem_post(&array_ready[i]->sem_pcb_ready);
}

int ciclo_mas_alto()
{
	int mayor = 0; //[] [Choripan] [Ch, Asado]
	if (cantidad_afinidades > 0)
	{
		for (int i = 0; i < cantidad_afinidades; i++)
		{
			if (array_ready[i]->ciclo_actual > array_ready[i + 1]->ciclo_actual)
				mayor = array_ready[i]->ciclo_actual;
			else
				mayor = array_ready[i + 1]->ciclo_actual;
		}
		return mayor;
	}
	return array_ready[0]->ciclo_actual;
}

int encolar_ready_0(t_pcb *pcb)
{
	pcb->estado = READY;
	int nro_cola = -1;
	int i = 0;
	for (; i < cantidad_afinidades; i++)
	{
		char *nombre_plato = pcb->datos->nombre_plato;
		char *nombre_afinidad = array_ready[i]->afinidad;
		if (string_equals_ignore_case(nombre_plato, nombre_afinidad))
		{
			pcb->index_afinidad = i;
			pthread_mutex_lock(&array_ready[i]->mx_ready);
			list_add(array_ready[i]->cola_ready, pcb);
			pthread_mutex_unlock(&array_ready[i]->mx_ready);
			log_info(logger, "PLATO [%s] encolado en READY de afinidad [%s]", pcb->datos->nombre_plato, array_ready[i]->afinidad);
			sem_post(&array_ready[i]->sem_pcb_ready);
			nro_cola = i;
			return nro_cola;
		}
	}
	nro_cola = i;
	pcb->index_afinidad = nro_cola;
	pthread_mutex_lock(&array_ready[nro_cola]->mx_ready);
	list_add(array_ready[nro_cola]->cola_ready, pcb);
	pthread_mutex_unlock(&array_ready[nro_cola]->mx_ready);
	log_info(logger, "PLATO [%s] encolado en READY de afinidad [%s]", pcb->datos->nombre_plato, array_ready[nro_cola]->afinidad);
	sem_post(&array_ready[nro_cola]->sem_pcb_ready);
	return nro_cola;
}

void planificador(q_afinidad *afinidad)
{
	while (1)
	{
		sem_wait(&afinidad->sem_pcb_ready);
		pthread_mutex_lock(&afinidad->mx_ready);
		t_pcb *plato = list_get(afinidad->cola_ready, 0);
		list_remove(afinidad->cola_ready, 0);
		pthread_mutex_unlock(&afinidad->mx_ready);
		cocinar(plato, EXEC);
	}
}

void cocinar(t_pcb *plato, int paso_receta)
{
	int i = plato->pc;
	int tiempo = atoi(plato->datos->cantidad_pasos[i]);
	plato->rafaga = tiempo;
	switch (paso_receta)
	{
	case REPOSO:;
		plato->estado = REPOSO;
		reposar_plato(plato);
		break;
	case HORNO:;
		encolar_horno(plato);
		break;
	case EXEC:
		plato->estado = EXEC;
		if (string_equals_ignore_case(algoritmo, "fifo"))
		{
			cocinar_fifo(plato);
		}
		else
		{
			cocinar_rr(plato);
		}
		break;
	}
}

e_estado paso_a_seguir(char *paso)
{
	if (string_equals_ignore_case(paso, "REPOSAR"))
	{
		return REPOSO;
	}
	else if (string_equals_ignore_case(paso, "HORNEAR"))
	{
		return HORNO;
	}
	else
	{
		return EXEC;
	}
}

void cocinar_fifo(t_pcb *plato)
{
	int id_afinidad = plato->index_afinidad;
	sem_wait(&array_ready[id_afinidad]->cocinero_libre);
	t_receta *receta = plato->datos;
	int pc = plato->pc;
	log_info(logger, "[%d][%s-%d] en EXEC(FIFO) se va a [%s] durante [%d]s", plato->id_pedido, receta->nombre_plato, plato->id_plato, receta->pasos[pc], plato->rafaga);
	for (int i = 0; i < plato->rafaga; i++)
	{
		pthread_mutex_lock(&array_ready[id_afinidad]->mx_ciclo);
		array_ready[id_afinidad]->ciclo_actual++;
		pthread_mutex_unlock(&array_ready[id_afinidad]->mx_ciclo);
		sleep(retardo_cpu); //data race?
	}
	plato->pc++;
	if (plato->pc == receta->total_pasos)
	{
		r_plato_listo(plato);
	}
	else
	{
		log_info(logger, "[%d][%s-%d] terminó de [%s]", plato->id_pedido, receta->nombre_plato, plato->id_plato, receta->pasos[pc]);
		int paso_siguiente = paso_a_seguir(plato->datos->pasos[plato->pc]);
		if (paso_siguiente == HORNO || paso_siguiente == REPOSO)
		{
			cocinar(plato, paso_siguiente);
		}
		else
		{
			encolar_ready(plato, false);
		}
	}
	sem_post(&array_ready[id_afinidad]->cocinero_libre);
}

void cocinar_rr(t_pcb *plato)
{
	int id_afinidad = plato->index_afinidad;
	sem_wait(&array_ready[id_afinidad]->cocinero_libre);
	t_receta *receta = plato->datos;
	int pc = plato->pc; //[Servir, Horno, Servir] [1,6,2]
	plato->prioridad = 3;
	if (plato->rafaga <= quantum)
	{
		log_info(logger, "[%d][%s-%d] en EXEC(RR) se va a [%s] durante [%d]s (rafaga)", plato->id_pedido, receta->nombre_plato, plato->id_plato, receta->pasos[pc], plato->rafaga);
		for (int i = 0; i < plato->rafaga; i++)
		{
			pthread_mutex_lock(&array_ready[id_afinidad]->mx_ciclo);
			array_ready[id_afinidad]->ciclo_actual++;
			pthread_mutex_unlock(&array_ready[id_afinidad]->mx_ciclo);
			sleep(retardo_cpu);
		}
		plato->pc++;
		if (plato->pc == receta->total_pasos)
		{
			r_plato_listo(plato);
		}
		else
		{
			log_info(logger, "[%d][%s-%d] terminó de [%s]", plato->id_pedido, receta->nombre_plato, plato->id_plato, receta->pasos[pc]);
			int paso_siguiente = paso_a_seguir(receta->pasos[plato->pc]);
			if (paso_siguiente == HORNO || paso_siguiente == REPOSO)
			{
				cocinar(plato, paso_siguiente);
			}
			else
			{
				if (plato->rafaga == quantum)
				{
					plato->prioridad = 1;
					encolar_ready(plato, true);
				}
				else
				{
					encolar_ready(plato, false);
				}
			}
		}
	}
	else
	{
		log_info(logger, "[%d][%s-%d] en EXEC(RR), se va a [%s] durante [%d]s (quantum)", plato->id_pedido, receta->nombre_plato, plato->id_plato, receta->pasos[pc], quantum);
		for (int i = 0; i < quantum; i++)
		{
			pthread_mutex_lock(&array_ready[id_afinidad]->mx_ciclo);
			array_ready[id_afinidad]->ciclo_actual++;
			pthread_mutex_unlock(&array_ready[id_afinidad]->mx_ciclo);
			sleep(retardo_cpu);
		}
		log_info(logger, "[%d][%s-%d] no terminó de [%s] completamente, vuelve a READY", plato->id_pedido, receta->nombre_plato, plato->id_plato, receta->pasos[pc]);
		plato->prioridad = 0;
		int rafaga_anterior = atoi(receta->cantidad_pasos[pc]);
		int rafaga_restante = rafaga_anterior - quantum;
		char *nueva_rafaga = string_from_format("%d", rafaga_restante);
		receta->cantidad_pasos[pc] = nueva_rafaga;
		encolar_ready(plato, true);
	}
	sem_post(&array_ready[id_afinidad]->cocinero_libre);
}

void reposar_plato(t_pcb *plato)
{
	log_info(logger, "[%s] pasa a REPOSO por [%d]s", plato->datos->nombre_plato, plato->rafaga);
	for (int i = 0; i < plato->rafaga; i++)
	{
		sleep(retardo_cpu);
	}
	log_info(logger, "[%s] sale de REPOSO, pasa a READY");
	plato->pc++;
	encolar_ready(plato, false);
}

void encolar_horno(t_pcb *plato)
{
	plato->estado = HORNO;
	plato->espera_horno = 0;
	pthread_mutex_lock(&mx_horno);
	list_add(cola_horno, plato);
	pthread_mutex_unlock(&mx_horno);
	sem_post(&sem_horno);
}

void consumo_quantum(void *_plato)
{
	t_pcb *plato = (t_pcb *)_plato;
	plato->espera_horno++;
}

void hornear_plato()
{
	while (true)
	{
		sem_wait(&sem_horno);
		sem_wait(&horno_disponible);

		pthread_mutex_lock(&mx_horno);
		t_pcb *plato = list_get(cola_horno, 0);
		list_remove(cola_horno, 0);
		pthread_mutex_unlock(&mx_horno);

		log_info(logger, "[%d][%s-%d] en HORNO por [%d]s - esperó [%d]s", plato->id_pedido, plato->datos->nombre_plato, plato->id_plato, plato->rafaga, plato->espera_horno);
		for (int i = 0; i < plato->rafaga; i++)
		{
			pthread_mutex_lock(&mx_horno);
			list_iterate(cola_horno, (void *)consumo_quantum);
			pthread_mutex_unlock(&mx_horno);
			sleep(retardo_cpu);
		}
		log_info(logger, "[%d][%s-%d] finaliza HORNO, pasa a READY", plato->id_pedido, plato->datos->nombre_plato, plato->id_plato);
		plato->pc++;
		plato->prioridad = 2;
		encolar_ready(plato, string_equals_ignore_case(algoritmo, "RR"));

		sem_post(&horno_disponible);
	}
}

void replanificar(int i)
{
	pthread_mutex_lock(&array_ready[i]->mx_ready);
	list_sort(array_ready[i]->cola_ready, orden_prioridad); //replanificaor
	pthread_mutex_unlock(&array_ready[i]->mx_ready);
}

bool orden_prioridad(void *elem1, void *elem2)
{
	t_pcb *plato1 = (t_pcb *)elem1;
	t_pcb *plato2 = (t_pcb *)elem2;
	if (plato1->cpu_llegada == plato2->cpu_llegada)
		return plato1->prioridad < plato2->prioridad ? true : false;
	return false;
}

void destruir_colas_planificador()
{
	if (!list_is_empty(cola_fin))
	{
		list_destroy_and_destroy_elements(cola_fin, free_t_pcb);
	}
}

void destruir_array_afinidad(q_afinidad **array)
{
	int afinidades = cantidad_afinidades + 1;
	for (int i = 0; i < afinidades; i++)
	{
		pthread_mutex_lock(&array[i]->mx_ready);
		if (!list_is_empty(array[i]->cola_ready))
		{
			list_destroy_and_destroy_elements(array[i]->cola_ready, free_t_pcb);
		}
		else
		{
			list_destroy(array[i]->cola_ready);
		}
		pthread_mutex_unlock(&array[i]->mx_ready);
		sem_destroy(&array[i]->sem_pcb_ready);
		sem_destroy(&array[i]->cocinero_libre);
		pthread_mutex_destroy(&array[i]->mx_ready);
		free(array[i]->afinidad);
		free(array[i]);
	}
	free(array);
}

void free_t_receta(t_receta *rec)
{
	liberar_recibido(rec->pasos);
	liberar_recibido(rec->cantidad_pasos);
	free(rec->nombre_plato);
	free(rec);
}

void free_t_pcb(void *pcb)
{
	t_pcb *pcb2 = (t_pcb *)pcb;
	while (pcb2 != NULL)
	{
		free_t_receta(pcb2->datos);
		sem_destroy(&pcb2->sem_listo);
		free(pcb2);
	}
}

//-----------------AUXILIARES-----------------------------------------
int string_array_length(char **array)
{
	int i = 0;
	while (array[i] != NULL)
	{
		i++;
	}
	return i;
}

bool string_in_array(char *string, char **array)
{
	for (int i = 0; array[i] != NULL; i++)
	{
		if (string_equals_ignore_case(array[i], string))
		{
			return true;
		}
	}
	return false;
}