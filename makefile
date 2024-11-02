# Variables
CC = gcc                      # Compilador
CFLAGS = -Wextra -Iinclude -Ilib/prom/include    # Flags de compilación
SRC = src/main.c src/metrics.c src/expose_metrics.c  # Archivos fuente
LIBS = -Llib/prom -lprom -lpromhttp -pthread  # Biblioteca de Prometheus
OBJ = $(SRC:.c=.o)             # Archivos objeto
EXEC = metrics                 # Nombre del ejecutable
export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH

# Regla principal
all: $(EXEC)

# Regla para generar el ejecutable
$(EXEC): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)
	#./$(EXEC)  # Ejecuta el binario tras compilar

# Regla para generar los archivos .o (objetos) de cada archivo fuente
src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f src/*.o $(EXEC)
# Regla de reconstrucción total
rebuild: clean all

# Declaración de "phony targets" para evitar conflictos con archivos del mismo nombre
.PHONY: all clean rebuild test
