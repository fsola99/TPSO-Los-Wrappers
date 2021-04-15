#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "estructuras.h"
#include "restaurante.h"

pthread_t planificator;
sem_t sem_planificador;
pthread_t hilo_exec;
pthread_t hilo_horno;
pthread_mutex_t mx_horno;
pthread_mutex_t mx_fin;

t_list *cola_new;
t_list *cola_fin;
t_list *cola_horno;
sem_t sem_horno;
sem_t horno_disponible;
q_afinidad** array_ready;

int procesadores; // = cantCocineros
int cantidad_afinidades;
char* algoritmo;
int quantum;
int retardo_cpu;

void inicializar_colas(char**);
void inicializar_planificador(void);
int asignar_afinidad(char*);
int encolar_ready_0(t_pcb*);
void encolar_ready(t_pcb *, bool);
void replanificar(int);
void encolar_horno(t_pcb*);
void planificador(q_afinidad*);
e_estado paso_a_seguir(char*);
void cocinar_fifo(t_pcb*);
void cocinar_rr(t_pcb*);
void cocinar(t_pcb*, int);
void hornear_plato();
void reposar_plato(t_pcb*);


void destruir_colas_planificador(void);
void destruir_planificador(void);
void destruir_array_afinidad(q_afinidad**);

//-----------------AUXILIARES-----------------------------------------
bool orden_prioridad(void*, void*);
int string_array_length(char **array);
bool string_in_array(char*, char**);
int ciclo_mas_alto();

#endif /* PLANIFICADOR_H_ */