#include "metrics.h"
#include <ctype.h>
#include <dirent.h>
#include <math.h>

#define BUFFER_SIZE 256
#define NUM_DISKS 2
#define NUM_NETS 2

double get_memory_usage() {
  FILE *fp;
  char buffer[BUFFER_SIZE];
  int free_mem = 0;
  // unsigned long long total_mem = 0;
  int total_mem = 0;

  // Abrir el archivo /proc/meminfo
  fp = fopen("/proc/meminfo", "r");
  if (fp == NULL) {
    perror("Error al abrir /proc/meminfo");
    return -1.0;
  }

  // Leer los valores de memoria total y disponible
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    if (sscanf(buffer, "MemTotal: %u kB", &total_mem) == 1) {
      continue; // MemTotal encontrado
    }
    if (sscanf(buffer, "MemAvailable: %u kB", &free_mem) == 1) {
      break; // MemAvailable encontrado, podemos dejar de leer
    }
  }

  fclose(fp);

  // Verificar si se encontraron ambos valores
  if (total_mem == 0 || free_mem == 0) {
    fprintf(stderr,
            "Error al leer la información de memoria desde /proc/meminfo\n");
    return -1.0;
  }

  // Calcular el porcentaje de uso de memoria
  double used_mem = (double)total_mem - free_mem;
  total_mem == 0 ? total_mem = 1 : total_mem;
  double mem_usage_percent = (used_mem / total_mem) * 100.0;

  return redon(mem_usage_percent);
}

double get_memory_system() {
  FILE *fp;
  char buffer[BUFFER_SIZE];
  int sys_mem = 0;
  // unsigned long long total_mem = 0;
  int total_mem = 0;

  // Abrir el archivo /proc/meminfo
  fp = fopen("/proc/meminfo", "r");
  if (fp == NULL) {
    perror("Error al abrir /proc/meminfo");
    return -1.0;
  }

  // Leer los valores de memoria total y de sistema
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    if (sscanf(buffer, "MemTotal: %u kB", &total_mem) == 1) {
      continue; // MemTotal encontrado
    }
    if (sscanf(buffer, "KernelStack: %u kB", &sys_mem) == 1) {
      break; // MemAvailable encontrado, podemos dejar de leer
    }
  }

  fclose(fp);

  // Verificar si se encontraron ambos valores
  if (total_mem == 0 || sys_mem == 0) {
    fprintf(stderr,
            "Error al leer la información de memoria desde /proc/meminfo\n");
    return -1.0;
  }

  return (double)sys_mem;
}

double get_cpu_usage() {
  static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0,
                            prev_idle = 0, prev_iowait = 0, prev_irq = 0,
                            prev_softirq = 0, prev_steal = 0;
  unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
  int totald, idled;
  double cpu_usage_percent = 1;

  // Abrir el archivo /proc/stat
  FILE *fp = fopen("/proc/stat", "r");
  if (fp == NULL) {
    perror("Error al abrir /proc/stat");
    return -1.0;
  }

  char buffer[BUFFER_SIZE * 4];
  if (fgets(buffer, sizeof(buffer), fp) == NULL) {
    perror("Error al leer /proc/stat");
    fclose(fp);
    return -1.0;
  }
  fclose(fp);

  // Analizar los valores de tiempo de CPU
  int ret =
      sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user,
             &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
  if (ret < 8) {
    fprintf(stderr, "Error al parsear /proc/stat\n");
    return -1.0;
  }

  // Calcular las diferencias entre las lecturas actuales y anteriores
  unsigned long long prev_idle_total = prev_idle + prev_iowait;
  unsigned long long idle_total = idle + iowait;

  unsigned long long prev_non_idle = prev_user + prev_nice + prev_system +
                                     prev_irq + prev_softirq + prev_steal;
  unsigned long long non_idle = user + nice + system + irq + softirq + steal;

  unsigned long long prev_total = prev_idle_total + prev_non_idle;
  unsigned long long total = idle_total + non_idle;

  totald = total - prev_total;
  idled = idle_total - prev_idle_total;

  if (totald == 0 || (totald - idled) == 0) {
    fprintf(stderr, "Totald es cero, no se puede calcular el uso de CPU!\n");
    return -1.0;
  }
  // Calcular el porcentaje de uso de CPU
  cpu_usage_percent = ((double)(totald - idled) / (double)totald) * 100.0;

  // Actualizar los valores anteriores para la siguiente lectura
  prev_user = user;
  prev_nice = nice;
  prev_system = system;
  prev_idle = idle;
  prev_iowait = iowait;
  prev_irq = irq;
  prev_softirq = softirq;
  prev_steal = steal;
  return redon(cpu_usage_percent);
}

