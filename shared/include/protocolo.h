
#ifndef PROTOCOLO_H
#define PROTOCOLO_H

typedef enum {
	consultar_restaurantes, 
    seleccionar_restaurante, 
	obtener_restaurante, 
    consultar_platos, 
	crear_pedido, 
    guardar_pedido, 
	aniadir_plato, 
    guardar_plato, 
	confirmar_pedido, 
    plato_listo,
    consultar_pedido, 
    obtener_pedido, 
	finalizar_pedido, 
    terminar_pedido,
    obtener_receta,
    cerrar_conexion, 
    atras,
    RECONOCIMIENTO,
    RETORNO,

} t_protocolo;

typedef enum{
    APP,
    COMANDA,
    RESTAURANTE,
    SINDICATO,
    CLIENTE,
}t_modulos;



#endif /* PROTOCOLO_H */