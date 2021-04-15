#ifndef FUNCIONES_MENSAJES_H_
#define FUNCIONES_MENSAJES_H_

#include "sindicato.h"

char *f_confirmar_pedido(char *nombre_restaurante, int pedido_ID);
char *f_obtener_restaurante(char *restaurante);
char *f_consultar_platos(char *nombre_restaurante);
char *f_guardar_pedido(char *nombre_restaurante, int pedido_ID);
char *f_obtener_pedido(char *nombre_restaurante, int pedido_ID);
char *f_obtener_receta(char *nombre_receta);
char *f_terminar_pedido(char *nombre_restaurante, int pedido_ID);
char *f_guardar_plato(char *nombre_restaurante, int pedido_ID, char *nombre_plato, int cantidad_plato_agregar);
char *f_plato_listo(char *nombre_restaurante, int pedido_ID, char *nombre_plato);

#endif /* FUNCIONES_MENSAJES_H_ */
