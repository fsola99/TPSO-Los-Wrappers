#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //ISDIGIT
#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <string.h>
#include "protocolo.h"
//#include "../../cliente/include/consola.h"

typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

typedef struct{
    int socket;
    char *ingresado;
    int op;
    int modulo;
}parametros_t;

typedef struct {
    uint8_t codigo_operacion;
    t_buffer* buffer;
} t_paquete;

typedef struct{
    char* nombre_palabra;
    uint32_t length_palabra;
}t_palabra;

void* serializar_y_enviar(void* arg);
bool esNumero(char* s);
t_paquete* recibir_y_desempaquetar(int socket);
char* deserializar_palabra(t_buffer* buffer);
char** deserializar_paquete(t_paquete* paquete);
void liberar_paquete(t_paquete* paquete, void* stream);
void liberar_palabra(t_palabra palabra);
void liberar_recibido(char** recibido2);
void a_enviar(char *datos, int socket, int modulo, int cod_op);

#endif /* SERIALIZER_H */ 