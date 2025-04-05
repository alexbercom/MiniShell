#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

#include "parser.h"

tline * line;

pid_t * pid;
int ** pipe_fd;

FILE *fEntrada, *fSalida, *fError;
int saved_input, saved_output, saved_error; //Guardan las entradas y salidas estandar originales para restaurarlas más adelante

void ejecutar_comando(int i);

int redireccion_entrada(char * entrada);
int redireccion_salida(char * salida);
int redireccion_error(char * error);
void restaurarES();

void cd();

bool comprobar_n(char *n);
void Umask(tcommand *command, mode_t *mask);

int main(void) {
    char buf[1024];
    int i;
    int status;

    mode_t mascara = 22;

    signal(SIGINT , SIG_IGN);

    printf("msh> ");
    while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);   //funcion que recoge los comandos de una línea
        if ((line->ncommands == 0) || (line->background)) {   //Comprobacion line valida
            printf("\nmsh> ");
            continue;
        }

        //Redirecciones//
        //Error
        if (line->redirect_error != NULL) {
            saved_error = dup(STDERR_FILENO);
            if (redireccion_error(line->redirect_error) != 0) {
                //Si hay un error, se restauran las entradas y salidas originales y se vuelve a mostrar el prompt
                restaurarES();
                printf("\nmsh> ");
                continue;
            }
        }
        //Entrada
        if (line->redirect_input != NULL) {
            saved_input = dup(STDIN_FILENO);
            //Si hay un error, se restauran las entradas y salidas originales y se vuelve a mostrar el prompt
            if (redireccion_entrada(line->redirect_input) != 0) {
                restaurarES();
                printf("\nmsh> ");
                continue;
            }
        }
        //Salida
        if (line->redirect_output != NULL) {
            saved_output = dup(STDOUT_FILENO);
            //Si hay un error, se restauran las entradas y salidas originales y se vuelve a mostrar el prompt
            if (redireccion_salida(line->redirect_output) != 0) {
                restaurarES();
                printf("\nmsh> ");
                continue;
            }
        }

        //Declaracion del puntero de pids
        pid = (pid_t *) malloc(sizeof line->ncommands);

        //Posibles mandatos//

        //cd
        if (strcmp(line->commands[0].argv[0], "cd") == 0) {
            if (line->ncommands == 1)
                cd();
            else {
                fprintf(stderr, "No se puede ejecutar 'cd' con más mandatos\n");
                printf("\nmsh> ");
                continue;
            }
        }

        //exit
        else if (strcmp(line->commands[0].argv[0], "exit") == 0) {
            if (line->ncommands == 1) {
                printf("Fin del programa\n");
                exit(0);
            } else {
                fprintf(stderr, "No se puede ejecutar 'exit' con más mandatos\n");
                printf("\nmsh> ");
                continue;
            }
        }

        //umask
        else if (strcmp(line->commands[0].argv[0], "umask") == 0)
            Umask(line->commands, &mascara);

        //Si no es ninguno de los anteriores
        else {
            //Se comprueba si hay algun mandato no valido
            int mandato_valido = 0;
            for (i=0; i < line->ncommands; i++) {
                if (line->commands[i].filename == NULL) {
                    fprintf(stderr, "%s: No se encuentra el mandato\n", line->commands[i].argv[0]);
                    mandato_valido = 1;     //hay un mandato no valido
                    break;
                }
            }
            if (mandato_valido == 1) {   //Se vuelve al inicio
                printf("\nmsh> ");
                continue;
            }

            //En otro caso:
            //Creacion de tuberias
            if (line->ncommands > 1) {
                pipe_fd=(int**) malloc ((line->ncommands-1)*sizeof(int*));
                for (i=0; i<line->ncommands-1; i++) {
                    pipe_fd[i]=(int*)malloc(2*sizeof(int));
                    if (pipe(pipe_fd[i]) == -1) {
                        fprintf(stderr, "Error al crear la tubería\n");
                        restaurarES();
                        printf("\nmsh> ");
                        continue;
                    }
                }
            }

            //Bucle de ejecucion de comandos
            for (i = 0; i < line->ncommands; i++) {
                //Creacion de procesos hijo
                pid[i] = fork();
                if (pid[i] == -1) {
                    fprintf(stderr, "Error en fork");
                    exit(EXIT_FAILURE);
                }

                //Hijo
                if (pid[i] == 0) {
                    //Manejo de los diferentes comandos y cerrar pipes//

                    //Primer y unico comando
                    if ((i == 0) && (i == line->ncommands - 1))
                        ejecutar_comando(i);

                    //Primer comando
                    if ((i == 0) && (i != line->ncommands - 1)) {
                        //Cerrar lectura y escribir en tuberia i
                        close(pipe_fd[i][0]);
                        dup2(pipe_fd[i][1], 1);
                        //Ejecutar el primer comando y redirigir su salida
                        ejecutar_comando(i);
                    }

                    //Comando en medio
                    if ((i > 0) && (i != line->ncommands - 1)) {
                        //Cerrar escritura y leer tuberia i-1
                        close(pipe_fd[i - 1][1]);
                        dup2(pipe_fd[i - 1][0], 0);

                        //Cerrar lectura y escribir salida en tuberia i
                        close(pipe_fd[i][0]);
                        dup2(pipe_fd[i][1], 1);

                        ejecutar_comando(i);
                    }

                    //Ultimo comando
                    if ((i > 0) && (i == line->ncommands - 1)) {
                        //Cerrar escritura tuberia y leer
                        close(pipe_fd[i - 1][1]);
                        dup2(pipe_fd[i - 1][0], 0);

                        ejecutar_comando(line->ncommands - 1);
                    }

                //Padre
                } else {
                    if (i < line->ncommands - 1)
                        close(pipe_fd[i][1]);
                }
            }
        }

        // Esperar que terminen todos los hijos, 0 significa esperar a cualquier proceso hijo
        for (i = 0; i < line->ncommands; i++)
            waitpid(pid[i], &status, 0);

        //Liberamos la memoria reservada antes
        if (line->ncommands > 1) {
            for (i = 0; i < line->ncommands - 1; i++)
                free(pipe_fd[i]);
            free(pipe_fd);
        }

        //Se restauran las entradas y salidas estandar originales
        restaurarES();

        printf("\nmsh> ");
    }
    return 0;
}

    //FUNCIONES//

