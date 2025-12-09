/*
PROYECTO FINAL SISTEMAS OPERATIVOS: IMPLEMENTACION DE BUDDY SYSTEM Y LAZY BUDDY SYSTEM

INTEGRANTES:
- José Gibran Alonso Bocanegra
- Valeria Marín de Santos
- Jessica Vanessa Martínez de la Rosa
- Danna Sofía Morales Esparza 
*/

#include <iostream>
#include <cmath>
#include <vector>
#include <queue> 
#include <time.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h> // Para Sleep

using namespace std;

// Velocidades minimas y maximas en milisegundos 
const int velMax = 50;
const int velMin = 500;

// ---------------------- CLASE DE PROCESOS ----------------------
class Procesos {
private:
    int id;
    int tam;
    int cuanto;
    int cuantoOriginal;

public:
    Procesos(int i, int t, int c) : id(i), tam(t), cuanto(c), cuantoOriginal(c) {}
    int getId() { return id; }
    int getTam() { return tam; }
    int getCuanto() { return cuanto; }
    int getCuantoOriginal() { return cuantoOriginal; }
    void disminuirCuanto() { cuanto--; }
    bool terminado() { return cuanto <= 0; }

    // Nuevo: actualizar cuanto (se usa para aplicar cuantoSistema y mantener estado)
    void setCuanto(int c) { cuanto = c; }
};

// ---------------------- VARIABLES GLOBALES ----------------------
int totProcesos; 
const int velocidad = 1;           // Velocidad de la simulación
float velActual;
float velAnterior;
int cuantoSistema ; 
int tamMaxMemoProces;    // Tamaño máximo de memoria de proceso
int tamMaxCuanto = 1;    // Tamaño máximo del cuanto de procesamiento (cuanto del sistema)
int tamMemoria;          // Tamaño total de memoria
int idProceso = 1;       // Contador de IDs de procesos

// Estadísticas finales 
int procesosAtendidos = 0;
int procesosGenerados = 0;

// ---------------------- ESTRUCTURA DE BLOQUE ----------------------
struct Bloque {
    int id;       // ID del proceso, 0 si está libre
    int tam;      // Tamaño del bloque
    bool libre;   // Estado del bloque
    int cuantoRestante; // Cuanto restante del proceso
    int tamProceso;
};

vector<Bloque> memoria; // Representación de la memoria
queue<Procesos> colaEspera; // Cola de procesos en espera

// ---------------------- PROTOTIPOS ----------------------
void mostrarMenuPrincipal();
void configurarParametros();
Procesos generarProceso();
void mostrarProceso(Procesos p);
void mostrarProceso(Procesos p, int memoriaAsignada);
void mostrarMemoria();
//void mostrarEstadisticasDuranteEjecucion();
bool detenerSimulacion();
//void control_velocidad();

bool asignarBuddy(Procesos p);
int calcularMemoriaOcupada();
void procesarCiclo();

// Implementación de Buddy System
void implementar_Buddy();
void fusionarBuddy();

// Implementación de Lazy Buddy System
void implementar_Lazy();
void fusionarLazyBuddy();

// Función auxiliar para buddy system
bool esPotenciaDeDos(int n);

// ---------------------- MAIN ----------------------
int main() {
    srand(time(NULL));
    mostrarMenuPrincipal();
    return 0;
}

// ---------------------- IMPLEMENTACIONES ----------------------

void mostrarMenuPrincipal() {
    int opcion;

    do {
        cout << "\n-------BIENVENIDO A LA SIMULACION DE GESTION DE MEMORIA-------";
        cout << "\nQue metodo desea implementar?";
        cout << "\n0- Salir";
        cout << "\n1- Buddy system";
        cout << "\n2- Lazy buddy system";
        cout << "\nElige una opcion: ";
        cin >> opcion;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            opcion = 500;
        }

        if (opcion != 0) configurarParametros();

        switch (opcion) {
        case 1: implementar_Buddy(); break;
        case 2: implementar_Lazy(); break;
        case 0: cout << "Saliendo..."; break;
        default: cout << "Opcion invalida\n";
        }

        cout << endl;
    } while (opcion != 0);
}

