#include <stdio.h>

int main() {
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <fcntl.h>
    char *archivoSal;
    char *archivoErr;
    char *archivoEnt;

int redireccion_de_entrada(char *archivoEnt);
    int redireccion_de_entrada(char *archivoEnt);
    int main() {
        return 0;
    }

    char *archivoSal;
    char *archivoErr;
    char *archivoEnt;
    int fdEntrada = open(archivoEnt, O_RDONLY);
    int redireccion_de_entrada(char *, int fdenter) {
        int fdEntrada = open(archivoEnt, O_RDONLY);
        if (fdEntrada == -1) {
            perror("Error durante la apertura");
            return 1;
        }
    }

    // Descriptor de archivo para redirección de salida
    int fdSalida = open(archivoSal, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fdSalida == -1) {
        perror("Error al salir");
        close(fdEntrada);
        return 1;
    }

    // Descriptor de archivo para redirección de error
    int fdError = open(archivoErr, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fdError == -1) {
        perror("Error al abrir el archivo de error");
        close(fdEntrada);
        close(fdSalida);
        return 1;
    }

    // Redirigir entrada estándar al archivo de entrada
    dup2(fdEntrada, STDIN_FILENO);

    // Redirigir salida estándar al archivo de salida
    dup2(fdSalida, STDOUT_FILENO);

    // Redirigir error estándar al archivo de error
    dup2(fdError, STDERR_FILENO);

    // Cerrar los descriptores de archivo no necesarios
    close(fdEntrada);
    close(fdSalida);
    close(fdError);

    // Aquí puedes ejecutar tu código, y la entrada, salida y error estándar estarán redirigidos a los archivos especificados.

    // Ejemplo de código (puedes reemplazarlo con tu propia lógica)
    printf("Esto debería ir al archivo de salida\n");
    fprintf(stderr, "Esto debería ir al archivo de error\n");

    return 0;
}

