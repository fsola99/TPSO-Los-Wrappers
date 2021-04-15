#include "planificacion.h"
int procesadores = 0;

void inicializacion_planificacion()
{
    pthread_mutex_init(&mutex_new, NULL);
    pthread_mutex_init(&mutex_FIFO, NULL);
    pthread_mutex_init(&mutex_SJF, NULL);
    pthread_mutex_init(&mutex_HRRN, NULL);
    pthread_mutex_init(&mutex_ready, NULL);
    pthread_mutex_init(&mutex_espera, NULL);
    pthread_mutex_init(&mutex_exit, NULL);
    new = queue_create();
    ready = queue_create();
    exit_plani = queue_create();
    lista_bloqueados = list_create();
    repartidores_libres = list_create();
    tiempo_retardo = config_get_int_value(app_config, "RETARDO_CICLO_CPU");
    grado_multiprocesamiento = config_get_int_value(app_config, "GRADO_DE_MULTIPROCESAMIENTO");
    procesadores = 0;
    obtener_repartidores();
    sleep(1);
    planificador();

    pthread_t new;
    pthread_mutex_lock(&mutex_new);
    pthread_create(&new, NULL, estado_new, (void *)app_config);

    return;
}

void *estado_new()
{
    repartidores_t *repartidor = malloc(sizeof(repartidores_t));
    while (1)
    {
        pthread_mutex_lock(&mutex_new);
        if (!queue_is_empty(new) && (!list_is_empty(repartidores_libres)))
        {
            pedido_t *pedido = queue_pop(new);
            repartidor = buscar_repartidor(pedido);
            pedido->repartidor = repartidor;
            log_info(logger, "El repartidor %c fue asignado al pedido %d, del restaurante %s", pedido->repartidor->Identificador, pedido->ID_pedido, pedido->restaurante);
            enviar_ready(pedido);
        }
    }
}

void obtener_repartidores()
{
    char **repartidores = config_get_array_value(app_config, "REPARTIDORES");
    char **tiempo_ejecucion = config_get_array_value(app_config, "FRECUENCIA_DE_DESCANSO");
    char **tiempo_bloqueado = config_get_array_value(app_config, "TIEMPO_DE_DESCANSO");
    int estimacion_inicial = config_get_int_value(app_config, "ESTIMACION_INICIAL");
    char **pos_repartidores;
    char letra = '@';
    for (int i = 0; repartidores[i] != NULL; i++)
    {
        repartidores_t *repartidor = malloc(sizeof(repartidores_t));
        pos_repartidores = string_split(repartidores[i], "|");
        repartidor->pos_repartidor[0] = atoi(pos_repartidores[0]);
        repartidor->pos_repartidor[1] = atoi(pos_repartidores[1]);
        repartidor->tiempo_ejecucion = atoi(tiempo_ejecucion[i]);
        repartidor->tiempo_bloqueado = atoi(tiempo_bloqueado[i]);
        repartidor->rafaga_anterior = 0;
        repartidor->tiempo_esperando = 0;
        repartidor->estimacion_anterior = estimacion_inicial;
        letra++;
        repartidor->Identificador = letra;
        log_info(logger, "El repartidor %c se encuentra en la posicion: [%d,%d]", repartidor->Identificador, repartidor->pos_repartidor[0], repartidor->pos_repartidor[1]);
        list_add(repartidores_libres, repartidor);
    }
    liberar_recibido(tiempo_bloqueado);
    liberar_recibido(tiempo_ejecucion);
    liberar_recibido(repartidores);
    liberar_recibido(pos_repartidores);
    return;
}

repartidores_t *buscar_repartidor(pedido_t *pedido)
{
    repartidores_t *repartidor;

    for (int i = 0; i < list_size(repartidores_libres); i++)
    {
        repartidor = list_get(repartidores_libres, i);
        int valor = distancia(pedido->pos_restaurante, repartidor->pos_repartidor);
        repartidor->distancia = valor;
    }
    //ORDENAMOS LA LISTA DE REPARTIDORES

    list_sort(repartidores_libres, comparador);

    repartidor = list_remove(repartidores_libres, 0);
    return repartidor;
}

int distancia(int restaurante[], int repartidor[])
{
    return fabs(restaurante[0] - repartidor[0]) + fabs(restaurante[1] - repartidor[1]);
}

bool comparador(void *element1, void *element2)
{
    repartidores_t *repa1 = (repartidores_t *)element1;
    repartidores_t *repa2 = (repartidores_t *)element2;
    if (repa1->distancia < repa2->distancia)
        return true;
    else
        return false;
}