// ---------------------------------------------------------------

void configurarParametros() {
    int maxPermitido;
    do {
        cout << "Tamanio total de memoria (1, 4 o 8 MB): ";
        cin >> tamMemoria;
    } while (tamMemoria != 1 && tamMemoria != 4 && tamMemoria != 8);

    tamMemoria *= 1024; // convertir MB a KB
    cuantoSistema = 3; // Cuanto del sistema fijo en 3 
    tamMaxCuanto = 10; // Cuanto maximo por proceso fijo en 10

    if (tamMemoria == 1024) { // 1 MB
        maxPermitido = 64;  // Tamaño máximo será de 64 KB
    } else if (tamMemoria == 4096) { // 4 MB
        maxPermitido = 128; // Tamaño máximo será de 128 KB
    } else if (tamMemoria == 8192) { // 8 MB
        maxPermitido = 256; // Tamaño máximo será de 256 KB
    }
    do {
        cout << "Tamanio maximo de memoria por proceso ( Minimo 32 KB, maximo "<< maxPermitido << " KB): ";
        cin >> tamMaxMemoProces;
    } while (tamMaxMemoProces < 32 || tamMaxMemoProces > tamMemoria || tamMaxMemoProces > maxPermitido);

    
    cout << "\n\n--- CONFIGURACION APLICADA ---";
    cout << "\nTamanio total de memoria: " << tamMemoria/1024 << " MB (" << tamMemoria << " KB)";
    cout << "\nTamanio maximo por proceso: " << tamMaxMemoProces << " KB";
    cout << "\nCuanto del sistema: " << cuantoSistema;
    cout << "\nCuanto maximo por proceso: " << tamMaxCuanto;
    cout << "\n--------------------------------\n";

    // Inicializamos la memoria y estadísticas
    memoria.clear();
    memoria.push_back({ 0, tamMemoria, true, 0, 0 });
    while (!colaEspera.empty()) colaEspera.pop();
    procesosAtendidos = 0;
    procesosGenerados = 0;
    idProceso = 1;
    velAnterior = 0;

    cout << "\nDurante la simulacion puedes:"
        << "\n1- AUMENTAR LA VELOCIDAD presionando la tecla de flecha hacia arriba;"
        << "\n2- DISMINUIR LA VELOCIDAD presionando la tecla de flecha hacia abajo;"
        << "\n3- TERMINAR LA SIMULACION presionando la tecla Esc.\n\n";
    system("pause");
}

// ---------------------------------------------------------------
// Muestra el contenido actual de la memoria
void mostrarMemoria() {
    for (auto& b : memoria) {
        cout << "[" << b.id << ",";
        if(!b.libre) cout << b.tamProceso;
        else cout << 0;
        cout << ",";
        if(!b.libre) cout << "(";
        cout << b.tam;
        if(!b.libre) cout << ")";
        cout << "," << b.cuantoRestante << "]";
    }
}

// ---------------------------------------------------------------
// Genera procesos aleatorios
Procesos generarProceso() {

    int tam = (rand() % (tamMaxMemoProces - 32 + 1)) + 32; // mínimo 32KB
    int cuanto = (rand() % (tamMaxCuanto - 3 + 1)) + 3;
    Procesos p(idProceso++, tam, cuanto);
    procesosGenerados++; 
    return p;
}

