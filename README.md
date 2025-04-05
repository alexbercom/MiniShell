# 🐚 MiniShell en C
## 📌 Descripción
Este proyecto es una implementación en C de un shell básico llamado `MiniShell`, que permite la ejecución de comandos de forma secuencial o en pipeline, incluyendo soporte para redirección de entrada, salida y error, así como ejecución en segundo plano (`&`) y manejo de señales.

El objetivo de este MiniShell es replicar algunas de las funcionalidades más comunes de Bash de forma simplificada, implementando internamente comandos como `cd`, `exit`, y `umask`.

Entre las funcionalidades principales se incluyen:
- Ejecución de múltiples comandos mediante pipes (`|`)
- Redirección de entrada (`<`), salida (`>`) y error (`>&`)
- Ejecución de comandos en segundo plano con `&`
- Comandos internos: `cd`, `exit` y `umask`
- Ignorar la señal `SIGINT` (Ctrl+C) en el shell principal

## 🛠️ Tecnologías utilizadas
- Lenguaje C
- Librería de parsing (`parser.h` y `libparser.a`)
- Llamadas al sistema de Unix (`fork`, `exec`, `pipe`, `dup2`, `waitpid`, `chdir`, `umask`, `signal`, etc.)
- Gestión de procesos y redirección de E/S

## 🏗️ Estructura del Proyecto

- **myshell.c**: Archivo principal que contiene la lógica del shell.
  - Lógica principal del bucle de lectura y ejecución de comandos.
  - Funciones auxiliares para redirecciones y ejecución de comandos.
  - Implementación de los comandos internos `cd`, `umask`, `exit`.
  - Manejo de tuberías para múltiples comandos.
  - Gestión de memoria y restauración de entradas/salidas estándar.

### Funciones destacadas

- `main`: Bucle principal del shell (`msh>`), lee líneas, analiza comandos y lanza procesos.
- `ejecutar_comando(i)`: Ejecuta un comando individual según su posición en el pipeline.
- `cd()`: Cambia el directorio actual.
- `Umask(command, mask)`: Muestra o cambia la umask del sistema.
- `restaurarES()`: Restaura la entrada/salida/error estándar original tras redirecciones.
- `redireccion_entrada/salida/error()`: Redirige los flujos estándar a ficheros especificados.

## ▶️ Compilación y ejecución

### 🔧 Requisitos
- Sistema Linux con compilador GCC
- Archivos `parser.h` y `libparser.a` disponibles en el directorio de trabajo

### 📦 Compilación
```bash
gcc myshell.c libparser.a -o myshell
```

## 🧪 Casos de Prueba
- ✅ Ejecutar un solo comando (ls, pwd, etc.)
- ✅ Redirecciones de entrada y salida (< input.txt, > output.txt)
- ✅ Comandos en pipeline (ls | grep .c)
- ✅ Comandos internos (cd, umask, exit)
- ✅ Comandos en background (sleep 10 &)
- ✅ Manejo de errores (comandos inválidos, ficheros no existentes)
- ✅ Ignorar Ctrl+C en la shell

## ⚙️ Funcionamiento del Programa
1. Inicio del Shell: Muestra msh> como prompt.

2. Lectura y parsing: Usa tokenize() de la librería parser para descomponer la línea.

3. Redirecciones: Configura stdin, stdout y stderr según los operadores <, >, >&.

4. Ejecutar comandos:
   - Si son comandos internos (cd, umask, exit), se manejan directamente.
   - Si son comandos externos, se crea un proceso hijo y se lanza mediante execvp.
   - Para pipelines, se crean las tuberías correspondientes.

5. Restauración: Se restauran las E/S estándar tras ejecutar los comandos.

6. Loop: Se vuelve a mostrar el prompt.

## 👨‍💻 Autor
Alex Bermejo Compán
