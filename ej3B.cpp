/*
*----------------------------------------
* ej3B.cpp
* ---------------------------------------
* UNIVERSIDAD DEL VALLE DE GUATEMALA
* CC3086 - Programacion de Microprocesadores
* Autor: Kimberly Barrera
* Modificado por: Dulce Ambrosio - 231143
* Modificado por: Javier Linares - 231135
* ---------------------------------------
* El código original fue proporcionado por
* la catedrática Kimberly Barrera, fue 
* modificado para que finalice de la manera 
* correcta, es decir que los productores no
* sigan generando piezas de sillas de manera
* indefinida, también se le agrego un nuevo 
* método para mostrar un reporte que indique 
* cuántas sillas se fabricaron en totalidad, 
* y cuántas piezas de cada tipo sobraron. 
*----------------------------------------
*/

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;


const char* productos[] = {"Pata", "Respaldo", "Asiento", "Pata", "Pata"};//Piezas de las sillas
const int numProductos = 5;  // Número total de tipos de piezas de las sillas
const int MAX_BUFFER = 5;    // Capacidad máxima para almacenar las piezas de la sillas
const int MAX_SILLAS = 3;    // Número máximo de sillas a producir

int buffer[MAX_BUFFER];       // Buffer para almacenar temporalmente las piezas
int in = 0;                   // Contador para agregar piezas al buffer
int out = 0;                  // Contador para retirar piezas del buffer
int sillasProducidas = 0;     // Contador de sillas producidas
bool produccionActiva = true; // Para controlar la producción de las sillas
int piezasSobrantes[numProductos] = {0}; // Contador de piezas sobrantes en el buffer

// Semáforos y mutex 
sem_t vacios;   // Semáforo que controla los espacios vacíos en el buffer
sem_t llenos;   // Semáforo que controla las piezas disponibles en el buffer
pthread_mutex_t mutex;  // Mutex para proteger las secciones críticas

// Función que simula la producción de una pieza de silla
void* productor(void* arg) {
    int id = *(int*)arg;  // ID del productor
    int piezaId;
    
    while (true) {
        // Si la producción ha terminado, el productor se detiene
        pthread_mutex_lock(&mutex);
        if (!produccionActiva) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        // Elige una pieza al azar para fabricar
        piezaId = rand() % numProductos;  

        // Espera hasta que haya espacio disponible en el buffer
        sem_wait(&vacios);
        pthread_mutex_lock(&mutex);  // Bloquea el acceso al buffer para agregar una pieza

        // Verifica nuevamente si la producción sigue activa
        if (!produccionActiva) {
            pthread_mutex_unlock(&mutex);
            sem_post(&vacios);
            break;
        }

        // Agrega la pieza fabricada al buffer
        buffer[in] = piezaId;
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER;  

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);  // Indica que hay una nueva pieza disponible

        sleep(1);  // Simula el tiempo de fabricación
    }
    
    return NULL;
}

// Función del consumidor para ensamblar las sillas
void* consumidor(void* arg) {
    int id = *(int*)arg;  // ID del consumidor
    int piezaId;

    while (true) {
        // Espera hasta que haya piezas disponibles en el buffer
        sem_wait(&llenos);
        pthread_mutex_lock(&mutex);  // Bloquea el acceso al buffer para retirar una pieza

        // Si ya se ensamblaron todas las sillas, el consumidor se detiene
        if (sillasProducidas >= MAX_SILLAS) {
            pthread_mutex_unlock(&mutex);
            sem_post(&llenos);
            break;
        }

        // Retira una pieza del buffer
        piezaId = buffer[out];
        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER;  

        // Si se completa una silla se suma al contador
        if (piezaId == numProductos - 1) {
            sillasProducidas++;
            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;

            // Si se alcanzó el número máximo de sillas, se detiene la producción
            if (sillasProducidas >= MAX_SILLAS) {
                produccionActiva = false;
            }
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);  // Indica que hay un espacio vacío en el buffer

        sleep(2);  // Simula el tiempo de ensamblaje
    }

    return NULL;
}

// Función para generar el reporte 
void generarReporte() {
    // Lleva el control de las piezas sobrantes en el buffer
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_BUFFER; i++) {
        if (buffer[i] >= 0 && buffer[i] < numProductos) {
            piezasSobrantes[buffer[i]]++;
        }
    }
    pthread_mutex_unlock(&mutex);

    // Imprimer el reporte
    cout << "\n--- Reporte Final ---\n";
    cout << "Sillas fabricadas: " << sillasProducidas << endl;
    cout << "Piezas sobrantes en el buffer:\n";
    for (int i = 0; i < numProductos; i++) {
        cout << productos[i] << ": " << piezasSobrantes[i] << " sobrantes" << endl;
    }
    cout << "---------------------\n";
}

int main() {
    int numProductores, numConsumidores;

    // Solicita la cantidad de productores y consumidores al usuario
    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];  // Arreglos de hilos para productores y consumidores
    int idProductores[100], idConsumidores[100];    // IDs de los productores y consumidores

    // Inicializa los semáforos y el mutex
    sem_init(&vacios, 0, MAX_BUFFER);  // Inicializa el semáforo de vacíos
    sem_init(&llenos, 0, 0);           // Inicializa el semáforo de llenos
    pthread_mutex_init(&mutex, NULL);  // Inicializa el mutex

    // Crea los hilos para los productores
    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    // Crea los hilos para los consumidores
    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    // Espera a que todos los hilos productores terminen
    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }

    // Espera a que todos los hilos consumidores terminen
    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    // Genera el reporte 
    generarReporte();

    // Destruye los semáforos y el mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}