// ---------------------------------------------------------------
// Función auxiliar para verificar si un número es potencia de 2
bool esPotenciaDeDos(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

// ---------------------------------------------------------------
// Fusiona bloques libres adyacentes del mismo tamaño que sean potencia de 2
void fusionarBuddy() {
    bool fusionado = true;
    bool indicador = false; // Para saber si hubo por lo menos una fusión
    
    while (fusionado) {
        fusionado = false;
        
        for (size_t i = 0; i < memoria.size() - 1; i++) {
            // Fusionar bloques adyacentes libres del mismo tamaño que sean potencia de 2
            if (memoria[i].libre && memoria[i + 1].libre && 
                memoria[i].tam == memoria[i + 1].tam && 
                esPotenciaDeDos(memoria[i].tam)) {
                
                // Verificar que el tamaño fusionado no exceda la memoria total
                int tamFusionado = memoria[i].tam * 2;
                if (tamFusionado <= tamMemoria) {
                    memoria[i].tam = tamFusionado;
                    memoria.erase(memoria.begin() + i + 1);
                    fusionado = true;
                    indicador = true;
                    break; // Reiniciar el bucle después de cada fusión

                }
            }
        }
    }

    if (indicador) cout << "\nFusion de bloques exitosa";
    else cout << "\nNo habia bloques para fusionar";
}

// Fusionar el par de bloques contiguos más pequeño     
void fusionarLazyBuddy(){
    int indiceMenor = -1;
    int tamMenor = tamMemoria + 1; // Inicializar con valor mayor al posible
    
    // Buscar el par de bloques contiguos más pequeños
    for (size_t i = 0; i < memoria.size() - 1; i++) {
        if (memoria[i].libre && memoria[i + 1].libre && 
            memoria[i].tam == memoria[i + 1].tam && 
            esPotenciaDeDos(memoria[i].tam)) {
            
            if (memoria[i].tam < tamMenor) {
                tamMenor = memoria[i].tam;
                indiceMenor = i;
            }
        }
    }
    
    // Fusionar el par más pequeño encontrado
    if (indiceMenor != -1) {
        int tamFusionado = memoria[indiceMenor].tam * 2;
        if (tamFusionado <= tamMemoria) {
            memoria[indiceMenor].tam = tamFusionado;
            memoria.erase(memoria.begin() + indiceMenor + 1);
        }
        cout << "\nFusion de bloques exitosa";
    }
    // Si no hay bloques para fusionar 
    else cout << "\nNo habia bloques para fusionar";
}   


// ---------------------------------------------------------------
// Inserta proceso en el primer bloque libre suficiente (Buddy)
bool asignarBuddy(Procesos p) {
    for (size_t i = 0; i < memoria.size(); i++) {
        if (memoria[i].libre && memoria[i].tam >= p.getTam()) {
            // Mientras el bloque sea mayor al doble del proceso, se divide
            while (memoria[i].tam / 2 >= p.getTam() && memoria[i].tam / 2 >= 32) {
                int nuevoTam = memoria[i].tam / 2;
                memoria[i].tam = nuevoTam;
                Bloque b2 = { 0, nuevoTam, true, 0, 0 };
                memoria.insert(memoria.begin() + i + 1, b2);
            }
            // Asignar proceso
            memoria[i].id = p.getId();
            memoria[i].libre = false;
            memoria[i].cuantoRestante = p.getCuanto();
            memoria[i].tamProceso = p.getTam();
            // Mostrar bloque de memoria con proceso asigando 
            cout << "\n\n- Se le asigno memoria al proceso: ";
            mostrarProceso(p, memoria[i].tam);
            return true;
        }
    }
    cout << "\n\n- El proceso nuevo esta en espera, no hay memoria para asignarle";
    return false;
}

// ---------------------------------------------------------------
// Calcula la memoria ocupada actualmente
int calcularMemoriaOcupada() {
    int ocupada = 0;
    for (auto& b : memoria) {
        if (!b.libre) {
            ocupada += b.tam;
        }
    }
    return ocupada;
}

// ---------------------------------------------------------------
// Simula ejecución de procesos con Buddy System 
void implementar_Buddy() {
    cout << "\n-- Simulacion Buddy System --\n";
    int ciclo = 0;

    // Reiniciar memoria para esta simulación
    memoria.clear();
    memoria.push_back({ 0, tamMemoria, true, 0, 0 });
    
    // Cola de procesos que YA ESTÁN EN MEMORIA esperando su turno
    queue<int> colaEjecucion; // Solo guardamos IDs de procesos

    while (!detenerSimulacion()) {
        ciclo++;
        cout << "\n\n=============================================\n";
        cout << "        CICLO " << ciclo << " BUDDY SYSTEM";
        cout << "\n=============================================\n\n";

        // Crear proceso si no hay otros en espera, solo mantener un proceso en espera 
        if (colaEspera.empty()) {
            Procesos p = generarProceso();

            cout << "- Nuevo proceso creado: "; 
            mostrarProceso(p);
            cout << "\n\n- Memoria antes de asigar proceso: ";
            mostrarMemoria();
            
            bool asignado = asignarBuddy(p); // Verificar que se le pueda asiganr memoria al proceso generado 
            
            // Poner en espera al proceso si no se le puede asignar memoria
            if (!asignado) {
                colaEspera.push(p); 
            }

            // Poner en la cola de ejecución si se le puede asignar memoria
            else{
                colaEjecucion.push(p.getId()); 
            }
            
        }
        else cout << "- No se genero ningun proceso nuevo en este ciclo porque ya hay uno en espera";

        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b: memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                    << b.tam << ")," << b.cuantoRestante <<  "]";    
                }
            }
        }

        // Ejecutar proceso frente a la cola de ejecucion 
        if (!colaEjecucion.empty()) {
            int idProcesoActual = colaEjecucion.front();
            colaEjecucion.pop();

            // Buscar bloque del proceso 
            int indiceBloque = -1;
            for (size_t i = 0; i < memoria.size(); i++) {
                if (memoria[i].id == idProcesoActual && !memoria[i].libre) {
                    indiceBloque = i;
                    break;
                }
            }

            if (indiceBloque != -1) {
                cout << "\n\n- Ejecutando proceso: ";
                cout << "[" << memoria[indiceBloque].id << "," << memoria[indiceBloque].tamProceso << ",(" 
                << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante <<  "]";

                // Ejecutar proceso (restarle el cuanto del sistema)
                memoria[indiceBloque].cuantoRestante -= cuantoSistema;
                if (memoria[indiceBloque].cuantoRestante < 0) memoria[indiceBloque].cuantoRestante = 0;

                // Verificar si el proceso terminó 
                if (memoria[indiceBloque].cuantoRestante <= 0) {
                    cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << "," 
                        << memoria[indiceBloque].tamProceso << ",(" 
                        << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante 
                        <<  "]"<< " termino su ejecucion";
                    // Liberar bloque de memoria que se le asignó si ya terminó 
                    memoria[indiceBloque].id = 0;
                    memoria[indiceBloque].libre = true;
                    memoria[indiceBloque].cuantoRestante = 0;
                    memoria[indiceBloque].tamProceso = 0;
                    procesosAtendidos++;

                    cout << "\n\n- Se libero el bloque de memoria ocupado por el proceso con ID [" << idProcesoActual << "]";

                    cout << "\n\n- Fusionando bloques";
                    // Fusionar bloques inmediatamente
                    fusionarBuddy();

                    // Verificar si el proceso en espera ya puede entrar a memoria (si es que hay uno en espera)
                    if (!colaEspera.empty()) {
                        Procesos e = colaEspera.front();
                        colaEspera.pop();
                        bool asignado = asignarBuddy(e);

                        // Volver a poner en espera si no aun no hay espacio en memoria disponible 
                        if (!asignado) {
                            colaEspera.push(e); 
                        }

                        // Poner en la cola de ejecución si se le puede asignar memoria
                        else {
                            colaEjecucion.push(e.getId()); 
                            cout << " que estaba en espera"; 
                        }
                    }
                }
                else { // reencolar proceso si no terminó
                    cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << "," 
                        << memoria[indiceBloque].tamProceso << ",(" 
                        << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante 
                        <<  "]"<< " NO termino su ejecucion"
                        << "\nReencolando proceso";
                    colaEjecucion.push(idProcesoActual);
                }
                
            }
        }
            
        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b: memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                    << b.tam << ")," << b.cuantoRestante <<  "]";    
                }
            }
        }

        Sleep(velocidad);
    }

    cout << "\n\nFin de la simulacion de BUDDY SYSTEM\n";

    /*cout << "\n\nEstadisticas finales de la simulacion  de Lazy Buddy System.\n";
    cout << "Procesos totales generados: " << procesosGenerados << endl;
    cout << "Procesos atendidos completamente: " << procesosAtendidos << endl;
    cout << "Procesos en cola de espera: " << colaEspera.size() << endl;
    system("pause");*/
}