void planificador()
{
    pthread_t ALG_FIFO;
    pthread_t ALG_SJF;
    pthread_t ALG_HRRN;
    algoritmo = config_get_string_value(app_config, "ALGORITMO_DE_PLANIFICACION");

    t_dictionary *diccionario_planificacion = dictionary_create();
    dictionary_put(diccionario_planificacion, "FIFO", (void *)0);
    dictionary_put(diccionario_planificacion, "SJF", (void *)1);
    dictionary_put(diccionario_planificacion, "HRRN", (void *)2);
    int elegido = (int)dictionary_get(diccionario_planificacion, algoritmo);

    switch (elegido)
    {
    case FIFO:;
        pthread_mutex_lock(&mutex_FIFO);
        pthread_create(&ALG_FIFO, NULL, ALGORITMO_FIFO, NULL);
        break;
    case SJF:;
        pthread_mutex_lock(&mutex_SJF);
        pthread_create(&ALG_SJF, NULL, ALGORITMO_SJF, NULL);
        break;

    case HRRN:;
        pthread_mutex_lock(&mutex_HRRN);
        pthread_create(&ALG_HRRN, NULL, ALGORITMO_HRRN, NULL);
        break;

    default:
        error_show("NO SE RECONOCE EL ALGORITMO DE PLANIFICACION\n");
        exit(1);
        break;
    }

    return;
}

void *ALGORITMO_FIFO()
{
    pthread_t execute;
    while (1)
    {
        pthread_mutex_lock(&mutex_FIFO);
        if ((!queue_is_empty(ready)) && (procesadores < grado_multiprocesamiento))
        {
            pthread_mutex_lock(&mutex_ready);
            procesadores++;
            pedido_t *pedido = queue_pop(ready);
            pthread_mutex_unlock(&mutex_ready);
            pthread_create(&execute, NULL, procesador, (void *)pedido);
        }
    }
}

void *ALGORITMO_SJF()
{
    double alpha = config_get_double_value(app_config, "ALPHA");
    t_list *lista_aux = list_create();
    int rafaga_anterior;
    int estimacion_anterior;
    int cant;
    pedido_t *pedido;
    pthread_t execute;
    // estimacion = alpha * rafaga anterior + (1 - alplha) * estimacion anterior

    while (1)
    {
        pthread_mutex_lock(&mutex_SJF);
        cant = queue_size(ready);
        pthread_mutex_lock(&mutex_ready);
        for (int i = 0; i < queue_size(ready); i++)
        {
            pedido = queue_pop(ready);
            list_add(lista_aux, pedido);
        }
        pthread_mutex_unlock(&mutex_ready);
        for (int i = 0; i < list_size(lista_aux); i++)
        {
            pedido = list_get(lista_aux, i);
            rafaga_anterior = pedido->repartidor->rafaga_anterior;
            estimacion_anterior = pedido->repartidor->estimacion_anterior;
            pedido->repartidor->estimacion_anterior = alpha * rafaga_anterior + (1 - alpha) * estimacion_anterior;
            pedido->repartidor->rafaga_anterior = pedido->repartidor->tiempo_ejecucion;
        }
        list_sort(lista_aux, ordenar_lista_SJF);
        pthread_mutex_lock(&mutex_ready);
        for (int i = 0; i < list_size(lista_aux); i++)
        {
            pedido = list_remove(lista_aux, i);
            queue_push(ready, pedido);
        }

        pthread_mutex_unlock(&mutex_ready);
        if (queue_size(ready) > cant)
            continue;

        pedido = NULL;
        if ((!queue_is_empty(ready)) && (procesadores < grado_multiprocesamiento))
        {
            pthread_mutex_lock(&mutex_ready);
            procesadores++;
            pedido = queue_pop(ready);
            pthread_mutex_unlock(&mutex_ready);
            pthread_create(&execute, NULL, procesador, (void *)pedido);
        }
    }
    return 0;
}

bool ordenar_lista_SJF(void *element1, void *element2)
{
    pedido_t *pedido1 = (pedido_t *)element1;
    pedido_t *pedido2 = (pedido_t *)element2;
    if (pedido1->repartidor->estimacion_anterior < pedido2->repartidor->estimacion_anterior)
        return true;
    else
        return false;
}