//Ejecutar un comando
void ejecutar_comando(int i) {
    //Ejecutar el comando correspondiente si el comando es válido
    if (line->commands[i].filename != NULL)
        execvp(line->commands[i].filename, line->commands[i].argv);
    else {
        fprintf(stderr, "%s: No se encuentra el mandato\n", line->commands[i].argv[0]);
        return;
    }
}

//Redirecciones
int redireccion_entrada(char * entrada) {
    //Se abre el fichero de entrada en modo lectura
    fEntrada = fopen(entrada, "r");
    //Se redirecciona la entrada estandar al fichero
    if (fEntrada != NULL)
        dup2(fileno(fEntrada), STDIN_FILENO);
    else {
        fprintf(stderr, "%s: Error. No se pudo encontrar el fichero de entrada\n", entrada);
        return 1;
    }
    //Se cierra el fichero y se sale de la funcion
    fclose(fEntrada);
    return 0;
}

int redireccion_salida(char * salida) {
    //Se abre el fichero de salida en modo escritura
    fSalida = fopen(salida, "w");
    //Se redirecciona la salida estandar al fichero
    if (fSalida != NULL)
        dup2(fileno(fSalida), STDOUT_FILENO);
    else {
        fprintf(stderr, "%s: Error. No se pudo escribir en el fichero de salida\n", salida);
        return 1;
    }
    //Se cierra el fichero y se sale de la funcion
    fclose(fSalida);
    return 0;
}

int redireccion_error(char * error) {
    //Se abre el fichero de salida en modo escritura
    fError = fopen(error, "w");
    //Se redirecciona la salida estandar al fichero
    if (fError != NULL)
        dup2(fileno(fError), STDERR_FILENO);
    else {
        fprintf(stderr, "%s: Error. No se pudo escribir en el fichero de error\n", error);
        return 1;
    }
    //Se cierra el fichero y se sale de la funcion
    fclose(fError);
    return 0;
}

//Funcion que sirve para restaurar las entradas y salidas originales para la siguiente línea de mandatos
void restaurarES() {
    if (line->redirect_input != NULL)
        dup2(saved_input, STDIN_FILENO);

    if (line->redirect_output != NULL)
        dup2(saved_output, STDOUT_FILENO);

    if (line->redirect_error != NULL)
        dup2(saved_error, STDERR_FILENO);
}

    //Funciones comandos//

//Funcion cd
void cd() {
    char *dir; // Variable de directorios
    char buffer[512];

    if (line->commands[0].argc > 2)
        fprintf(stderr,"Uso: %s directorio\n", line->commands[0].argv[0]);

    if (line->commands[0].argc == 1) {
        dir = getenv("HOME");
        if (dir == NULL)
            fprintf(stderr,"No existe la variable $HOME\n");
    } else
        dir = line->commands[0].argv[1];

    // Comprobar si es un directorio.
    if (chdir(dir) != 0)  // Si no es distinto de 0 lo hace normal el chdir
        fprintf(stderr,"Error al cambiar de directorio: %s\n", strerror(errno));
    else
        printf( "El directorio actual es: %s\n", getcwd(buffer,-1));
}

// comprobacion de errores (num de umask correcto)
bool comprobar_n(char *n) {
    int aux;
    if (strlen(n) > 4)     // num umask con menos de 4 cifras
        return false;

    for (int i = (int)strlen(n) - 1; i >= 0; i--) {
        aux = n[i] - 48; // transformar de char a int
        if ((aux > 7) || (aux < 0) )  // numeros < 8
            return false;
    }
    return true;
}

//umask
void Umask(tcommand *command, mode_t *mask) {
    int tempMask = *mask;
    int digitCount = 4;
    int octal;

    // Si se55 pasa sin parámetros, se imprime la máscara actual
    if (command->argc == 1) {
        // Si la máscara sea 0, se decrementa el contador
        if (tempMask == 0)
            digitCount--;

        // Bucle para calcular cuántos ceros se deben imprimir antes de la máscara
        while (tempMask > 0) {
            tempMask /= 10;
            digitCount--;
        }

        // Imprime la cantidad necesaria de ceros
        for(int i = 0; i < digitCount; i++)
            printf("0");

        // Imprime el valor de la máscara
        printf("%i \n", *mask);
    }
    // Si el comando se pasa con un parámetro, se intenta cambiar la máscara
    else {
        // Comprueba si el parámetro proporcionado es un número octal válido
        if (comprobar_n(command->argv[1])) {
            // Si es válido, se cambia la máscara al valor del parámetro
            *mask = atoi(command->argv[1]);
            // Se convierte el valor del parámetro a octal
            sscanf(command->argv[1], "%o", &octal);
            // Se establece la nueva máscara
            umask(octal);
        }
        // Si el parámetro proporcionado no es un número octal válido, se muestra un mensaje de error
        else {
            fprintf(stderr, "%s: no es válido. Debe ser un número octal de 4 cifras o menos\n", command->argv[1]);
            return;
        }
    }
}