// ---------------------------------------------------------------
// Lazy Buddy System 
void implementar_Lazy() {
    cout << "\n-- Simulacion Lazy Buddy System --\n";
    int ciclo = 0;

    // Reiniciar memoria para esta simulación
    memoria.clear();
    memoria.push_back({ 0, tamMemoria, true, 0, 0 });
    
    // Cola de procesos que YA ESTÁN EN MEMORIA esperando su turno
    queue<int> colaEjecucion; // Solo guardamos IDs de procesos

    while (!detenerSimulacion()) {
        ciclo++;
        cout << "\n\n================================================\n";
        cout << "        CICLO " << ciclo << " LAZY BUDDY SYSTEM";
        cout << "\n================================================\n\n";

        // Crear proceso si no hay otros en espera, solo mantener un proceso en espera 
        if (colaEspera.empty()) {
            Procesos p = generarProceso();

            cout << "- Nuevo proceso creado: "; 
            mostrarProceso(p);

            cout << "\n\n- Memoria antes de asigar proceso: ";
            mostrarMemoria();

            bool asignado = asignarBuddy(p); // Verificar que se le pueda asiganr memoria al proceso generado 
            
            // Poner en espera al proceso si no se le puede asignar memoria
            if (!asignado) {
                colaEspera.push(p); 
            }

            // Poner en la cola de ejecución si se le puede asignar memoria
            else{
                colaEjecucion.push(p.getId()); 
            }
        }
        else cout << "- No se genero ningun proceso nuevo en este ciclo porque ya hay uno en espera";

        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b: memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                    << b.tam << ")," << b.cuantoRestante <<  "]";    
                }
            }
        }

        // Ejecutar proceso frente a la cola de ejecucion 
        if (!colaEjecucion.empty()) {
            int idProcesoActual = colaEjecucion.front();
            colaEjecucion.pop();

            // Buscar bloque del proceso 
            int indiceBloque = -1;
            for (size_t i = 0; i < memoria.size(); i++) {
                if (memoria[i].id == idProcesoActual && !memoria[i].libre) {
                    indiceBloque = i;
                    break;
                }
            }

            if (indiceBloque != -1) {
                cout << "\n\n- Ejecutando proceso: ";
                cout << "[" << memoria[indiceBloque].id << "," << memoria[indiceBloque].tamProceso << ",(" 
                << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante <<  "]";

                // Ejecutar proceso (restarle el cuanto del sistema)
                memoria[indiceBloque].cuantoRestante -= cuantoSistema;
                if (memoria[indiceBloque].cuantoRestante < 0) memoria[indiceBloque].cuantoRestante = 0;

                // Verificar si el proceso terminó 
                if (memoria[indiceBloque].cuantoRestante <= 0) {
                        cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << "," 
                        << memoria[indiceBloque].tamProceso << ",(" 
                        << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante 
                        <<  "]"<< " termino su ejecucion";
                    // Liberar bloque de memoria que se le asignó si ya terminó 
                    memoria[indiceBloque].id = 0;
                    memoria[indiceBloque].libre = true;
                    memoria[indiceBloque].cuantoRestante = 0;
                    memoria[indiceBloque].tamProceso = 0;
                    procesosAtendidos++;

                    cout << "\n\n- Se libero el bloque de memoria ocupado por el proceso con ID [" << idProcesoActual << "]";

                    // Verificar si el porceso en espera ya puede entrar a memoria (si es que hay uno en espera)
                    if (!colaEspera.empty()) {
                        Procesos e = colaEspera.front();
                        colaEspera.pop();
                        bool asignado = asignarBuddy(e);

                        // Volver a poner en espera si no aun no hay espacio en memoria disponible 
                        if (!asignado) {
                            colaEspera.push(e); 
                        }

                        // Poner en la cola de ejecución si se le puede asignar memoria
                        else {
                            colaEjecucion.push(e.getId()); 
                            cout << " que estaba en espera";  
                        }
                    }
                }
                else { // reencolar proceso si no terminó
                    cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << "," 
                        << memoria[indiceBloque].tamProceso << ",(" 
                        << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante 
                        <<  "]"<< " NO termino su ejecucion"
                        << "\nReencolando proceso";
                    colaEjecucion.push(idProcesoActual);
                }
                
            }
        }

        // Fusionar el par de bloques contiguos más pequeños (solo una fusión pro ciclo)
        cout << "\n\n- Fusionando bloques";
        fusionarLazyBuddy();
            
        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b: memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                    << b.tam << ")," << b.cuantoRestante <<  "]";    
                }
            }
        }

        Sleep(velocidad);
    }

    cout << "\n\nFin de la simulacion de LAZY BUDDY SYSTEM\n";

    /*cout << "\n\nEstadisticas finales de la simulacion  de Lazy Buddy System.\n";
    cout << "Procesos totales generados: " << procesosGenerados << endl;
    cout << "Procesos atendidos completamente: " << procesosAtendidos << endl;
    cout << "Procesos en cola de espera: " << colaEspera.size() << endl;
    system("pause");*/
}

