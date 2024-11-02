/**
 * @file expose_metrics.h
 * @brief Programa para leer el uso de CPU y memoria y exponerlos como métricas
 * de Prometheus.
 */
#include "metrics.h"
#include <errno.h>
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para sleep
/**
 * @def BUFFER_SIZE
 * @brief Define el tamaño del buffer utilizado para leer datos del sistema.
 *
 * Esta macro define el tamaño del buffer que se utilizará en las funciones
 * de lectura de archivos dentro de este programa.
 */
#define BUFFER_SIZE 256

/**
 * @brief Actualiza la métrica de uso de CPU.
 */
void update_cpu_gauge();

/**
 * @brief Actualiza la métrica de uso de memoria.
 */
void update_memory_gauge();
/**
 * @brief Actualiza la métrica de uso de memoria por parte del sistema.
 */
void update_sys_gauge();
/**
 * @brief Actualiza la metrica de estadisticas de disco.
 */
void update_diskstat();
/**
 * @brief Actualiza la metrica de estadistica de red.
 */
void update_netstat();
/**
 * @brief Actualiza la metrica de cantidad de procesos.
 */
void update_ps_count();
/**
 * @brief Actualiza la metrica de cambios de contexto.
 */
void update_context_count();
/**
 * @brief Función del hilo para exponer las métricas vía HTTP en el puerto 8000.
 * @param arg Argumento no utilizado.
 * @return NULL
 */
void *expose_metrics(void *arg);
/**
 * @brief Inicializar mutex y métricas.
 */
void init_metrics();
/**
 * @brief Destructor de mutex
 */
void destroy_mutex();