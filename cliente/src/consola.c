#include "consola.h"

//funciones de consola
void consola(int socket, int modulo)
{

	char** ingresado;
	char* leido;
	int val;
	pthread_t tid;
	parametros_t parametros;
	parametros.socket = socket;
	int cant=0;

	int permitidas [4][16] = { {1,1,0,1,1,0,1,0,1,1,1,0,0,0,0,1},//app
							   {0,0,0,0,0,1,0,1,1,1,0,1,1,0,0,1},//comanda
							   {0,0,0,1,1,0,1,0,1,0,1,0,0,0,0,1},//restaurante
							   {0,0,1,1,0,1,0,1,1,1,0,1,0,1,1,1}//sindicato
	                         };
	int control [16] = {0,1,1,0,0,2,2,4,2,3,1,2,2,2,1,0};

	t_dictionary * diccionario_consola = dictionary_create();
	dictionary_put(diccionario_consola, "consultar_restaurantes", (void*)0);
	dictionary_put(diccionario_consola, "seleccionar_restaurante", (void*)1);
	dictionary_put(diccionario_consola, "obtener_restaurante", (void*)2);
	dictionary_put(diccionario_consola, "consultar_platos", (void*)3);
	dictionary_put(diccionario_consola, "crear_pedido", (void*)4);
	dictionary_put(diccionario_consola, "guardar_pedido", (void*)5);
	dictionary_put(diccionario_consola, "aniadir_plato", (void*)6);
	dictionary_put(diccionario_consola, "guardar_plato", (void*)7);
	dictionary_put(diccionario_consola, "confirmar_pedido", (void*)8);
	dictionary_put(diccionario_consola, "plato_listo", (void*)9);
	dictionary_put(diccionario_consola, "consultar_pedido", (void*)10);
	dictionary_put(diccionario_consola, "obtener_pedido", (void*)11);
	dictionary_put(diccionario_consola, "finalizar_pedido", (void*)12);
	dictionary_put(diccionario_consola, "terminar_pedido", (void*)13);
	dictionary_put(diccionario_consola, "obtener_receta", (void*)14);
	dictionary_put(diccionario_consola, "atras", (void*)15);


	
	while(1)
	{
		cant=0;
		leido = readline("ingrese comando: ");
		if(!strcmp(leido,"") || !strncmp(leido," ",1))
		{
			error_show("No se ingreso nada\n");
			continue;
		}
			
		//string_to_lower(leido);
		ingresado = string_split(leido," ");
		
		if(! dictionary_has_key(diccionario_consola,ingresado[0]))
		{
			error_show("Comando desconocido\n");
			continue;
		}
		
		val = (int) dictionary_get(diccionario_consola,ingresado[0]);

		if( !permitidas[modulo][val])
		{
			error_show("Este modulo no acepta la funcion: %s\n",ingresado[0]);
			continue;
		}

		while(ingresado[cant]!=NULL)
			cant++;
		
		if((val == consultar_platos && modulo == SINDICATO))
			cant--;
		if(val == confirmar_pedido && (modulo == APP || modulo == RESTAURANTE) )
			cant++;
			
		if(cant-1 != control[val])
		{
			error_show("Cantidad de argumentos incorrecta\n");
			continue;
		}

		if(val == atras)
			return;
		
		ingresado = string_n_split(leido,2," ");
		
		parametros.ingresado = ingresado[1];
		
		parametros.op = val;
		parametros.modulo = modulo;
		pthread_create(&tid,NULL,serializar_y_enviar,(void*)&parametros);
		
			

	}
	

}

