#include "serializer.h"
#include <unistd.h>

//SERIALIZAMOS

void *serializar_y_enviar(void *arg)
{
	parametros_t *p;
	p = (parametros_t *)arg;
	if(p->modulo == CLIENTE)
		send(p->socket, ".", 2, 0);
	
	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;

	if (p->ingresado == NULL)
	{
		t_paquete *paquete = malloc(sizeof(t_paquete));
		paquete->codigo_operacion = p->op;

		int tamanio_paquete = sizeof(uint8_t) + sizeof(uint32_t);
		void *a_enviar = malloc(tamanio_paquete); //codigo de op
		int offset = 0;
		int control = 0;
		memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
		offset += sizeof(uint8_t);
		memcpy(a_enviar + offset, &control, sizeof(uint32_t));

		send(p->socket, a_enviar, tamanio_paquete, 0);
		free(a_enviar);
		free(paquete->buffer);
		free(paquete);
	}
	else
	{
		//RESERVAMOS MEMORIA PARA EL BUFFER

		buffer->size += strlen(p->ingresado) + 1 + sizeof(uint32_t);

		//RELLENAMOS EL BUFFER
		void *stream = malloc(buffer->size);
		int offset = 0; // Desplazamiento
		int palabra_length;

		palabra_length = strlen(p->ingresado) + 1;
		memcpy(stream + offset, &palabra_length, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, p->ingresado, strlen(p->ingresado) + 1);
		buffer->stream = stream;

		//CREAMOS EL PAQUETE!
		t_paquete *paquete = malloc(sizeof(t_paquete));
		paquete->codigo_operacion = p->op;

		paquete->buffer = buffer;

		int tamanio_paquete = sizeof(uint8_t) + sizeof(uint32_t) + buffer->size;
		void *a_enviar = malloc(tamanio_paquete); //codigo de op + el size + el tamanio del stream
		offset = 0;

		memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
		offset += sizeof(uint8_t);
		memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

		//ENVIAR!

		send(p->socket, a_enviar, tamanio_paquete, 0);

		//LIBERAMOS MEMORIA USADA
		free(a_enviar);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	return 0;
}

//DESERIALIZAMOS

t_paquete *recibir_y_desempaquetar(int socket)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	recv(socket, &(paquete->codigo_operacion), sizeof(uint8_t), 0);
	paquete->buffer = malloc(sizeof(t_buffer));
	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
	if (paquete->buffer->size == 0)
	{
		paquete->buffer->stream = NULL;
		return paquete;
	}
	else
	{
		paquete->buffer->stream = malloc(paquete->buffer->size);
		recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);
		return paquete;
	}
}

char *deserializar_palabra(t_buffer *buffer)
{
	t_palabra palabra;
	//Deserializo la palabra
	memcpy(&(palabra.length_palabra), buffer->stream, sizeof(uint32_t));
	buffer->stream += sizeof(uint32_t);
	palabra.nombre_palabra = malloc(palabra.length_palabra);
	memcpy(palabra.nombre_palabra, buffer->stream, palabra.length_palabra);
	return palabra.nombre_palabra;
}
char** deserializar_paquete(t_paquete* paquete) {
	char* texto = deserializar_palabra(paquete->buffer);
	char** recibido = string_split(texto," ");
	free(texto);
	return recibido;
}
void liberar_paquete(t_paquete *paquete, void* stream)
{
	free(stream);
	free(paquete->buffer);
	free(paquete);
}
void liberar_recibido(char **recibido)
{
	int i = 0;
	while (recibido[i] != NULL)
	{
		free(recibido[i]);
		i++;
	}
	free(recibido);
}

void a_enviar(char *datos, int socket, int modulo, int cod_op)
{
	parametros_t arg;
	arg.ingresado = datos;
	arg.socket = socket;
	arg.modulo = modulo;
	arg.op = cod_op;

	serializar_y_enviar((void *)&arg);
}
