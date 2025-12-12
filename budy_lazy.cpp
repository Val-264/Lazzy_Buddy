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
#include <ctime>
#include <cstdlib>

/* ----------  INICIO PORTABILIDAD  ---------- */
#ifdef _WIN32
    #include <conio.h>      // _kbhit, _getch
    #include <windows.h>    // Sleep

    static void restauraTerminal() {}
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    static struct termios oldT, newT;
    static int tiosListo = 0;

    static void preparaTerminal()
    {
        tcgetattr(STDIN_FILENO, &oldT);
        newT = oldT;
        newT.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newT);
        tiosListo = 1;
    }
    static void restauraTerminal()
    {
        if (tiosListo) tcsetattr(STDIN_FILENO, TCSANOW, &oldT);
    }

    int _kbhit(void)
    {
        if (!tiosListo) preparaTerminal();
        int ch, oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        ch = getchar();
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        if (ch != EOF) { ungetc(ch, stdin); return 1; }
        return 0;
    }
    int _getch(void)
    {
        if (!tiosListo) preparaTerminal();
        return getchar();
    }
    #define Sleep(ms) usleep((ms)*1000)
#endif
/* ----------  FIN PORTABILIDAD  ---------- */

using namespace std;

/* ---------------------- CLASE DE PROCESOS ---------------------- */
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
    void setCuanto(int c) { cuanto = c; }
};

/* ---------------------- VARIABLES GLOBALES ---------------------- */
int totProcesos;
int cuantoSistema;
int tamMaxMemoProces;
int tamMaxCuanto = 1;
int tamMemoria;
int idProceso = 1;
int procesosAtendidos = 0;
int procesosGenerados = 0;

/* ---------------------- ESTRUCTURA DE BLOQUE ---------------------- */
struct Bloque {
    int id;
    int tam;
    bool libre;
    int cuantoRestante;
    int tamProceso;
};
vector<Bloque> memoria;
queue<Procesos> colaEspera;

/* ---------------------- PROTOTIPOS ---------------------- */
void mostrarMenuPrincipal();
void configurarParametros();
Procesos generarProceso();
void mostrarProceso(Procesos p);
void mostrarProceso(Procesos p, int memoriaAsignada);
void mostrarMemoria();
int control_velocidad(int velActual, char tecla);
bool asignarBuddy(Procesos p);
int calcularMemoriaOcupada();
void procesarCiclo();
void implementar_Buddy();
void fusionarBuddy();
void implementar_Lazy();
void fusionarLazyBuddy();
bool esPotenciaDeDos(int n);

/* ---------------------- MAIN ---------------------- */
int main() {
    srand(time(nullptr));
    mostrarMenuPrincipal();
    restauraTerminal();   // Linux: deja el terminal como estaba
    return 0;
}

/* ---------------------- IMPLEMENTACIONES ---------------------- */
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
        switch (opcion) {
            case 1: configurarParametros(); implementar_Buddy(); break;
            case 2: configurarParametros(); implementar_Lazy();  break;
            case 0: cout << "Saliendo..."; break;
            default: cout << "Opcion invalida\n";
        }
        cout << endl;
    } while (opcion != 0);
}

void configurarParametros() {
    int maxPermitido;
    do {
        cout << "Tamanio total de memoria (1, 4 o 8 MB): ";
        cin >> tamMemoria;
    } while (tamMemoria != 1 && tamMemoria != 4 && tamMemoria != 8);
    tamMemoria *= 1024;
    cuantoSistema = 3;
    tamMaxCuanto = 10;

    if (tamMemoria == 1024) maxPermitido = 64;
    else if (tamMemoria == 4096) maxPermitido = 128;
    else maxPermitido = 256;

    float tamProces = 0;
    do {
        cout << "Tamanio maximo de memoria por proceso ( Minimo 32 KB, maximo " << maxPermitido << " KB): ";
        cin >> tamProces;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            tamMaxMemoProces = 0;
        } else if (fmod(tamProces, 1) != 0) {
            tamMaxMemoProces = 0;
        } else {
            tamMaxMemoProces = static_cast<int>(tamProces);
        }
    } while (tamMaxMemoProces < 32 || tamMaxMemoProces > tamMemoria || tamMaxMemoProces > maxPermitido);

    cout << "\n\n--- CONFIGURACION APLICADA ---";
    cout << "\nTamanio total de memoria: " << tamMemoria / 1024 << " MB (" << tamMemoria << " KB)";
    cout << "\nTamanio maximo por proceso: " << tamMaxMemoProces << " KB";
    cout << "\nCuanto del sistema: " << cuantoSistema;
    cout << "\nCuanto maximo por proceso: " << tamMaxCuanto;
    cout << "\n--------------------------------\n";

    memoria.clear();
    memoria.push_back({0, tamMemoria, true, 0, 0});
    while (!colaEspera.empty()) colaEspera.pop();
    procesosAtendidos = 0;
    procesosGenerados = 0;
    idProceso = 1;

    cout << "\nDurante la simulacion puedes:"
         << "\n1- AUMENTAR LA VELOCIDAD presionando la 'A';"
         << "\n2- DISMINUIR LA VELOCIDAD presionando la tecla 'D';"
         << "\n3- TERMINAR LA SIMULACION presionando la tecla 'S'.\n\n";
    cout << "(Presiona Enter para continuar...)" << endl;
    cin.get();
}