void *ALGORITMO_HRRN()
{   
    liberar = false;
    t_list *lista_aux = list_create();
    pedido_t *pedido;
    pthread_t execute, clock;
    int tiempo_esperando, rafaga;
    while (1)
    {
        // ratio = tiempo de espera + rafaga / rafaga
        pthread_mutex_lock(&mutex_HRRN);
             
        pthread_mutex_lock(&mutex_ready);
        while (!queue_is_empty(ready))
        {
            pedido = queue_pop(ready);
            list_add(lista_aux, pedido);
        }
        pthread_mutex_unlock(&mutex_ready);

        for (int i = 0; i < list_size(lista_aux); i++)
        {
            pedido = list_get(lista_aux, i);
            tiempo_esperando = pedido->repartidor->tiempo_esperando;
            rafaga = pedido->repartidor->tiempo_ejecucion;
            pedido->repartidor->tiempo_esperando = ((tiempo_esperando + rafaga) / rafaga);
        }
        list_sort(lista_aux, ordenar_lista_HRRN);

        for (int i = 0; i < list_size(lista_aux); i++)
        {
            pedido = list_get(lista_aux, i);
            pedido->repartidor->tiempo_esperando = 0;
        }
        
        if (queue_size(ready) > list_size(lista_aux))
        {
            continue;
        }

        pthread_mutex_lock(&mutex_ready);
        while (!list_is_empty(lista_aux))
        {
            pedido = list_remove(lista_aux, 0);
            queue_push(ready, pedido);
        }
        pthread_mutex_unlock(&mutex_ready);

        pedido = NULL;

        if (!queue_is_empty(ready) && (procesadores < grado_multiprocesamiento))
        {
            pthread_mutex_lock(&mutex_ready);
            procesadores++;
            pedido = queue_pop(ready);
            pthread_mutex_unlock(&mutex_ready);
            pthread_create(&execute, NULL, procesador, (void *)pedido);
        }

        if (queue_size(ready) == 1)
        {
            pthread_mutex_lock(&mutex_ready);
            pedido = queue_pop(ready);
            pthread_mutex_unlock(&mutex_ready);
            pthread_create(&clock, NULL, contador, (void *)pedido);
        }
    }
    return 0;
}

void *contador(void *ped)
{
    pedido_t *pedido = (pedido_t *)ped;
    while (queue_is_empty(ready) && !liberar)
    {
        pedido->repartidor->tiempo_esperando++;
        log_info(logger, "Pedido %d esta esperando: %d", pedido->ID_pedido, pedido->repartidor->tiempo_esperando);
        sleep(1);
    }
    liberar = false;
    pthread_mutex_lock(&mutex_ready);
    queue_push(ready, pedido);
    pthread_mutex_unlock(&mutex_ready);
    pthread_mutex_unlock(&mutex_HRRN);
    return 0;
}

bool ordenar_lista_HRRN(void *element1, void *element2)
{
    pedido_t *pedido1 = (pedido_t *)element1;
    pedido_t *pedido2 = (pedido_t *)element2;
    if (pedido1->repartidor->tiempo_esperando > pedido2->repartidor->tiempo_esperando)
        return true;
    else
        return false;
}

