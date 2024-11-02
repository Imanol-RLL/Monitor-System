/**
 * @file main.c
 * @brief Entry point of the system
 */
#include "expose_metrics.h"
#include "signal.h"
#include <stdbool.h>
/**
 * @def SLEEP_TIME
 * @brief Define un tiempo de actualizacion constante
 */
#define SLEEP_TIME 1

/**
 * @brief Funcion de manejo de señales del sistema operativo
 *
 * Alerta sobre una señal recibida y detiene el sistema de forma segura
 *
 */
void sigsegv_handler(int signum) {
  fprintf(stderr, "Error: Se recibió la señal SIGSEGV, cerrando el servidor de "
                  "manera segura.\n");
  exit(EXIT_FAILURE);
}
/**
 * @brief Configura el manejo de señales para errores de segmentación.
 *
 * Esta función establece un manejador personalizado para la señal SIGSEGV,
 * que se ejecutará cuando ocurra un error de segmentación.
 */
void setup_signal_handling() { signal(SIGSEGV, sigsegv_handler); }
/**
 * @brief Punto de entrada del programa.
 *
 * Esta función es el punto de entrada principal del programa y maneja los
 * argumentos proporcionados desde la línea de comandos.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Array de cadenas que contiene los argumentos de la línea de
 * comandos.
 *
 * @return int Código de salida del programa. Retorna 0 si la ejecución es
 * exitosa, o un valor diferente si ocurre un error.
 */
int main(int argc, char *argv[]) {

  init_metrics();
  setup_signal_handling();
  // Creamos un hilo para exponer las métricas vía HTTP
  pthread_t tid;
  if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0) {
    fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
    return EXIT_FAILURE;
  }
  while (true) // Bucle principal para actualizar las métricas cada segundo
  {
    update_cpu_gauge();
    update_memory_gauge();
    update_sys_gauge();
    update_diskstat();
    update_netstat();
    update_ps_count();
    update_context_count();
    sleep(SLEEP_TIME);
  }

  return EXIT_SUCCESS;
}