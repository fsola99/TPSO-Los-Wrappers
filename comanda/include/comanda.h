#ifndef COMANDA_H_
#define COMANDA_H_

#include "sockets.h"
#include "serializer.h"
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#define CONFIG_PATH "./cfg/comanda.config"
#define CONEXIONES_PATH "../shared/cfg/conexiones.config"
char* IP;
t_config* conexiones_config;
pthread_mutex_t mutex_lista_restaurantes;
pthread_mutex_t mutex_lista_pedidos;
pthread_mutex_t mutex_lista_platos;
pthread_mutex_t mutex_memoria;
pthread_mutex_t mutex_swap;
pthread_mutex_t mutex_auxiliar;
pthread_mutex_t mutex_aguja;
//pthread_mutex_t mutex_buscar_restaurante = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_buscar_pedido = PTHREAD_MUTEX_INITIALIZER;


typedef struct {
	char* nombre_restaurante;
	uint32_t id_pedido;
} t_argumento;

typedef struct {
	char* nombre_restaurante;
	uint32_t id_pedido;
	char* nombre_plato;
	uint32_t cantidad;
} t_argumento_2;

t_log *logger;
t_list *restaurantes;
t_list *lista_aux;
int modulo;
char* algoritmo_reemplazo;
typedef struct
{
    char nombre[24];
    uint32_t cantidad;
    uint32_t cantidad_lista;
} t_plato;

int tamanio_memoria;
int tamanio_swap;
t_plato* memoria_principal;
t_plato* memoria_swap;
int* bitarray_memoria;
int* bitarray_swap;

typedef struct
{
    char* nombre;
    t_list *lista_pedidos;
} t_restaurante;

typedef struct
{
    uint32_t id_pedido;
    char* estado;
    t_list* lista_platos;
} t_pedido;

typedef struct 
{
    uint64_t timestamp;
    uint32_t frame_swap;
    uint32_t frame_memoria;
    bool presente;
    bool modificado;
    bool comparado;
    bool utilizado;
} t_auxiliar;

//t_plato* aguja;
int aguja_contador;

typedef struct
{   
    uint32_t indice;
    uint32_t frame_memoria;
    t_auxiliar* auxiliar;
} t_pagina;


//Funciones principales

void atender_cliente(void* cliente);

char *f_guardar_pedido(char *nombre_restaurante, uint32_t id_pedido);
char* f_guardar_plato(char *nombre_restaurante, uint32_t id_pedido, char *nombre_plato, uint32_t cantidad);
char *f_obtener_pedido(char *nombre_restaurante, uint32_t id_pedido);
char* f_confirmar_pedido(char *nombre_restaurante, uint32_t id_pedido);
char *f_plato_listo(char *nombre_restaurante, char *nombre_plato, uint32_t id_pedido);
char *f_finalizar_pedido(char *nombre_restaurante, uint32_t id_pedido);

//Funciones auxiliares

t_restaurante* buscar_restaurante(char *nombre_restaurante);
t_pedido *buscar_pedido(t_list *pedidos, uint32_t id_pedido);
int buscar_auxiliar(int pos_swap);
t_auxiliar* buscar_auxiliar_2(int pos_memoria);
int buscar_posicion_pedido(t_list*pedidos, t_pedido* pedido);
int buscar_plato_vacio(int bitarray[], int tamanio);
int buscar_plato_vacio_memoria();
int buscar_plato_vacio_swap();

int verificar_plato_en_pagina(t_pedido *pedido, char *nombre_plato);
int agregar_pagina(char *nombre_plato, t_pedido *pedido, int cantidad);
void modificar_cantidad(int plato_en_pagina, int cantidad, t_pedido *pedido);
void modificar_cantidad_lista(int plato_en_pagina, int cantidad_lista, t_pedido *pedido);
bool pedido_terminado(char *nombre_restaurante, t_pedido *pedido);
char *obtener_datos_pedido(t_restaurante *restaurante, uint32_t id_pedido);

int mover_plato_a_swap();
uint64_t timestamp();
bool comparador(void* auxiliar1, void* auxiliar2);
void modificar_auxiliar(t_auxiliar* auxiliar, int pos_memoria, int pos_swap, bool presente, bool modificado, bool comparado, bool utilizado);
int paso_dos_clock();
int paso_uno_clock();

void liberar_memoria(t_plato* memoria);
void liberar_bitarray(int* bitarray);
void liberar_pagina(void* pagina);
void liberar_restaurante(void* restaurante);
void liberar_pedido(void* pedido);
void liberar_estructura();
void eliminar_paginas_lista(t_list *lista_platos);
void eliminar_paginas_memoria(t_list* lista_platos);
void agregar_plato(t_plato plato, t_plato *plato_array, int tamanio, int pos_array);
void agregar_plato_memoria(t_plato plato,int pos_array);
void agregar_plato_swap(t_plato plato, int pos_array);

void recorrer_lista();
void recorrer_array(t_plato array[], int tamanio);
void ocupar_frame(int bitarray[],int pos);
void ocupar_frame_swap(int pos);
void ocupar_frame_memoria(int pos);
void desocupar_frame(int bitarray[], int pos);
void desocupar_frame_swap(int pos);
void desocupar_frame_memoria(int pos);
void reiniciar_comparado(t_list* paginas);
void recorrer_memorias();
void pruebas();
#endif /* COMANDA_H_ */