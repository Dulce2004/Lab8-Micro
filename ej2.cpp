/*
*----------------------------------------
* eje2.cpp
* ---------------------------------------
* UNIVERSIDAD DEL VALLE DE GUATEMALA
* CC3086 - Programación de Microprocesadores
* Autores:
* Dulce Ambrosio - 231143
* Javier Linares - 231135
* ---------------------------------------
* Este programa simula el acceso concurrente de varios clientes a un cajero 
* automático (ATM). Para garantizar que solo un cliente acceda al cajero a 
* la vez, se utiliza un semáforo. Cada cliente intenta retirar una cantidad 
* específica de dinero, y el saldo disponible se actualiza en consecuencia. 
* Si el saldo es insuficiente, el cliente no podrá retirar la cantidad 
* solicitada.
* ----------------------------------------
*/

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // Para la función sleep()

using namespace std;

// Variables globales
double saldo = 100000.00; // Saldo inicial de la cuenta compartida
sem_t semaforo; // Semáforo para controlar el acceso concurrente al cajero

// Estructura que contiene los datos de cada cliente
struct Cliente {
    int id;            // ID del cliente
    double montoRetiro; // Monto que el cliente desea retirar
};

// Función que simula el retiro de dinero por parte de un cliente
void* realizarRetiro(void* arg) {
    // Se recibe la estructura Cliente como argumento
    Cliente* cliente = (Cliente*) arg;

    // Se espera a que el semáforo permita el acceso (1 cliente a la vez)
    sem_wait(&semaforo);

    // Mostrar el ID del cliente y el monto que desea retirar
    cout << "Cliente " << cliente->id << " retirando Q " << cliente->montoRetiro << endl;

    // Verificar si hay saldo suficiente para realizar el retiro
    if (saldo >= cliente->montoRetiro) {
        // Si hay suficiente saldo, se realiza el retiro y se actualiza el saldo
        saldo -= cliente->montoRetiro;
        cout << "Cliente " << cliente->id << " ha retirado Q " << cliente->montoRetiro 
             << ". Saldo restante: Q " << saldo << endl;
    } else {
        // Si no hay suficiente saldo, se notifica al cliente
        cout << "Cliente " << cliente->id << " no puede retirar. Saldo insuficiente." << endl;
    }

    // Liberar el semáforo para que otro cliente pueda acceder
    sem_post(&semaforo);
    pthread_exit(NULL); // Finalizar el hilo

    return NULL; // Evitar warnings al compilar
}

int main() {
    int numClientes; // Variable para almacenar la cantidad de clientes

    // Solicitar al usuario el número de clientes que participarán en la simulación
    cout << "Ingrese el numero de clientes: ";
    cin >> numClientes;

    // Arreglo de hilos y datos para cada cliente
    pthread_t clientes[numClientes]; // Arreglo para los hilos de los clientes
    Cliente clienteDatos[numClientes]; // Arreglo para los datos de cada cliente

    // Inicializar el semáforo, permitiendo 1 acceso a la vez
    sem_init(&semaforo, 0, 1);

    // Solicitar el monto de retiro para cada cliente
    for (int i = 0; i < numClientes; ++i) {
        cout << "Ingrese el monto de retiro para el cliente " << i + 1 << ": Q ";
        cin >> clienteDatos[i].montoRetiro; // Almacenar el monto de retiro
        clienteDatos[i].id = i + 1; // Asignar un ID al cliente
    }

    // Crear un hilo para cada cliente
    for (int i = 0; i < numClientes; ++i) {
        pthread_create(&clientes[i], NULL, realizarRetiro, (void*)&clienteDatos[i]);
    }

    // Esperar a que todos los hilos terminen su ejecución
    for (int i = 0; i < numClientes; ++i) {
        pthread_join(clientes[i], NULL); // Unir los hilos al hilo principal
    }

    // Destruir el semáforo al final del programa
    sem_destroy(&semaforo);

    return 0; // Indicar que el programa terminó correctamente
}