/*
* Julián Muñoz M 
* SO y lab 2018 -1
* UdeA
* 
* Para ejecutarlo de forma global, copie el archivo psinfo a la carpeta
* ~/bin y asegurase que esta se encuentra en el $PATH con cat $PATH.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>

/* Se define el tipo de datos booleano */
typedef int bool;
#define true 1
#define false 0

// El maximo número de caracteres para un nombre de archivo.
#define FILENAME_MAX 4096
// Se puede configurar en algunas máquinas hasta valores 2^22, este número es de 7
// cifras.
#define PID_MAX 7

void usage(char *filename);
int searchInFile(char *filename, char *s);
char ** get_status(char *pid);
char * trim(int left_offset, char *line);

int main (int argc, char *argv[]){

    // int result;
    if(argc < 2){
        // para remover el ./
        usage(argv[0]+2);
        exit(1);
    }

    bool list = false;
    bool save = false;

    printf("\n");
    /**/
    // i: primer pid
    int i;
    if(argc == 2){
        list = false;
        i = 1;
    } else if(strncmp(argv[1], "-l", 2) == 0 && strncmp(argv[2], "-r", 2) != 0){
        list = true;
        save = false;
        i = 2;
    } else if(strncmp(argv[1], "-l", 2) == 0 && strncmp(argv[2], "-r", 2) == 0){
        list = true;
        save = true;
        i = 3;
    } else  if(strncmp(argv[1], "-r", 2) == 0 && strncmp(argv[2], "-l", 2) != 0 && argc <= 3){
        save = true;
        list = false;
        i = 2;
    }
    else if(strncmp(argv[1], "-r", 2) == 0 && strncmp(argv[2], "-l", 2) == 0){
        save = true;
        list = true;
        i = 3;
    } else{
        usage(argv[0]+2);
        exit(1);
    }
    /**/

    FILE *report;
    char filename[FILENAME_MAX];

    if(save){
        char item[PID_MAX+1] = "-";
        // cat /tmp/psinfo-report-[PID's].txt
        // strncpy(filename, "/home/julio/psinfo-report", FILENAME_MAX);
        strncpy(filename, "/tmp/psinfo-report", FILENAME_MAX);
        for(int j = i; j < argc; j++){
            strncat(item, argv[j], PID_MAX+1);
            strncat(filename, item, FILENAME_MAX);
            snprintf(item, PID_MAX, "-");
        }
        strncat(filename, ".txt", FILENAME_MAX);
        report = fopen(filename, "w");
        if(report == NULL) {
            printf("No se pudo guardar el archivo.\n");
            return 1;
        }
    }

    /* Si se tiene la opción -r se guarda la salida en el archivo destino,
     * mientras se imprime la info en consola.
     */
    if(list){
        for(int j = i; j < argc; j++){
            char** psinfo = get_status(argv[j]);
            if(psinfo == NULL) {
                printf("PID: %s, no válido\n", argv[j]);
                return 1;
            }
            for(int k = 0; k < 11; k++){
                printf("%s", psinfo[k]);
                if(save){
                    fprintf(report, "%s", psinfo[k]);
                } //se libera cada bloque de memoria k
                free(psinfo[k]);
            } //se libera el bloque de memoria asignado a psinfo
            free(psinfo);
        }
    } else{
        char** psinfo = get_status(argv[i]);
        if(psinfo == NULL) {
                printf("PID: %s, no válido\n", argv[i]);
                return 1;
        }
        for(int k = 0; k < 11; k++){
            printf("%s", psinfo[k]);
            if(save){
                    fprintf(report, "%s", psinfo[k]);
            }
            free(psinfo[k]);
        }
        free(psinfo);
    }
    if(save) { 
            fclose(report);
            printf("\nSe ha guardado el reporte en la ruta: ");
            printf("\n%s\n\n", filename);
    }
    return 0;
}

void usage(char *app){
    printf("Uso:\t%s [PID]\n", app);
    printf("\t%s -l [PID's]\n", app);
    printf("\t%s -l -r [PID's]\n", app);
    printf("\t%s -r [PID]\n", app);
    printf("\t%s -r - l [PID's]\n", app);
}

