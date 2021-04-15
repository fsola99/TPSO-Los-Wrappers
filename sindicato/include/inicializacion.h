#ifndef INICIALIZACION_H_
#define INICIALIZACION_H_

#include "sindicato.h"

void inicializacion(int blocks, int block_size);

void inicializar_carpetas();
void nueva_carpeta(char* nueva_carpeta);

void crear_metadata(int blocks, int block_size);
void nuevo_archivo_config(char *nuevo_archivo);
void agregar_datos_config(char* archivo, char* parametro, char* valor);

void leer_metadata(); 

void inicializar_bloques();
void crear_bloque(int numeroBloque);
void crear_archivo(char *nuevo_archivo);

void crear_archivo_Bitmap(); 
void cargar_Bitmap();

#endif /* INICIALIZACION_H_ */