#ifndef PLANIFICACION_H
#define PLANIICACION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/string.h>
#include <string.h>
#include "protocolo.h"
#include <pthread.h>
#include "serializer.h"
#include "config.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "app.h"
#include <math.h>

t_queue *new;
t_queue *ready, *ready_aux;
t_queue *exit_plani;
t_list *repartidores_libres;
int tiempo_retardo;
int grado_multiprocesamiento;
int procesadores;
char *algoritmo;
bool liberar;
typedef enum
{
    FIFO,
    SJF,
    HRRN,
}enum_algoritmo;


repartidores_t *buscar_repartidor(pedido_t *pedido);
void *estado_new();
void obtener_repartidores();
int distancia(int restaurante[], int repartidor[]);
bool comparador(void * element1,void * element2);
void planificador();
void *ALGORITMO_FIFO();
void *ALGORITMO_SJF();
void *ALGORITMO_HRRN();
void *contador(void* ped);
bool ordenar_lista_SJF(void *element1, void *element2);
bool ordenar_lista_HRRN(void *element1, void *element2);
void *procesador(void *proceso);
void desplazar_repartidor(pedido_t *pedido,int posicion_destino[]);
void *f_block(void *proceso);



#endif /* PLANIFICACION_H */ 