void mostrarMemoria() {
    for (auto& b : memoria) {
        cout << "[" << b.id << ",";
        if (!b.libre) cout << b.tamProceso;
        else cout << 0;
        cout << ",";
        if (!b.libre) cout << "(";
        cout << b.tam;
        if (!b.libre) cout << ")";
        cout << "," << b.cuantoRestante << "]";
    }
}

Procesos generarProceso() {
    int tam = (rand() % (tamMaxMemoProces - 32 + 1)) + 32;
    int cuanto = (rand() % (tamMaxCuanto - 3 + 1)) + 3;
    Procesos p(idProceso++, tam, cuanto);
    procesosGenerados++;
    return p;
}

bool esPotenciaDeDos(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

void fusionarBuddy() {
    bool fusionado = true, indicador = false;
    while (fusionado) {
        fusionado = false;
        for (size_t i = 0; i < memoria.size() - 1; i++) {
            if (memoria[i].libre && memoria[i + 1].libre &&
                memoria[i].tam == memoria[i + 1].tam &&
                esPotenciaDeDos(memoria[i].tam)) {
                int tamFusionado = memoria[i].tam * 2;
                if (tamFusionado <= tamMemoria) {
                    memoria[i].tam = tamFusionado;
                    memoria.erase(memoria.begin() + i + 1);
                    fusionado = true;
                    indicador = true;
                    break;
                }
            }
        }
    }
    if (indicador) cout << "\nFusion de bloques exitosa";
    else cout << "\nNo habia bloques para fusionar";
}

void fusionarLazyBuddy() {
    int indiceMenor = -1, tamMenor = tamMemoria + 1;
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
    if (indiceMenor != -1) {
        int tamFusionado = memoria[indiceMenor].tam * 2;
        if (tamFusionado <= tamMemoria) {
            memoria[indiceMenor].tam = tamFusionado;
            memoria.erase(memoria.begin() + indiceMenor + 1);
        }
        cout << "\nFusion de bloques exitosa";
    } else cout << "\nNo habia bloques para fusionar";
}

bool asignarBuddy(Procesos p) {
    for (size_t i = 0; i < memoria.size(); i++) {
        if (memoria[i].libre && memoria[i].tam >= p.getTam()) {
            while (memoria[i].tam / 2 >= p.getTam() && memoria[i].tam / 2 >= 32) {
                int nuevoTam = memoria[i].tam / 2;
                memoria[i].tam = nuevoTam;
                Bloque b2 = {0, nuevoTam, true, 0, 0};
                memoria.insert(memoria.begin() + i + 1, b2);
            }
            memoria[i].id = p.getId();
            memoria[i].libre = false;
            memoria[i].cuantoRestante = p.getCuanto();
            memoria[i].tamProceso = p.getTam();
            cout << "\n\n- Se le asigno memoria al proceso: ";
            mostrarProceso(p, memoria[i].tam);
            return true;
        }
    }
    cout << "\n\n- El proceso nuevo esta en espera, no hay memoria para asignarle";
    return false;
}

int calcularMemoriaOcupada() {
    int ocupada = 0;
    for (auto& b : memoria) if (!b.libre) ocupada += b.tam;
    return ocupada;
}

void mostrarProceso(Procesos p) {
    cout << "[" << p.getId() << "," << p.getTam() << "," << p.getCuanto() << "]";
}

void mostrarProceso(Procesos p, int memoriaAsignada) {
    cout << "[" << p.getId() << "," << p.getTam() << ",(" << memoriaAsignada << ")," << p.getCuanto() << "]";
}

char leeTeclaSimple()
{
    char c;
    do {
        c = static_cast<char>(toupper(_getch()));
    } while (c != 'A' && c != 'D' && c != 'S');
    return c;
}


int control_velocidad(int velActual, char tecla) {
    if (tecla == 'A') {          // aumentar
        velActual -= 1000;
        if (velActual < 1) velActual = 1;
    } else if (tecla == 'D') {   // disminuir
        velActual += 1000;
    }
    return velActual;
}

/* ---------------- Buddy System -------------- */
void implementar_Buddy() {
    cout << "\n-- Simulacion Buddy System --\n";
    int ciclo = 0, velocidad = 1000;
    memoria.clear();
    memoria.push_back({0, tamMemoria, true, 0, 0});
    queue<int> colaEjecucion;

    while (true) {
        if (_kbhit()) {
            char tecla = leeTeclaSimple();
            if (tecla == 'S') break;           // Salir 
            else velocidad = control_velocidad(velocidad, tecla);
        }

        ciclo++;
        cout << "\n\n=============================================\n";
        cout << "        CICLO " << ciclo << " BUDDY SYSTEM";
        cout << "\n=============================================\n\n";
        cout << "VELOCIDAD: " << velocidad/1000 << " segundos" << "\n\n";

        if (colaEspera.empty()) {
            Procesos p = generarProceso();
            cout << "- Nuevo proceso creado: ";
            mostrarProceso(p);
            cout << "\n\n- Memoria antes de asigar proceso: ";
            mostrarMemoria();

            bool asignado = asignarBuddy(p);
            if (!asignado) colaEspera.push(p);
            else colaEjecucion.push(p.getId());
        } else cout << "- No se genero ningun proceso nuevo en este ciclo porque ya hay uno en espera";

        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso 
                    << ",(" << b.tam << ")," << b.cuantoRestante << "]";
                }
            }        
        }

        if (!colaEjecucion.empty()) {
            int idProcesoActual = colaEjecucion.front();
            colaEjecucion.pop();
            int indiceBloque = -1;
            for (size_t i = 0; i < memoria.size(); i++)
                if (memoria[i].id == idProcesoActual && !memoria[i].libre) { indiceBloque = i; break; }

            if (indiceBloque != -1) {
                cout << "\n\n- Ejecutando proceso: ";
                cout << "[" << memoria[indiceBloque].id << "," << memoria[indiceBloque].tamProceso << ",("
                     << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante << "]";

                memoria[indiceBloque].cuantoRestante -= cuantoSistema;
                if (memoria[indiceBloque].cuantoRestante < 0) memoria[indiceBloque].cuantoRestante = 0;

                if (memoria[indiceBloque].cuantoRestante <= 0) {
                    cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " termino su ejecucion";
                    memoria[indiceBloque].id = 0;
                    memoria[indiceBloque].libre = true;
                    memoria[indiceBloque].cuantoRestante = 0;
                    memoria[indiceBloque].tamProceso = 0;
                    procesosAtendidos++;
                    cout << "\n\n- Se libero el bloque de memoria ocupado por el proceso con ID [" << idProcesoActual << "]";

                    cout << "\n\n- Fusionando bloques";
                    fusionarBuddy();

                    if (!colaEspera.empty()) {
                        Procesos e = colaEspera.front();
                        colaEspera.pop();
                        bool asignado = asignarBuddy(e);
                        if (!asignado) colaEspera.push(e);
                        else {
                            colaEjecucion.push(e.getId());
                            cout << " que estaba en espera";
                        }
                    }
                } else {
                    cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " NO termino su ejecucion"
                         << "\nReencolando proceso";
                    colaEjecucion.push(idProcesoActual);
                }
            }
        }

        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                    << b.tam << ")," << b.cuantoRestante <<  "]";    
                }
            }
        }    

        Sleep(velocidad);
    }

    cout << "\n\nFin de la simulacion de BUDDY SYSTEM\n";
    cout << "\n\nEstadisticas finales de la simulacion  de Buddy System.\n";
    cout << "Procesos totales generados: " << procesosGenerados << endl;
    cout << "Procesos atendidos completamente: " << procesosAtendidos << endl;
    cout << "(Presiona Enter para continuar...)" << endl;
    cin.ignore(); cin.get();
}

