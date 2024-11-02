/**
 * @file metrics.h
 * @brief Funciones para obtener el uso de CPU y memoria desde el sistema de
 * archivos /proc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/**
 * @def BUFFER_SIZE
 * @brief Define el tamaño del buffer utilizado para leer datos del sistema.
 *
 * Esta macro define el tamaño del buffer que se utilizará en las funciones
 * de lectura de archivos dentro de este programa.
 */
#define BUFFER_SIZE 256
/**
 * @def NUM_DISKS
 * @brief Define cantidad de discos admitidos en el sistema
 */
#define NUM_DISKS 2
/**
 * @def NUM_CPUS
 * @brief Define la cantidad de nucleos del procesador
 */
#define NUM_CPUS 6
/**
 * @struct DiskStats
 * @brief Estructura que contiene las estadísticas de escritura y lectura de
 * disco
 *
 * La estructra DiskStats alamacena en 4 campos los datos utiles de disco
 * contenidos en /proc/diskstats interfaz de red, incluyendo los bytes y
 * paquetes recibidos y transmitidos.
 */

typedef struct {
  char name[8];             /**< Nombre del disco (e.g., "sda", "sdb")*/
  unsigned long reads;      /**< Número de lecturas*/
  unsigned long writes;     /**< Número de escrituras*/
  unsigned long read_time;  /**< Tiempo de lectura en milisegundos*/
  unsigned long write_time; /**< Tiempo de escritura en milisegundos*/
} DiskStats;

/**
 * @struct NetworkStat
 * @brief Estructura que contiene las estadísticas de red de una interfaz.
 *
 * La estructura NetworkStats almacena la información de tráfico de red de una
 * interfaz de red, incluyendo los bytes y paquetes recibidos y transmitidos.
 */

typedef struct {
  char interface[8];     /**< espacio para almacenar nombre de interface*/
  unsigned long r_bytes; /**<bytes recibidos*/
  unsigned long t_bytes; /**< bytes transmitidos*/
} NetworkStat;
/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de
 * error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();
/**
 * @brief Obtiene el valor en Kb de uso de memoria del kernel desde
 * /proc/meminfo.
 *
 *
 * @return valor double de los kb que utiliza el kernel
 */
double get_memory_system();

/**
 * @brief Redondea el numero y corrije errores numericos.
 *
 *
 * @return valor redondeado con cifras significativas
 */
double redon(double numero);

/**
 * @brief obtiene las estadisticas del disco, como escritura y lectura
 * @param num selecciona el numero de disco que puede contener
 * @return estructura DiskStat con los detalles del sistema
 */
DiskStats get_disk_stats(int num);
/**
 * @brief obtiene las estadisticas de la red
 * la transmision y recepcion de bytes leidos en /proc/dev/net
 * @param num selecciona la red de la cual toma lectura
 * @return estructura NetworkStat con los detalles de la red
 */
NetworkStat get_net_stats(int num);

/**
 * @brief obtiene la cantidad de procesos activos en el sistema
 * extrae la informacion de /proc
 * @return numero con la cantidad de procesos activos
 */
int get_ps_count();

/**
 * @brief obtiene la cantidad de cambios de contexto de los procesadores
 * @return media de cambios de contexto de los procesadores
 */
unsigned long long get_count_context();