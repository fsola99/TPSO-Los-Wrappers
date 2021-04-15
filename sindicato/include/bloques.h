#ifndef BLOQUES_H_
#define BLOQUES_H_

#include "sindicato.h"

int buscar_bloque_desocupado();
//int verificar_bloque_completo(int nro_bloque); 
char *obtener_ruta_bloque(int pos_bloque); 
int escribir_bloque(char * texto);
int obtener_initial_block(char * archivo);
char *obtener_datos_bloques(int bloque_inicial);
char *obtener_bloques_usados(int bloque_inicial);  
void eliminar_bloques(char* bloques); 

#endif /* BLOQUES_H_ */