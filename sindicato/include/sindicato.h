#ifndef SINDICATO_H_
#define SINDICATO_H_

#include "sockets.h"
#include "serializer.h"
#include <sys/stat.h>
#include <readline/readline.h>
#include <dirent.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <commons/txt.h>
#include <math.h>
#include "inicializacion.h"
#include "bloques.h"
#include "funciones_mesajes.h"
#include <pthread.h>

char* IP;
t_config* conexiones_config;
t_bitarray* bitmap;
t_log* logger;
char* punto_montaje;
int tamanio_bloque;
int cantidad_bloques;
int modulo;

pthread_mutex_t mutex;

void menu_sindicato();

void nuevo_archivo(char *nuevo_archivo);
void* consola_sindicato();
void atender_cliente(void* cliente_fd);

int parametros_check(char** datos_restaurante);
void crear_restaurante();
void crear_receta();

int verificar_archivo(char *ruta);
int verificar_restaurante(char* nombre_restaurante);
int verificar_pedido(char* ruta);

void eliminar_caracteres(char *x,int a, int b);

int obtener_size_archivo(char * archivo); 

char *obtener_datos_restaurante(char*restaurante);
char *obtener_datos_receta(char* receta);
char* obtener_datos_pedido(char* restaurante,int pedido_ID);

// para guardar plato
int obtener_precio_plato(char * nombre_restaurante, char * nombre_plato);
char * de_array_a_string(char ** array);
int prox_pos_vacia(char** array);
char * agregar_elemento (char * array, char * nuevo_dato);

#endif /* SINDICATO_H_ */