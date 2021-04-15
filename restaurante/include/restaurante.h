#ifndef RESTAURANTE_H_
#define RESTAURANTE_H_

#include "estructuras.h"
#include "planificador.h"
#include "signal.h"

char* IP;
t_config* conexiones_config;

pthread_t hilo_listo;
pthread_mutex_t mx_resto;

t_rest* resto;
t_list* pedidos;
pthread_mutex_t mx_pedidos;

t_rest* obtener_metadata(char*);
t_receta* r_obtener_receta(char*);
void r_terminar_pedido(t_pedido*);
t_pedido* agregar_pedido(char*, char**, char**);
t_pedido* encontrar_pedido(int);

void r_confirmar_pedido(char*);
void r_plato_listo(t_pcb*);

char* mensaje_sindicato(char*, int);

//ATENDER CONEXIONES
int modulo;
int socket_conexion;
void atender_cliente();
void cerrar_restaurante();

#endif /* RESTAURANTE_H_ */