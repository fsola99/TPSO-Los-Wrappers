#ifndef CONSOLA_CLIENTE_H
#define CONSOLA_CLIENTE_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<commons/string.h>
#include<readline/readline.h>
#include "serializer.h"
#include<commons/collections/dictionary.h>
#include <commons/error.h>
#include <pthread.h>


//consola

void consola(int socket, int modulo);



#endif /* CONSOLA_H */