// ---------------------------------------------------------------
void mostrarProceso(Procesos p) {
    cout << "[" << p.getId() << "," << p.getTam() << "," << p.getCuanto() << "]";
}

// mostrar proceso juntamente con tamaño de memoria asignada, se usa en asignarBuddy
void mostrarProceso(Procesos p, int memoriaAsignada) {
    cout << "[" << p.getId() << "," << p.getTam() << ",(" << memoriaAsignada << ")," << p.getCuanto() << "]";
}

bool detenerSimulacion() {
    if (_kbhit()) {
        char tecla = _getch();
        if (tecla == 27) return true; // tecla ESC
    }
    return false;
}

/*void control_velocidad() {
    if (_kbhit()) {
        char tecla = _getch();
        // Detectar teclas especiales (flechas)
        if (tecla == -32 || tecla == 0) { // Prefijo de teclas especiales en Windows
            tecla = _getch(); // Leer el segundo byte
            if (tecla == 72) { // Flecha arriba aumenta velocidad (disminuye tiempo)
                velocidad -= 50; 
                if (velocidad < velMax) velocidad = velMax;
                cout << "\n Velocidad aumentada a " << velocidad << " ms";
            } 
            else if (tecla == 80) { // Flecha abajo disminuye velocidad (aumenta tiempo)
                velocidad += 50;
                if (velocidad > velMin) velocidad = velMin;
                cout << "\n Velocidad disminuida a " << velocidad << " ms";
            }
        }
    }
}*/