// Función parser
char ** get_status(char *pid){
    // Crea un array de strings de 20 posiciones
    char** psinfo = malloc(50 * sizeof(char*));
    // Asigna a cada string un region de memoria de 50 caracteres cada una.
    for(int i = 0; i < 20; i++) 
        psinfo[i] = malloc(50 * sizeof(char));
    
    // Para moverse a la siguiente linea de la información del proceso
    int j = 0;
    /* snprintf crea un string con un buffer de n=50 en este caso, usando el tercer
     * y el cuarto argumento, como en la función printf, finalmente almacena el resultado en el primer parametro
     * en este caso es la variable psinfo[0].
     * 
     * j++ aumenta a j en 1 despues de realizar la operación, ++j lo hace antes de la
     * operación.
     */
    snprintf(psinfo[j++], 50, "********* PID del proceso: %s *********\n", pid);
    int offset = 0;

    char path[40], line[100];
    // nombre del atributo, por ejemplo Nombre
    char *attr_name;
    // Esta cadena es el value del atributo.
    char *value;
    FILE *statusf;

    // Crea la ruta para el proceso
    snprintf(path, 40, "/proc/%s/status", pid);

    snprintf(psinfo[j++], 50, "Ruta:\t\t/proc/%s/status\n", pid);
    
    // La opción r es para leer el archivo psinfo en el archivo psinfo del proceso,
    // el contenido de este se guarda en la variable statusf, de "psinfo file"
    statusf = fopen(path, "r");
    if(!statusf)
        return NULL;
    
    /* Mientras statusf tenga lineas, aqui se hacen 2 operaciones, se extrae una linea
     * de statusf y se almacena en la variable line y la vez se comprueba si se ha 
     * terminado de leer el archivo.
     * 
     * La función fgets es la alternativa segura a gets
     */
    while(fgets(line, 100, statusf)){
        // La función strncmp, compara 2 strings y devuelve 0 si son iguales.
        if(strncmp(line, "Name:", 5) == 0)
        {   // Sirve para saltarse los caracteres Name: y obtener el value de la linea
            // Ejemplo 1: si la linea es "Name: gedit", se obtiene "gedit".
            offset = 6;
            attr_name = "Nombre: \t";
            value = trim(offset, line);
        }
        if(strncmp(line, "State:", 6) == 0)
        {
            offset = 7;
            attr_name = "Estado: \t";
            value = trim(offset, line);
        }
        if(strncmp(line, "VmData:", 7) == 0){
            offset = 8;
            snprintf(psinfo[j++], 50, "Tamaño de las regiones de memoria:\n");
            attr_name = "\tDATA: \t\t";
            value = trim(offset, line);
        }
        if(strncmp(line, "VmStk:", 6) == 0){
            offset = 7;
            attr_name = "\tSTACK: \t\t";
            value = trim(offset, line);
        } 
        if(strncmp(line, "VmExe:", 6) == 0){
            offset = 7;
            attr_name = "\tTEXT: \t\t";
            value = trim(offset, line);
        }
        if(strncmp(line, "voluntary_ctxt_switches:", 24) == 0){
            offset = 25;
            snprintf(psinfo[j++], 50, "Número de cambios de contexto realizados:\n");
            attr_name = "\tVoluntarios: \t";
            value = trim(offset, line);
        }
        if(strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0){
            offset = 28;
            attr_name = "\tNo voluntarios: ";
            value = trim(offset, line);
        }
        // Si offset = 0 significa que ya la linea no contiene la info deseada.
        if(offset == 0)
            continue;
        
        snprintf(psinfo[j++], 50, "%s%s", attr_name, value);
        offset = 0;
    }

    fclose(statusf);
    return psinfo;
}

char * trim(int left_offset, char *line){
    // Este puntero nos ayudará a remover el espacio en blanco.
    char *trimmed_line;
    // Ignorar la leyenda, por ejemplo Name:   gedit, se borra Name:
    trimmed_line = line + left_offset;
    // Ignorar espacio en blanco
    // Del ejemplo 1, se puede obtener trimmed_line = "  gedit", la siguiente instrucción
    // devuelve trimmed_line = "gedit"
    // *trimmed_line caracter a la izquierda de trimmed_line, ++trimmed_line remueve un caracter a la izquierda de trimmed_line.
    // de trimmed_line.
    while(isspace(*trimmed_line)) ++trimmed_line;
    return trimmed_line;
}
