#include "expose_metrics.h"
/** Mutex para sincronización de hilos */
pthread_mutex_t lock;

/** Métrica de Prometheus para el uso de CPU */
static prom_gauge_t *cpu_usage_metric;

/** Métrica de Prometheus para el uso de memoria */
static prom_gauge_t *memory_usage_metric;
/**metrica de Prometheus para uso de de meoria del sistema */
static prom_gauge_t *memory_system_metric;

/**Metricas para la lectura de datos de disco*/
static prom_gauge_t *sda_read_metric;
static prom_gauge_t *sda_write_metric;
static prom_gauge_t *sdb_read_metric;
static prom_gauge_t *sdb_write_metric;

/**Metricas para la lectura de datos de red*/
static prom_gauge_t *r_eth0_byte_metric;
static prom_gauge_t *t_eth0_byte_metric;
static prom_gauge_t *r_lo_byte_metric;
static prom_gauge_t *t_lo_byte_metric;

/**Metrica para conteo de procesos*/

static prom_gauge_t *count_ps_metric;

/** Metrica de conteo de cantidad de cambios de contexto*/

static prom_gauge_t *count_ctxt_metric;
/**Estructura para lectura de discos*/
#define NUM_DISKS 2

void update_cpu_gauge() {
  double usage = get_cpu_usage();
  if (usage >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(cpu_usage_metric, usage, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener el uso de CPU \n");
  }
}

void update_memory_gauge() {
  double usage = get_memory_usage();
  if (usage >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(memory_usage_metric, usage, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener el uso de memoria\n");
  }
}

void update_sys_gauge() {
  double usage = get_memory_system();
  if (usage >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(memory_system_metric, usage, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener el uso de memoria\n");
  }
}

void update_ps_count() {
  int ps_num = get_ps_count();
  if (ps_num >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(count_ps_metric, ps_num, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener la cantidad de procesos\n");
  }
}
void update_diskstat() {
  DiskStats stat_1 = get_disk_stats(0);
  DiskStats stat_2 = get_disk_stats(1);
  int datos[4] = {stat_1.reads, stat_1.writes, stat_2.reads, stat_2.writes};
  prom_gauge_t *titulos[4] = {sda_read_metric, sda_write_metric,
                              sdb_read_metric, sdb_write_metric};
  for (int i = 0; i < 4; i++) {
    if (datos[i] >= 0) {
      pthread_mutex_lock(&lock);
      prom_gauge_set(titulos[i], datos[i], NULL);
      pthread_mutex_unlock(&lock);
    } else {
      fprintf(stderr, "Error al obtener el uso de disco\n");
    }
  }
}

void update_netstat() {
  NetworkStat net_1 = get_net_stats(0);
  NetworkStat net_2 = get_net_stats(1);
  int datos_net[4] = {net_1.r_bytes, net_1.t_bytes, net_2.r_bytes,
                      net_2.t_bytes};
  prom_gauge_t *titulos[4] = {r_lo_byte_metric, t_lo_byte_metric,
                              r_eth0_byte_metric, t_eth0_byte_metric};
  for (int i = 0; i < 4; i++) {
    if (datos_net[i] >= 0) {
      pthread_mutex_lock(&lock);
      prom_gauge_set(titulos[i], datos_net[i], NULL);
      pthread_mutex_unlock(&lock);
    } else {
      fprintf(stderr, "Error al obtener el uso de red\n");
    }
  }
}

void update_context_count() {
  int ctxt_num = get_count_context();
  if (ctxt_num >= 0) {
    pthread_mutex_lock(&lock);
    prom_gauge_set(count_ctxt_metric, ctxt_num, NULL);
    pthread_mutex_unlock(&lock);
  } else {
    fprintf(stderr, "Error al obtener la cantidad de cambios de contexto\n");
  }
}
void *expose_metrics(void *arg) {
  (void)arg; // Argumento no utilizado

  // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
  promhttp_set_active_collector_registry(NULL);

  // Iniciamos el servidor HTTP en el puerto 9090
  struct MHD_Daemon *daemon =
      promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 9091, NULL, NULL);
  if (daemon == NULL) {
    fprintf(stderr, "Error al iniciar el servidor HTTP\n");
    return NULL;
  }

  // Mantenemos el servidor en ejecución
  while (1) {
    sleep(1);
  }

  // Nunca debería llegar aquí
  MHD_stop_daemon(daemon);
  return NULL;
}

void init_metrics() {

  // Inicializamos el mutex
  if (pthread_mutex_init(&lock, NULL) != 0) {
    fprintf(stderr, "Error al inicializar el mutex\n");
  }

  // Inicializamos el registro de coleccionistas de Prometheus
  if (prom_collector_registry_default_init() != 0) {
    fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
  }
  // Creamos la métrica para el uso de CPU
  cpu_usage_metric = prom_gauge_new("cpu_usage_percentage",
                                    "Porcentaje de uso de CPU", 0, NULL);
  if (cpu_usage_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
  }
  // Creamos la métrica para el uso de memoria
  memory_usage_metric =
      prom_gauge_new("memory_usage_percentage", "Porcentaje de uso de memoria",
                     0, (const char *[]){"label"});

  if (memory_usage_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de uso de memoria\n");
  }

  // Creamos la métrica para el uso de memoria del sistema
  memory_system_metric =
      prom_gauge_new("memory_system_percentage",
                     "porcentaje de uso de memoria del sistema", 0, NULL);

  if (memory_system_metric == NULL) {
    fprintf(stderr,
            "Error al crear la métrica de uso de memoria del sistema\n");
  }
  // Creamos la métrica para lecturas de sda
  sda_read_metric = prom_gauge_new("sda_read_metric",
                                   "cantidad de lecturas del sda", 0, NULL);
  if (sda_read_metric == NULL) {
    fprintf(stderr,
            "Error al crear la métrica de cantidad de escrituras del sda\n");
  }
  // Creamos la métrica para escrituras en sda
  sda_write_metric = prom_gauge_new("sda_write_metric",
                                    "cantidad de escrituras del sda", 0, NULL);

  if (sda_write_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de escriturar sda\n");
  }

  // Creamos la métrica para lecturas de sdb
  sdb_read_metric = prom_gauge_new("sdb_read_metric",
                                   "cantidad de lecturas del sdb", 0, NULL);

  if (sdb_read_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de lecturas de sdb\n");
  }
  // Creamos la métrica para escrituras en sdb
  sdb_write_metric = prom_gauge_new("sdb_write_metric",
                                    "cantidad de escrituras del sdb", 0, NULL);

  if (sdb_write_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de escrituras\n");
  }

  r_lo_byte_metric =
      prom_gauge_new("r_lo_byte_metric", "bytes recibidos en lookup", 0, NULL);
  t_lo_byte_metric = prom_gauge_new("t_lo_byte_metric",
                                    "bytes transmitidos en lookup", 0, NULL);
  r_eth0_byte_metric =
      prom_gauge_new("r_eth0_byte_metric", "bytes recibidos en red", 0, NULL);
  t_eth0_byte_metric = prom_gauge_new("t_eth0_byte_metric",
                                      "bytes transmitidos en red", 0, NULL);

  if (r_lo_byte_metric == NULL || t_lo_byte_metric == NULL ||
      r_eth0_byte_metric == NULL || t_eth0_byte_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de red\n");
  }

  // creamos la metrica de conteo de procesos
  count_ps_metric = prom_gauge_new("count_ps_metric",
                                   "cantidad de procesos en sistema", 0, NULL);

  if (count_ps_metric == NULL) {
    fprintf(stderr, "Error al crear la métrica de cantidad de procesos\n");
  } // Registramos las métricas en el registro por defecto

  // Creamos la métrica para conteo de cantidad de cambios de contexto
  count_ctxt_metric =
      prom_gauge_new("count_ctxt_metric",
                     "cantidad de cambios de contexto del sistema", 0, NULL);

  if (count_ps_metric == NULL) {
    fprintf(stderr,
            "Error al crear la métrica de cantidad de cambios de contexto.\n");
  }

  // Registramos las métricas en el registro por defecto
  if (prom_collector_registry_register_metric(cpu_usage_metric) != 0 ||
      prom_collector_registry_register_metric(memory_usage_metric) != 0 ||
      prom_collector_registry_register_metric(memory_system_metric) != 0 ||
      prom_collector_registry_register_metric(sda_read_metric) != 0 ||
      prom_collector_registry_register_metric(sda_write_metric) != 0 ||
      prom_collector_registry_register_metric(sdb_read_metric) != 0 ||
      prom_collector_registry_register_metric(sdb_write_metric) != 0 ||
      prom_collector_registry_register_metric(r_lo_byte_metric) != 0 ||
      prom_collector_registry_register_metric(t_lo_byte_metric) != 0 ||
      prom_collector_registry_register_metric(r_eth0_byte_metric) != 0 ||
      prom_collector_registry_register_metric(t_eth0_byte_metric) != 0 ||
      prom_collector_registry_register_metric(count_ps_metric) != 0 ||
      prom_collector_registry_register_metric(count_ctxt_metric) != 0) {
    fprintf(stderr, "Error al registrar las métricas\n");
  }
}

void destroy_mutex() { pthread_mutex_destroy(&lock); }