// Función para obtener las estadísticas de un disco
DiskStats get_disk_stats(int num) {
  FILE *fp;
  char buffer[BUFFER_SIZE];
  DiskStats stats[NUM_DISKS];

  // Inicializar estadísticas
  for (int i = 0; i < NUM_DISKS; i++) {
    strcpy(stats[i].name, (i == 0) ? "sda" : "sdb");
    stats[i].reads = 0;
    stats[i].writes = 0;
    stats[i].read_time = 0;
    stats[i].write_time = 0;
  }

  // Abrir el archivo /proc/diskstats
  fp = fopen("/proc/diskstats", "r");
  if (fp == NULL) {
    perror("Error al abrir /proc/diskstats");
    return stats[num];
  }

  // Leer las estadísticas de disco
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    char device[32];
    unsigned long long reads, writes, read_time, write_time;

    // Buscar datos para sda y sdb
    if (sscanf(buffer, "%*d %*d %s %llu %*u %llu %llu %llu", device, &reads,
               &writes, &read_time, &write_time) == 5) {
      for (int i = 0; i < NUM_DISKS; i++) {
        if (strcmp(device, stats[i].name) == 0) {
          stats[i].reads = reads;
          stats[i].writes = writes;
          stats[i].read_time = read_time;
          stats[i].write_time = write_time;
        }
      }
    }
  }

  fclose(fp);
  return stats[num]; // retorna una estadistica de 1 disco en una estructura
                     // DiskStat
}

NetworkStat get_net_stats(int num) {
  FILE *fp;
  char buffer[BUFFER_SIZE];
  NetworkStat stats[NUM_NETS];

  // Inicializar estadísticas

  strcpy(stats[num].interface,
         (num == 0) ? "lo:" : "enp2s0:"); // nombres de lookup y mi placa de red
  stats[num].r_bytes = 0;
  stats[num].t_bytes = 0;

  // Abrir el archivo /proc/diskstats
  fp = fopen("/proc/net/dev", "r");
  if (fp == NULL) {
    perror("Error al abrir /proc/net/dev");
    return stats[num];
  }

  // Leer las estadísticas de disco
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    char device[32];
    unsigned long long rx_b, tx_b;
    // Saltar las dos primeras líneas del encabezado
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);

    // Verificar si se puede extraer correctamente el dispositivo y sus métricas
    if (sscanf(buffer, "%8s %llu %*u %*u %*u %*u %*u %*u %*u  %llu", device,
               &rx_b, &tx_b) == 3) {
      // Comprobar si el dispositivo actual es el que estamos buscando
      if (strcmp(device, stats[num].interface) == 0) {
        // Actualizar los bytes recibidos y transmitidos para este dispositivo
        stats[num].r_bytes = rx_b;
        stats[num].t_bytes = tx_b;
      }
    }
  }
  fclose(fp);
  return stats[num]; // retorna una estadistica de 1 disco en una estructura
                     // DiskStat
}

int get_ps_count() {
  struct dirent *entry;
  DIR *proc_dir;
  int process_count = 0;

  proc_dir = opendir("/proc");
  if (proc_dir == NULL) {
    perror("Error al abrir /proc");
    return -1;
  }
  // Recorre el directorio /proc y cuenta los directorios numéricos (PID)
  while ((entry = readdir(proc_dir)) != NULL) {
    // Verifica si el nombre del directorio es numérico (es un PID)
    if (isdigit(entry->d_name[0])) {
      process_count++;
    }
  }
  // Cerrar el directorio /proc
  closedir(proc_dir);
  // Retorna la cantidad de procesos activos
  return process_count;
}

unsigned long long get_count_context() {
  FILE *fp;
  char buffer[256];
  unsigned long long context_switches =
      0; // Variable para almacenar el número de cambios de contexto

  fp = fopen("/proc/stat", "r");
  if (fp == NULL) {
    perror("Error al abrir /proc/stat");
    return -1;
  }

  // Leer línea por línea el archivo /proc/stat
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    // Buscar la línea que contiene el número de cambios de contexto
    if (sscanf(buffer, "ctxt %llu", &context_switches) == 1) {
      break; // Encontrado el número de cambios de contexto
    }
  }

  fclose(fp);

  return context_switches;
}

double redon(double numero) {
  double factor = pow(10, 3);
  return (numero * factor + 0.5) / factor;
}
