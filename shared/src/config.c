#include "config.h"

t_config* leer_config(char* config_path)
{
	t_config *config;
	if((config=config_create(config_path)) == NULL)
	{
		printf(" No pude leer la config\n");
		exit(2);
	}
	return config;
}