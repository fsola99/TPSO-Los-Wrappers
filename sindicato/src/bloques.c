#include "bloques.h"

int buscar_bloque_desocupado()
{
	int i = 0; //1 o 0
	//Lock
	while (i < bitarray_get_max_bit(bitmap) && bitarray_test_bit(bitmap, i) != 0)
	{
		i++;
	}
	//Unlock
	if (i == cantidad_bloques)
	{
		error_show("TODOS LOS BLOQUES OCUPADOS\n");
	}
	log_info(logger, "El primer bloque desocupado es: %d", i);
	return i;
}

/*int verificar_bloque_completo(int nro_bloque)
{
	char *ruta = obtener_ruta_bloque(nro_bloque);
	FILE *archivo = fopen(ruta, "wb+");
	fseek(archivo, 0, SEEK_END);
	int file_size = ftell(archivo);
	fclose(archivo);
	if (file_size == tamanio_bloque)
	{
		//Devuelve que esta llenoide
		file_size = -1;
	}
	free(ruta);
	return file_size;
}*/

char *obtener_ruta_bloque(int pos_bloque)
{
	return string_from_format("%s/Blocks/%d.AFIP", punto_montaje, pos_bloque);
}

int escribir_bloque(char * texto) { //Retorna initial_block
	int tamanio_texto = (string_length(texto));
	int bloques_necesarios = ceil(tamanio_texto / (tamanio_bloque - sizeof(uint32_t)));
	int initial_block = 0;
	char *ruta_bloque;
	char * duplicado = malloc(tamanio_texto +1);
	strcpy(duplicado,texto);

	for (int i = 0; i <= bloques_necesarios && tamanio_texto > 0;i ++) {
		int numero_bloque = buscar_bloque_desocupado();
		if (i==0) {
			initial_block = numero_bloque;
		}
		ruta_bloque = obtener_ruta_bloque(numero_bloque);
		FILE *archivo_bloque = fopen(ruta_bloque, "w+");

		char *datos;
		if(tamanio_texto < tamanio_bloque-sizeof(uint32_t))
		{
			datos = malloc(tamanio_texto);
			memcpy(datos,duplicado + ((tamanio_bloque-sizeof(uint32_t))*i), tamanio_texto) ;
			fwrite(datos, 1, tamanio_texto, archivo_bloque);
			tamanio_texto -= tamanio_texto;
		}
		else
		{
			datos = malloc(tamanio_bloque-sizeof(uint32_t));
			memcpy(datos,duplicado + ((tamanio_bloque-sizeof(uint32_t))*i), tamanio_bloque-sizeof(uint32_t));
			fwrite(datos, 1, tamanio_bloque-sizeof(uint32_t), archivo_bloque);
			tamanio_texto -= (tamanio_bloque-sizeof(uint32_t));
		}


		bitarray_clean_bit(bitmap, (size_t)numero_bloque);
		bitarray_set_bit(bitmap, (size_t)numero_bloque);
		log_info(logger,"Se asigno el bloque %d",numero_bloque);
		if (tamanio_texto > 0)
		{
			int prox_bloque = buscar_bloque_desocupado();
			putw(((uint32_t)prox_bloque), archivo_bloque);
		}
		free(datos);
		fclose(archivo_bloque);
		free(ruta_bloque);
	}
	free(duplicado);
	return initial_block;
}

int parametros_check(char **datos_restaurante)
{
	int i = 0;
	while (datos_restaurante[i] != NULL)
	{
		i++;
	}
	return i;
}

int obtener_initial_block(char *archivo)
{
	int initial_block;
	char *ruta = string_new();
	string_append(&ruta, punto_montaje);
	string_append(&ruta, archivo);

	t_config *config;
	config = leer_config(ruta);
	initial_block = config_get_int_value(config, "INITIAL_BLOCK");
	config_destroy(config);
	free(ruta);
	return initial_block;
}

char * obtener_datos_bloques(int bloque_inicial)
{
	int datos_por_bloque = tamanio_bloque - sizeof(uint32_t);	
	int prox_bloque=bloque_inicial;
	char * ruta_bloque;
	char * datos = string_new();
	while(prox_bloque!=EOF) {
		char * datos_bloque=malloc(tamanio_bloque);
		ruta_bloque = obtener_ruta_bloque(prox_bloque);
		FILE * archivo_bloque = fopen(ruta_bloque,"r");
		fgets(datos_bloque, datos_por_bloque + 1, archivo_bloque);
		string_append(&datos,datos_bloque);
		fseek(archivo_bloque, tamanio_bloque - sizeof(uint32_t), SEEK_SET);
		prox_bloque = getw(archivo_bloque);
		fclose(archivo_bloque);
		free(ruta_bloque);
		free(datos_bloque);
	}
	return datos;
}

char* obtener_bloques_usados(int bloque_inicial)
{
	int prox_bloque=bloque_inicial;
	char * ruta_bloque;
	char * ret_bloques_ocupados = string_new();
	while(prox_bloque!=EOF) {
		ruta_bloque = obtener_ruta_bloque(prox_bloque);
		FILE * archivo_bloque = fopen(ruta_bloque,"r");
		fseek(archivo_bloque, tamanio_bloque - sizeof(uint32_t), SEEK_SET);
		char * bloque = string_itoa(prox_bloque);
		string_append(&ret_bloques_ocupados,bloque);
		string_append(&ret_bloques_ocupados," ");
		prox_bloque = getw(archivo_bloque);
		fclose(archivo_bloque);
		free(ruta_bloque);
		free(bloque);
	}
	return ret_bloques_ocupados;
}	

void eliminar_bloques(char* bloques)
{
	char *ruta_bloque;
	char **array_bloques=string_split(bloques," ");
	int i=0;
	while(array_bloques[i]!=NULL)
	{
		ruta_bloque=obtener_ruta_bloque(atoi(array_bloques[i]));
		int bloque = atoi(array_bloques[i]);
		bitarray_clean_bit(bitmap, (size_t)bloque);
		remove(ruta_bloque);
		log_info(logger,"Se desasignó el bloque %d",bloque);
		i++;
		free(ruta_bloque);
	}
	liberar_recibido(array_bloques);
}