/*void mostrarEstadisticasDuranteEjecucion() {
    velActual = velocidad / 100.0;
    
    cout << "\n\n--- ESTADISTICAS ---";
    cout << "\nVelocidad actual: " << velocidad << " ms (" << velActual << " segundos)";
    
    // Contar procesos en ejecución
    int procesosEnEjecucion = 0;
    int procesoActual = 0;
    for (auto& b : memoria) {
        if (!b.libre) {
            procesosEnEjecucion++;
            if (procesoActual == 0) procesoActual = b.id;
        }
    }
    
    cout << "\nProceso que se esta atendiendo: ";
    if (procesoActual > 0) {
        cout << "Proceso [" << procesoActual << "]";
    } else {
        cout << "Ninguno";
    }
    
    cout << "\nProcesos en memoria: " << procesosEnEjecucion;
    cout << "\nProcesos en cola de espera: " << colaEspera.size();
    cout << "\nProcesos atendidos completamente: " << procesosAtendidos;
    
    int memoriaOcupada = calcularMemoriaOcupada();
    float porcentaje = (memoriaOcupada * 100.0) / tamMemoria;
    cout << "\nPorcentaje de memoria ocupado: " << porcentaje << "% (" 
         << memoriaOcupada << " KB de " << tamMemoria << " KB)";
    
    cout << "\nParticionamiento de la memoria: " << memoria.size() << " bloques";
    cout << endl;
    
    velAnterior = velActual;
}*/