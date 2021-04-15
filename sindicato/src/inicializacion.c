#include "inicializacion.h"

void inicializacion(int blocks, int block_size)
{
    inicializar_carpetas();
	crear_metadata(blocks,block_size);
	leer_metadata();
	inicializar_bloques();
	crear_archivo_Bitmap();
	cargar_Bitmap();
    return;
}

void inicializar_carpetas()
{
	//Crear directorio AFIP en el escritorio
	nueva_carpeta("");
	log_info(logger, "Creada carpeta AFIP");

	//Crear carpeta Files
	nueva_carpeta("/Files");
	log_info(logger, "Creada carpeta Files");

	//Crear carpeta Restaurantes
	nueva_carpeta("/Files/Restaurantes");
	log_info(logger, "Creada carpeta Restaurantes");

	//Crear carpeta Recetas
	nueva_carpeta("/Files/Recetas");
	log_info(logger, "Creada carpeta Recetas");

	//Crear carpeta Blocks
	nueva_carpeta("/Blocks");
	log_info(logger, "Creada carpeta Blocks");

	//Crear carpeta Metadata
	nueva_carpeta("/Metadata");
	log_info(logger, "Creada carpeta Metadata");
}

void nueva_carpeta(char *nueva_carpeta)
{
	char *ruta_general = string_new();
	string_append(&ruta_general, punto_montaje);
	string_append(&ruta_general, nueva_carpeta);
	mkdir(ruta_general, 0777);
	free(ruta_general);
}

void crear_metadata(int blocks, int block_size) {
	//Crear archivo metadata.
	if (!verificar_archivo("/Metadata/Metadata.AFIP"))
	{
		char * bloques;
		char * tamanio_bloques;
		bloques = string_itoa(blocks);
		tamanio_bloques = string_itoa(block_size);
		nuevo_archivo_config("/Metadata/Metadata.AFIP");
		log_info(logger, "Creado archivo metadata.AFIP");
		agregar_datos_config("/Metadata/Metadata.AFIP", "BLOCK_SIZE", tamanio_bloques);
		agregar_datos_config("/Metadata/Metadata.AFIP", "BLOCKS", bloques);
		agregar_datos_config("/Metadata/Metadata.AFIP", "MAGIC_NUMBER", "AFIP");
		log_info(logger, "Datos agregados a metadata.AFIP");
		free(bloques);
		free(tamanio_bloques);
	}
	else
	{
		error_show("Metadata ya existe\n");
	}

}

void nuevo_archivo_config(char *nuevo_archivo)
{
	char *ruta_archivo = string_new();
	string_append(&ruta_archivo, punto_montaje);
	string_append(&ruta_archivo, nuevo_archivo);
	FILE *archivo = fopen(ruta_archivo, "w+b");
	t_config *config_metadata = config_create(ruta_archivo);
	config_destroy(config_metadata);
	fclose(archivo);
	free(ruta_archivo);
}

void agregar_datos_config(char *archivo, char *parametro, char *valor)
{
	char *ruta_archivo = string_new();
	string_append(&ruta_archivo, punto_montaje);
	string_append(&ruta_archivo, archivo);
	FILE *archivoLeido = fopen(ruta_archivo, "a");
	t_config *config = leer_config(ruta_archivo);
	config_set_value(config, parametro, valor);
	config_save(config);
	config_destroy(config);
	fclose(archivoLeido);
	free(ruta_archivo);
}

void leer_metadata() {
	char *ruta_archivo = string_new();
	string_append(&ruta_archivo, punto_montaje);
	string_append(&ruta_archivo, "/Metadata/Metadata.AFIP");
	t_config* config_meta;
	config_meta = leer_config(ruta_archivo);
	tamanio_bloque = config_get_int_value(config_meta, "BLOCK_SIZE");
	cantidad_bloques = config_get_int_value(config_meta, "BLOCKS");
	config_destroy(config_meta);
	free(ruta_archivo);
}

void inicializar_bloques()
{
	//Creamos todos los bloques
	int indice;
	for (indice = 0; indice < cantidad_bloques; indice++)
	{
		crear_bloque(indice);
	}
	log_info(logger, "Bloques creados");
}

void crear_bloque(int numeroBloque)
{
	char *ruta = string_from_format("/Blocks/%d.AFIP",numeroBloque);
	crear_archivo(ruta);
	free(ruta);
}

void crear_archivo(char *nuevo_archivo)
{
	char *ruta_archivo = string_new();
	string_append(&ruta_archivo, punto_montaje);
	string_append(&ruta_archivo, nuevo_archivo);

	if (!verificar_archivo(nuevo_archivo))
	{
		FILE *archivo = fopen(ruta_archivo, "w");
		fclose(archivo);
	}
	else
	{
		//error_show("El directorio: %s ya existe\n", ruta_archivo);
	}
	free(ruta_archivo);
}

void crear_archivo_Bitmap()
{
	if (!verificar_archivo("/Metadata/Bitmap.bin"))
	{
		crear_archivo("/Metadata/Bitmap.bin");
		log_info(logger, "Archivo de Bitmap creado");
		char *ruta_archivo = string_new();
		string_append(&ruta_archivo, punto_montaje);
		string_append(&ruta_archivo, "/Metadata/Bitmap.bin");
		FILE *archivo = fopen(ruta_archivo, "wb+");
		char *puntero_a_bits = calloc(1, cantidad_bloques / 8);
		fwrite((void *)puntero_a_bits, cantidad_bloques / 8, 1, archivo);
		fflush(archivo);
		fclose(archivo);
		free(ruta_archivo);
		free(puntero_a_bits);
	}
	else
	{
		error_show("Ya existe el bitmap\n");
	}
}


//carga el bitmap con 0s
void cargar_Bitmap()
{
	log_info(logger, "Leyendo Archivo de Bitmap");
	char *ruta_archivo = string_new();
	string_append(&ruta_archivo, punto_montaje);
	string_append(&ruta_archivo, "/Metadata/Bitmap.bin");
	FILE *archivo = fopen(ruta_archivo, "rb+");
	fseek(archivo, 0, SEEK_END);
	int file_size = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char *bitarray_str = (char *)mmap(NULL, file_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fileno(archivo), 0);
	if (bitarray_str == (char *)-1)
	{
		error_show("Fallo el mmap\n");
	}
	fread((void *)bitarray_str, sizeof(char), file_size, archivo);
	bitmap = bitarray_create_with_mode(bitarray_str, file_size, MSB_FIRST);
	log_info(logger, "Bitmap cargado completamente");
	fclose(archivo);
	free(ruta_archivo);
}