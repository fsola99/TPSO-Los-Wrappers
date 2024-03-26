# Sistema de Pedidos de Comida - Trabajo Práctico de Sistemas Operativos

Este repositorio contiene el trabajo práctico realizado como parte del curso de Sistemas Operativos en la UTN FRBA por el grupo "Los Wrappers", con Federico Sola como miembro del equipo. El objetivo principal de este proyecto fue adquirir conocimientos prácticos sobre el uso de herramientas de programación y APIs proporcionadas por los sistemas operativos, así como comprender aspectos del diseño de un sistema operativo y familiarizarse con técnicas de programación de sistemas en el contexto de Linux utilizando el lenguaje de programación C.

## Descripción del Proyecto

El trabajo práctico consistió en desarrollar una solución para simular un sistema distribuido que gestiona pedidos de comida. Se utilizó la metodología Iterativa Incremental para el desarrollo, donde se implementaron módulos específicos en etapas sucesivas para luego integrarlos en un sistema completo. Los componentes del sistema trabajan en conjunto para la planificación y ejecución de diversas operaciones relacionadas con la gestión de pedidos entre restaurantes y repartidores.

## Componentes del Sistema

El proyecto consta de los siguientes componentes:

- **Cliente**: Encargado de consultar restaurantes y recetas, así como de realizar, confirmar y consultar pedidos. Actúa como intermediario entre la aplicación y los restaurantes, o puede comunicarse directamente con estos últimos, según la modalidad utilizada.

- **App**: Centraliza, administra y planifica pedidos entre repartidores y actúa como vínculo entre clientes y restaurantes registrados en la aplicación.

- **Comanda**: Almacena y mantiene los pedidos activos en la aplicación en un momento dado.

- **Restaurante**: Administra y planifica los pedidos entre los empleados y cocineros del restaurante.

- **Sindicato**: Almacena y gestiona la información de los restaurantes y los pedidos realizados a ellos.

## Contribución

¡Las contribuciones son bienvenidas! Si deseas contribuir al proyecto, por favor sigue estas pautas:
- Realiza un fork del repositorio.
- Crea una nueva rama para tu funcionalidad (`git checkout -b feature/nueva-funcionalidad`).
- Realiza tus cambios y commitea (`git commit -am 'Agrega nueva funcionalidad'`).
- Sube tus cambios a tu repositorio en GitHub (`git push origin feature/nueva-funcionalidad`).
- Realiza un pull request a la rama `main` del repositorio original.

---
Este proyecto fue desarrollado como parte del trabajo práctico para la materia de Sistemas Operativos en la UTN FRBA.
