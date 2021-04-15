#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include "sockets.h"
#include "serializer.h"
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

t_config* restaurante_config;
t_log* logger;

typedef enum{
    NEW, READY, EXEC, HORNO, REPOSO, FIN
} e_estado;

typedef struct {
    char* afinidad;
    int index;
    t_list* cola_ready;
    pthread_mutex_t mx_ready;
    pthread_t hilo_afinidad;
    sem_t sem_pcb_ready;
    sem_t cocinero_libre;
    int ciclo_actual;
    pthread_mutex_t mx_ciclo;
} q_afinidad;

typedef struct {
    char* nombre_plato;
    char** pasos;
    char** cantidad_pasos;
    int total_pasos;
} t_receta;

typedef struct {
    int id_pedido;
    int id_plato;
    t_receta* datos;
    e_estado estado;
    int prioridad; // 0 NO USO TODO EL Q,  1 EXEC-> READY, 1 BLOQ->READY,, 3 NEW->READY (default)
    sem_t sem_listo;
    int rafaga;
    int pc;
    int index_afinidad;
    int espera_horno;
    int cpu_llegada;
} t_pcb;

typedef struct {
    int id;
    int cantidad_platos;
    sem_t sem_plato_listo;
    pthread_t hilo_pedido;
    char* precio_total;
} t_pedido;

typedef struct {
    char* nombre;
    int cantidad_cocineros;   // CANTIDAD_COCINEROS=5
    int pos_x;
    int pos_y;
    char** afinidades;   // AFINIDAD_COCINEROS=[Milanesas]
    char** platos;       // PLATOS=[Milanesas,Empanadas,Ensalada]
    char** precios;      // PRECIO_PLATOS=[200,50,150]
    int cantidad_hornos;      // CANTIDAD_HORNOS=2
    int cantidad_pedidos;
} t_rest;

//FIN DEL PROCESO
void free_t_rest(t_rest*);
void free_t_pedido(t_pedido*);
void free_t_receta(t_receta*);
void free_t_pcb(void*);

#endif /* ESTRUCTURAS_H_ */