/* ---------------- Lazy Buddy System -------------- */
void implementar_Lazy() {
    cout << "\n-- Simulacion Lazy Buddy System --\n";
    int ciclo = 0, velocidad = 1000;
    memoria.clear();
    memoria.push_back({0, tamMemoria, true, 0, 0});
    queue<int> colaEjecucion;

    while (true) {
        if (_kbhit()) {
            char tecla = leeTeclaSimple();
            if (tecla == 'S') break;           // Salir 
            else velocidad = control_velocidad(velocidad, tecla);
        }

        ciclo++;
        cout << "\n\n================================================\n";
        cout << "        CICLO " << ciclo << " LAZY BUDDY SYSTEM";
        cout << "\n================================================\n\n";
        cout << "VELOCIDAD: " << velocidad << "\n\n";

        if (colaEspera.empty()) {
            Procesos p = generarProceso();
            cout << "- Nuevo proceso creado: ";
            mostrarProceso(p);
            cout << "\n\n- Memoria antes de asigar proceso: ";
            mostrarMemoria();

            bool asignado = asignarBuddy(p);
            if (!asignado) colaEspera.push(p);
            else colaEjecucion.push(p.getId());
        } else cout << "- No se genero ningun proceso nuevo en este ciclo porque ya hay uno en espera";

        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre)
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                         << b.tam << ")," << b.cuantoRestante << "]";
            }
        }

        if (!colaEjecucion.empty()) {
            int idProcesoActual = colaEjecucion.front();
            colaEjecucion.pop();
            int indiceBloque = -1;
            for (size_t i = 0; i < memoria.size(); i++)
                if (memoria[i].id == idProcesoActual && !memoria[i].libre) { indiceBloque = i; break; }

            if (indiceBloque != -1) {
                cout << "\n\n- Ejecutando proceso: ";
                cout << "[" << memoria[indiceBloque].id << "," 
                     << memoria[indiceBloque].tamProceso << ",("
                     << memoria[indiceBloque].tam << ")," 
                     << memoria[indiceBloque].cuantoRestante << "]";

                memoria[indiceBloque].cuantoRestante -= cuantoSistema;
                if (memoria[indiceBloque].cuantoRestante < 0) memoria[indiceBloque].cuantoRestante = 0;

                if (memoria[indiceBloque].cuantoRestante <= 0) {
                    cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " termino su ejecucion";
                    memoria[indiceBloque].id = 0;
                    memoria[indiceBloque].libre = true;
                    memoria[indiceBloque].cuantoRestante = 0;
                    memoria[indiceBloque].tamProceso = 0;
                    procesosAtendidos++;

                    cout << "\n\n- Se libero el bloque de memoria ocupado por el proceso con ID [" << idProcesoActual << "]";

                    if (!colaEspera.empty()) {
                        Procesos e = colaEspera.front();
                        colaEspera.pop();
                        bool asignado = asignarBuddy(e);
                        if (!asignado) colaEspera.push(e);
                        else {
                            colaEjecucion.push(e.getId());
                            cout << " que estaba en espera";
                        }
                    }
                } else {
                    cout << "\n\n- El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " NO termino su ejecucion"
                         << "\nReencolando proceso";
                    colaEjecucion.push(idProcesoActual);
                }
            }
        }

        cout << "\n\n- Fusionando bloques";
        fusionarLazyBuddy();

        cout << "\n\n- Memoria: ";
        mostrarMemoria();

        cout << "\n- Procesos restantes (cola de Round Robin): ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre)
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                         << b.tam << ")," << b.cuantoRestante << "]";
            }
        }

        Sleep(velocidad);
    }

    cout << "\n\nFin de la simulacion de LAZY BUDDY SYSTEM\n";
    cout << "\n\nEstadisticas finales de la simulacion  de Lazy Buddy System.\n";
    cout << "Procesos totales generados: " << procesosGenerados << endl;
    cout << "Procesos atendidos completamente: " << procesosAtendidos << endl;
    cout << "(Presiona Enter para continuar...)" << endl;
    cin.ignore(); 
    cin.get();
}