void *procesador(void *proceso)
{
    pthread_t bloqueado;
    pedido_t *pedido = (pedido_t *)proceso;
    for (int i = 0; i < pedido->repartidor->tiempo_ejecucion; i++)
    {
        if (pedido->pedido_listo)
        {
            // DE RESTAURANTE A CLIENTE
            desplazar_repartidor(pedido, pedido->pos_cliente);
            log_info(logger, "Posicion del repartidor %c :[%d,%d]", pedido->repartidor->Identificador, pedido->repartidor->pos_repartidor[0], pedido->repartidor->pos_repartidor[1]);
            if ((pedido->repartidor->pos_repartidor[0] == pedido->pos_cliente[0]) && (pedido->repartidor->pos_repartidor[1] == pedido->pos_cliente[1]))
            {
                log_info(logger, "El repartidor %c llego al cliente", pedido->repartidor->Identificador);
                char *mensaje = string_from_format("%s %d", pedido->restaurante, pedido->ID_pedido);
                char *respuesta = mensaje_comanda(mensaje, finalizar_pedido);
                if (!strcmp(respuesta, "FAIL"))
                    printf("ERROR EN COMANDA CON FINALIZAR PEDIDO\n");
                a_enviar("El pedido llego correctamente al cliente", pedido->cliente, CLIENTE, RETORNO);
                log_info(logger, "Pedido del repartidor %c en exit", pedido->repartidor->Identificador);
                pthread_mutex_lock(&mutex_exit);
                queue_push(exit_plani, pedido);
                pthread_mutex_unlock(&mutex_exit);
                list_add(repartidores_libres, pedido->repartidor);
                pthread_mutex_unlock(&mutex_new);
                pthread_mutex_lock(&mutex_ready);
                procesadores--;
                pthread_mutex_unlock(&mutex_ready);
                if (!strcmp(algoritmo, "FIFO"))
                    pthread_mutex_unlock(&mutex_FIFO);
                if (!strcmp(algoritmo, "SJF"))
                    pthread_mutex_unlock(&mutex_SJF);
                if (!strcmp(algoritmo, "HRRN"))
                {
                    pthread_mutex_unlock(&mutex_HRRN);
                    liberar = true;
                }

                return 0;
            }
        }
        else
        {
            // DE REPARTIDOR A RESTAURANTE
            desplazar_repartidor(pedido, pedido->pos_restaurante);
            log_info(logger, "Posicion del repartidor %c :[%d,%d]", pedido->repartidor->Identificador, pedido->repartidor->pos_repartidor[0], pedido->repartidor->pos_repartidor[1]);
            if ((pedido->repartidor->pos_repartidor[0] == pedido->pos_restaurante[0]) && (pedido->repartidor->pos_repartidor[1] == pedido->pos_restaurante[1]))
            {
                log_info(logger, "El repartidor %c llego al restaurante", pedido->repartidor->Identificador);
                if (!strcmp(pedido->restaurante, "restaurante_default"))
                    pedido->pedido_listo = 1;
                else
                {
                    char *datos = string_new();
                    datos = string_from_format("%s %d", pedido->restaurante, pedido->ID_pedido);
                    char *respuesta = mensaje_comanda(datos, obtener_pedido);
                    char **estado = string_split(respuesta, ",");
                    if (strcmp(estado[0], "Terminado"))
                    {
                        pthread_mutex_lock(&mutex_espera);
                        log_info(logger, "El repartidor %c esperando el pedido", pedido->repartidor->Identificador);
                        list_add(lista_bloqueados, pedido);
                        pthread_mutex_unlock(&mutex_espera);
                        pthread_mutex_lock(&mutex_ready);
                        procesadores--;
                        pthread_mutex_unlock(&mutex_ready);
                        if (!strcmp(algoritmo, "FIFO"))
                            pthread_mutex_unlock(&mutex_FIFO);
                        if (!strcmp(algoritmo, "SJF"))
                            pthread_mutex_unlock(&mutex_SJF);
                        if (!strcmp(algoritmo, "HRRN"))
                            pthread_mutex_unlock(&mutex_HRRN);
                        return 0;
                    }
                    else
                    {
                        pedido->pedido_listo = 1;
                        enviar_ready(pedido);
                        pthread_mutex_lock(&mutex_ready);
                        procesadores--;
                        pthread_mutex_unlock(&mutex_ready);
                        return 0;
                    }
                }
            }
        }
        sleep(tiempo_retardo);
    }

    pthread_mutex_lock(&mutex_ready);
    procesadores--;
    pthread_mutex_unlock(&mutex_ready);
    pthread_create(&bloqueado, NULL, f_block, (void *)pedido);
    return 0;
}

void enviar_ready(pedido_t *pedido)
{
    pthread_mutex_lock(&mutex_ready);
    queue_push(ready, pedido);
    pthread_mutex_unlock(&mutex_ready);
    if (!strcmp(algoritmo, "FIFO"))
        pthread_mutex_unlock(&mutex_FIFO);
    if (!strcmp(algoritmo, "SJF"))
        pthread_mutex_unlock(&mutex_SJF);
    if (!strcmp(algoritmo, "HRRN"))
        pthread_mutex_unlock(&mutex_HRRN);
}

void desplazar_repartidor(pedido_t *pedido, int posicion_destino[])
{
    if (pedido->repartidor->pos_repartidor[0] < posicion_destino[0])
        pedido->repartidor->pos_repartidor[0]++;
    else if (pedido->repartidor->pos_repartidor[0] > posicion_destino[0])
        pedido->repartidor->pos_repartidor[0]--;
    else
    {
        if (pedido->repartidor->pos_repartidor[1] < posicion_destino[1])
            pedido->repartidor->pos_repartidor[1]++;
        else if (pedido->repartidor->pos_repartidor[1] > posicion_destino[1])
            pedido->repartidor->pos_repartidor[1]--;
    }
    return;
}

void *f_block(void *proceso)
{
    pedido_t *pedido = (pedido_t *)proceso;
    for (int i = 0; i < pedido->repartidor->tiempo_bloqueado; i++)
    {
        log_info(logger, "Repartidor %c en periodo de descanso", pedido->repartidor->Identificador);
        sleep(tiempo_retardo);
    }
    enviar_ready(pedido);

    return 0;
}
