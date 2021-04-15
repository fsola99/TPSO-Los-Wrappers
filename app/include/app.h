#ifndef APP_H_
#define APP_H_

#include "sockets.h"
#include "serializer.h"
#include "protocolo.h"
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <readline/readline.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <signal.h>

#define CONFIG_PATH "./cfg/app.config"
#define CONEXIONES_PATH "../shared/cfg/conexiones.config"
char *IP;
char *ip_comanda;
int puerto_comanda;

fd_set readset, tempset;
t_config *app_config;
t_config *conexiones_config;
t_log *logger;

t_list *lista_cliente;
t_list *lista_restaurante;
t_list *lista_bloqueados;
pthread_mutex_t mutex_new;
pthread_mutex_t mutex_ready;
pthread_mutex_t mutex_espera;
pthread_mutex_t mutex_exit;
pthread_mutex_t mutex_FIFO;
pthread_mutex_t mutex_SJF;
pthread_mutex_t mutex_HRRN;

typedef struct
{
    char *ID;
    int pos_x;
    int pos_y;
    int socket;
    int ID_pedido;
} restaurante_t;

typedef struct
{
    char *ID;
    int pos_x;
    int pos_y;
    int socket;
    restaurante_t *restaurante;
} datos_t;


typedef struct
{
    int tiempo_esperando;
    int rafaga_anterior;
    int estimacion_anterior;
    char Identificador;
    int pos_repartidor[2];
    int distancia;
    int tiempo_ejecucion;
    int tiempo_bloqueado;
} repartidores_t;

typedef struct
{
    char *restaurante;
    int pos_restaurante[2];
    int pos_cliente[2];
    repartidores_t *repartidor;
    int ID_pedido;
    int pedido_listo;
    int cliente;
} pedido_t;

void inicializacion_planificacion();
void enviar_ready(pedido_t *pedido);

bool verificar(t_list *lista, char *ID, int pos_x, int pos_y);
bool filtrar_restaurantes(void *element);
void liberar_recibido(char **recibido);
void cerrar_cliente(int cliente);
char *mensaje_comanda(char *mensaje, int codigo_operacion);
char *mensaje_restaurante(char *mensaje, int codigo_operacion, int socket_restaurante);
datos_t *obtener_cliente(int cliente);
void crear_PCB(datos_t *datos_cliente, char* ID_pedido);
pedido_t* verificar_lista_block(char* ID_pedido, char *nombre_restaurante,char *condicion);

void f_reconocimiento(char* ID, int pos_x, int pos_y, int cliente);
void f_consultar_restaurantes(int cliente);
void f_seleccionar_restaurante(char* restaurante, int cliente);
void f_consultar_platos(int cliente);
void f_crear_pedido(int cliente);
void f_aniadir_plato(char* ID, char* plato, int cliente);
void f_confirmar_pedido(char* ID, int cliente);
void f_consultar_pedido(char* restaurante, char* ID_pedido, int cliente);
void f_plato_listo(char* nombre_restaurante, char* id_pedido, char* plato, int cliente);

void cerrar();

#endif /* APP_H_ */


