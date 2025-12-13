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
#include <iomanip>

/* ----------  INICIO PORTABILIDAD  ---------- */
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
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

/* ---------------------- COLORES ANSI ---------------------- */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"

#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define WHITE   "\033[97m"
#define GRAY    "\033[90m"

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

void limpiarPantalla();
void animacionBienvenida();
void dibujarMemoriaVisual();
void dibujarBarraProgreso(int actual, int total, int ancho);
void dibujarLinea(int ancho, char c);
void animacionCarga(string mensaje, int duracion);
void mostrarEstadisticas();

/* ---------------------- FUNCIONES GRÁFICAS ---------------------- */
void limpiarPantalla() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void dibujarLinea(int ancho, char c) {
    for(int i = 0; i < ancho; i++) cout << c;
    cout << endl;
}

void animacionCarga(string mensaje, int duracion) {
    const char* animacion[] = {"|", "/", "-", "\\"};
    int pasos = duracion / 100;
    
    for(int i = 0; i < pasos; i++) {
        cout << "\r  " << animacion[i % 4] << " " << mensaje << "..." << flush;
        Sleep(100);
    }
    cout << "\r  " << GREEN << "> " << mensaje << " [OK]" << RESET << "          " << endl;
}

void dibujarBarraProgreso(int actual, int total, int ancho) {
    float progreso = (float)actual / total;
    int lleno = progreso * ancho;
    
    cout << "[";
    for(int i = 0; i < ancho; i++) {
        if(i < lleno) cout << GREEN << "=" << RESET;
        else cout << GRAY << "-" << RESET;
    }
    cout << "] " << fixed << setprecision(1) << (progreso * 100) << "%";
}

void animacionBienvenida() {
    limpiarPantalla();
    
    cout << endl << endl;
    cout << CYAN;
    string linea1 = "  ===============================================================================";
    for(char c : linea1) {
        cout << c << flush;
        Sleep(5);
    }
    cout << endl;
    
    string banner[] = {
    "  ===============================================================================",
    "                                                                              ",
    "            ####  #  #   #  #  #  #      ###    ###   #   ###   #   #         ",
    "           #      #  ## ##  #  #  #     #   #  #      #  #   #  ##  #         ",
    "            ###   #  # # #  #  #  #     #####  #      #  #   #  # # #         ",
    "               #  #  #   #  #  #  #     #   #  #      #  #   #  #  ##         ",
    "           ####   #  #   #   ##   ####  #   #   ###   #   ###   #   #         ",
    "                                                                              ",
    "                              DE GESTION DE MEMORIA                           ",
    "                                                                              ",
    "                         Buddy System & Lazy Buddy System                     ",
    "                                                                              ",
    "  ===============================================================================",
    };
    
    for(const string& linea : banner) {
        cout << linea << endl;
        Sleep(50);
    }
    
    cout << "  ===============================================================================" << RESET << endl;
    cout << endl;
    
    string subtitulo = "                      Algoritmos: Buddy System & Lazy Buddy";
    cout << WHITE;
    for(char c : subtitulo) {
        cout << c << flush;
        Sleep(20);
    }
    cout << RESET << endl << endl;
    
    cout << CYAN;
    dibujarLinea(80, '=');
    cout << RESET << endl;
    
    animacionCarga("Inicializando sistema", 400);
    animacionCarga("Cargando modulos de memoria", 400);
    animacionCarga("Verificando integridad de datos", 400);
    animacionCarga("Preparando interfaz grafica", 400);
    animacionCarga("Configurando algoritmos", 400);
    
    cout << endl;
    
    for(int i = 0; i < 3; i++) {
        cout << "\r  " << GREEN << ">>> Sistema listo para usar <<<" << RESET << flush;
        Sleep(300);
        cout << "\r  " << WHITE << ">>> Sistema listo para usar <<<" << RESET << flush;
        Sleep(300);
    }
    cout << "\r  " << GREEN << ">>> Sistema listo para usar <<<" << RESET << endl;
    Sleep(800);
}

void dibujarMemoriaVisual() {
    cout << endl;
    cout << YELLOW << "  Visualizacion de Memoria:" << RESET << endl;
    cout << "  ";
    dibujarLinea(60, '-');
    
    cout << "  ";
    int bloquesPorLinea = 16;
    int contador = 0;
    
    for (auto& b : memoria) {
        if (contador % bloquesPorLinea == 0 && contador != 0) {
            cout << endl << "  ";
        }
        
        if (!b.libre) {
            cout << CYAN << "[" << WHITE << "P" << b.id << CYAN << "]" << RESET;
        } else {
            cout << GRAY << "[ ]" << RESET;
        }
        Sleep(10);
        contador++;
    }
    
    cout << endl << "  ";
    dibujarLinea(60, '-');
    cout << "  " << GRAY << "Leyenda: " << CYAN << "[Px]" << GRAY << " = Ocupado   " 
         << "[ ] = Libre" << RESET << endl;
}

void mostrarEstadisticas() {
    int memoriaOcupada = calcularMemoriaOcupada();
    int memoriaLibre = tamMemoria - memoriaOcupada;
    float porcentajeUso = (float)memoriaOcupada / tamMemoria * 100;
    
    cout << endl;
    cout << CYAN;
    dibujarLinea(70, '=');
    cout << RESET;
    cout << YELLOW << "  ESTADISTICAS DEL SISTEMA" << RESET << endl;
    cout << CYAN;
    dibujarLinea(70, '-');
    cout << RESET;
    
    cout << "  Memoria Total:      " << WHITE << tamMemoria << " KB" << RESET << endl;
    cout << "  Memoria Ocupada:    " << GREEN << memoriaOcupada << " KB" << RESET 
         << " (" << fixed << setprecision(1) << porcentajeUso << "%)" << endl;
    cout << "  Memoria Libre:      " << BLUE << memoriaLibre << " KB" << RESET << endl;
    
    cout << CYAN;
    dibujarLinea(70, '-');
    cout << RESET;
    
    cout << "  Procesos Generados:   " << WHITE << procesosGenerados << RESET << endl;
    cout << "  Procesos Completados: " << GREEN << procesosAtendidos << RESET << endl;
    cout << "  En Cola de Espera:    " << YELLOW << colaEspera.size() << RESET << endl;
    
    cout << CYAN;
    dibujarLinea(70, '-');
    cout << RESET;
    
    cout << "  Uso de Memoria: ";
    dibujarBarraProgreso(memoriaOcupada, tamMemoria, 40);
    cout << endl;
    
    cout << "  Estado: ";
    if (procesosAtendidos < procesosGenerados) {
        cout << GREEN << "[ ACTIVO ]" << RESET;
    } else {
        cout << GRAY << "[ INACTIVO ]" << RESET;
    }
    cout << endl;
    
    cout << CYAN;
    dibujarLinea(70, '=');
    cout << RESET;
}

/* ---------------------- MAIN ---------------------- */
int main() {
    srand(time(nullptr));
    animacionBienvenida();
    mostrarMenuPrincipal();
    restauraTerminal();
    return 0;
}

/* ---------------------- IMPLEMENTACIONES ---------------------- */
void mostrarMenuPrincipal() {
    int opcion;
    do {
        limpiarPantalla();
        cout << endl;
        cout << CYAN;
        dibujarLinea(60, '=');
        cout << RESET;
        cout << YELLOW << "  MENU PRINCIPAL - SIMULADOR DE GESTION DE MEMORIA" << RESET << endl;
        cout << CYAN;
        dibujarLinea(60, '=');
        cout << RESET << endl;
        
        cout << "  " << RED << "0." << RESET << " Salir del programa" << endl;
        cout << "  " << GREEN << "1." << RESET << " Buddy System (Fusion inmediata)" << endl;
        cout << "  " << BLUE << "2." << RESET << " Lazy Buddy System (Fusion diferida)" << endl;
        cout << endl;
        cout << CYAN;
        dibujarLinea(60, '-');
        cout << RESET;
        cout << "  Elige una opcion: ";
        cin >> opcion;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            opcion = 500;
        }
        
        switch (opcion) {
            case 1: 
                configurarParametros(); 
                implementar_Buddy(); 
                break;
            case 2: 
                configurarParametros(); 
                implementar_Lazy();  
                break;
            case 0: 
                limpiarPantalla();
                cout << endl << endl;
                cout << GREEN << "  Gracias por usar el simulador." << endl;
                cout << "  Hasta pronto!" << RESET << endl << endl;
                break;
            default: 
                cout << endl << RED << "  Opcion invalida. Intenta de nuevo." << RESET << endl;
                Sleep(1500);
        }
        cout << endl;
    } while (opcion != 0);
}

void configurarParametros() {
    limpiarPantalla();
    int maxPermitido;
    cout << endl;
    cout << CYAN;
    dibujarLinea(60, '=');
    cout << RESET;
    cout << MAGENTA << "  CONFIGURACION DE PARAMETROS DEL SISTEMA" << RESET << endl;
    cout << CYAN;
    dibujarLinea(60, '=');
    cout << RESET << endl;
    
    do {
        cout << "  Tamanio total de memoria (1, 4 o 8 MB): ";
        cin >> tamMemoria;
    } while (tamMemoria != 1 && tamMemoria != 4 && tamMemoria != 8);
    
    tamMemoria *= 1024;

    if (tamMemoria == 1024) maxPermitido = 64;
    else if (tamMemoria == 4096) maxPermitido = 128;
    else maxPermitido = 256;

    float tamProces = 0;
    do {
        cout << "  Tamanio maximo por proceso (Min: 32 KB, Max: " << maxPermitido << " KB): ";
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

    float cuantProces = 0;
    do {
        cout << "  Cuanto del proceso: ";
        cin >> cuantProces;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            tamMaxCuanto = 0;
        } else if (fmod(cuantProces, 1) != 0) {
            tamMaxCuanto= 0;
        } else {
            tamMaxCuanto = static_cast<int>(cuantProces);
        }
    } while (tamMaxCuanto  <= 0);


    float cuantSist = 0;
    do {
        cout << "  Cuanto del sistema: ";
        cin >> cuantSist;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cuantoSistema = 0;
        } else if (fmod(cuantSist, 1) != 0) {
            cuantoSistema = 0;
        } else {
            cuantoSistema = static_cast<int>(cuantSist);
        }
    } while (cuantoSistema  <= 0);

    cout << endl;
    animacionCarga("Aplicando configuracion", 400);
    animacionCarga("Validando parametros", 300);
    animacionCarga("Inicializando estructuras de memoria", 400);
    
    cout << endl;
    cout << CYAN;
    dibujarLinea(60, '=');
    cout << RESET;
    cout << GREEN << "  CONFIGURACION APLICADA EXITOSAMENTE" << RESET << endl;
    cout << CYAN;
    dibujarLinea(60, '=');
    cout << RESET << endl;
    
    cout << "  Memoria Total:         " << WHITE << tamMemoria / 1024 << " MB" << RESET 
         << " (" << tamMemoria << " KB)" << flush;
    Sleep(200);
    cout << endl;
    
    cout << "  Tamanio Max. Proceso:  " << WHITE << tamMaxMemoProces << " KB" << RESET << flush;
    Sleep(200);
    cout << endl;
    
    cout << "  Cuanto del Sistema:    " << WHITE << cuantoSistema << RESET << flush;
    Sleep(200);
    cout << endl;
    
    cout << "  Cuanto Max. Proceso:   " << WHITE << tamMaxCuanto << RESET << flush;
    Sleep(200);
    cout << endl;
    
    cout << endl;
    cout << CYAN;
    dibujarLinea(60, '-');
    cout << RESET;
    cout << BLUE << "  CONTROLES DURANTE LA SIMULACION:" << RESET << endl;
    cout << "    Tecla 'A'    - Aumentar velocidad" << endl;
    cout << "    Tecla 'D'    - Disminuir velocidad" << endl;
    cout << "    Tecla 'S'    - Terminar simulacion" << endl;
    cout << CYAN;
    dibujarLinea(60, '-');
    cout << RESET;

    memoria.clear();
    memoria.push_back({0, tamMemoria, true, 0, 0});
    while (!colaEspera.empty()) colaEspera.pop();
    procesosAtendidos = 0;
    procesosGenerados = 0;
    idProceso = 1;

    cout << endl;
    for(int i = 0; i < 3; i++) {
        cout << "\r  " << GREEN << ">>> Presiona Enter para comenzar <<<" << RESET << flush;
        Sleep(300);
        cout << "\r  " << WHITE << ">>> Presiona Enter para comenzar <<<" << RESET << flush;
        Sleep(300);
    }
    cout << "\r  " << GREEN << ">>> Presiona Enter para comenzar <<<" << RESET;
    cin.ignore(); 
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
    if (indicador) cout << GREEN << "  > Fusion de bloques exitosa" << RESET << endl;
    else cout << YELLOW << "  > No habia bloques para fusionar" << RESET << endl;
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
        cout << GREEN << "  > Fusion de bloques exitosa" << RESET << endl;
    } else cout << YELLOW << "  > No habia bloques para fusionar" << RESET << endl;
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
            cout << GREEN << "  > Se asigno memoria al proceso: " << RESET;
            mostrarProceso(p, memoria[i].tam);
            cout << endl;
            return true;
        }
    }
    cout << YELLOW << "  > El proceso esta en espera, no hay memoria disponible" << RESET << endl;
    return false;
}

int calcularMemoriaOcupada() {
    int ocupada = 0;
    for (auto& b : memoria) if (!b.libre) ocupada += b.tam;
    return ocupada;
}

void mostrarProceso(Procesos p) {
    cout << WHITE << "[" << p.getId() << "," << p.getTam() << "," << p.getCuanto() << "]" << RESET;
}

void mostrarProceso(Procesos p, int memoriaAsignada) {
    cout << WHITE << "[" << p.getId() << "," << p.getTam() << ",(" << memoriaAsignada << ")," << p.getCuanto() << "]" << RESET;
}

char leeTeclaSimple()
{
    char c;
    do {
        c = static_cast<char>(toupper(_getch()));
        if (c != 'A' && c != 'D' && c != 'S') c = 'N';
    } while (c != 'A' && c != 'D' && c != 'S' && c != 'N');
    return c;
}

int control_velocidad(int velActual, char tecla) {
    if (tecla == 'A') {
        velActual -= 1000;
        if (velActual < 1) velActual = 1;
    } else if (tecla == 'D') {
        velActual += 1000;
    }
    return velActual;
}

/* ---------------- Buddy System -------------- */
void implementar_Buddy() {
    limpiarPantalla();
    cout << endl;
    cout << GREEN;
    dibujarLinea(60, '=');
    cout << "  INICIANDO SIMULACION BUDDY SYSTEM" << endl;
    dibujarLinea(60, '=');
    cout << RESET;
    Sleep(1000);
    
    int ciclo = 0, velocidad = 1000;
    memoria.clear();
    memoria.push_back({0, tamMemoria, true, 0, 0});
    queue<int> colaEjecucion;

    while (true) {
        if (_kbhit()) {
            char tecla = leeTeclaSimple();
            if (tecla == 'S') break;
            else if (tecla == 'N') cout << "";
            else velocidad = control_velocidad(velocidad, tecla);
        }

        ciclo++;
        
        cout << endl;
        cout << CYAN;
        dibujarLinea(70, '=');
        cout << RESET;
        cout << YELLOW << "  CICLO " << ciclo << " - BUDDY SYSTEM" << RESET << endl;
        cout << CYAN;
        dibujarLinea(70, '=');
        cout << RESET << endl;
        
        cout << "  Velocidad: " << WHITE << velocidad/1000 << " segundo(s)" << RESET << endl;

        if (colaEspera.empty()) {
            Procesos p = generarProceso();
            cout << endl << CYAN << "  Nuevo proceso creado: " << RESET;
            mostrarProceso(p);
            cout << endl << "  Memoria antes de asignar: ";
            mostrarMemoria();
            cout << endl;

            bool asignado = asignarBuddy(p);
            if (!asignado) colaEspera.push(p);
            else colaEjecucion.push(p.getId());
        } else cout << endl << YELLOW << "  No se genero proceso nuevo (hay uno en espera)" << RESET << endl;

        cout << endl << "  Memoria actual: ";
        mostrarMemoria();
        dibujarMemoriaVisual();

        cout << endl << "  Procesos en cola Round Robin: ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso 
                    << ",(" << b.tam << ")," << b.cuantoRestante << "]";
                }
            }        
        } else {
            cout << GRAY << "vacia" << RESET;
        }

        if (!colaEjecucion.empty()) {
            int idProcesoActual = colaEjecucion.front();
            colaEjecucion.pop();
            int indiceBloque = -1;
            for (size_t i = 0; i < memoria.size(); i++)
                if (memoria[i].id == idProcesoActual && !memoria[i].libre) { indiceBloque = i; break; }

            if (indiceBloque != -1) {
                cout << endl << endl << MAGENTA << "  > Ejecutando proceso: " << RESET;
                cout << "[" << memoria[indiceBloque].id << "," << memoria[indiceBloque].tamProceso << ",("
                     << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante << "]";

                memoria[indiceBloque].cuantoRestante -= cuantoSistema;
                if (memoria[indiceBloque].cuantoRestante < 0) memoria[indiceBloque].cuantoRestante = 0;

                if (memoria[indiceBloque].cuantoRestante <= 0) {
                    cout << endl << endl << GREEN << "  > El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " termino su ejecucion" << RESET << endl;
                    memoria[indiceBloque].id = 0;
                    memoria[indiceBloque].libre = true;
                    memoria[indiceBloque].cuantoRestante = 0;
                    memoria[indiceBloque].tamProceso = 0;
                    procesosAtendidos++;
                    cout << BLUE << "  > Se libero el bloque del proceso ID [" << idProcesoActual << "]" << RESET << endl;

                    cout << YELLOW << "  > Fusionando bloques..." << RESET << endl;
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
                    cout << endl << endl << YELLOW << "  > El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " NO termino su ejecucion" << RESET
                         << endl << BLUE << "  > Reencolando proceso..." << RESET << endl;
                    colaEjecucion.push(idProcesoActual);
                }
            }
        }

        cout << endl << endl << "  Memoria final del ciclo: ";
        mostrarMemoria();
        dibujarMemoriaVisual();

        cout << endl << "  Procesos restantes: ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre) {
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                    << b.tam << ")," << b.cuantoRestante <<  "]";    
                }
            }
        } else {
            cout << GRAY << "ninguno" << RESET;
        }

        mostrarEstadisticas();
        Sleep(velocidad);
    }

    cout << endl;
    cout << RED;
    dibujarLinea(60, '=');
    cout << "  FIN DE SIMULACION BUDDY SYSTEM" << endl;
    dibujarLinea(60, '=');
    cout << RESET << endl;
    
    cout << YELLOW << "  ESTADISTICAS FINALES - BUDDY SYSTEM" << RESET << endl;
    cout << CYAN;
    dibujarLinea(60, '=');
    cout << RESET << endl;
    
    cout << "  Procesos totales generados:      " << WHITE << procesosGenerados << RESET << endl;
    cout << "  Procesos atendidos completamente: " << GREEN << procesosAtendidos << RESET << endl;
    
    if (procesosGenerados > 0) {
        float tasaExito = (float)procesosAtendidos / procesosGenerados * 100;
        cout << "  Tasa de exito:                    " << CYAN << fixed << setprecision(1) 
             << tasaExito << "%" << RESET << endl;
    }
    
    cout << endl << "  Presiona Enter para continuar...";
    cin.get();
}

/* ---------------- Lazy Buddy System -------------- */
void implementar_Lazy() {
    limpiarPantalla();
    cout << endl;
    cout << BLUE;
    dibujarLinea(60, '=');
    cout << "  INICIANDO SIMULACION LAZY BUDDY SYSTEM" << endl;
    dibujarLinea(60, '=');
    cout << RESET;
    Sleep(1000);
    
    int ciclo = 0, velocidad = 1000;
    memoria.clear();
    memoria.push_back({0, tamMemoria, true, 0, 0});
    queue<int> colaEjecucion;

    while (true) {
        if (_kbhit()) {
            char tecla = leeTeclaSimple();
            if (tecla == 'S') break;
            else if (tecla == 'N') cout << "";
            else velocidad = control_velocidad(velocidad, tecla);
        }

        ciclo++;
        
        cout << endl;
        cout << CYAN;
        dibujarLinea(70, '=');
        cout << RESET;
        cout << BLUE << "  CICLO " << ciclo << " - LAZY BUDDY SYSTEM" << RESET << endl;
        cout << CYAN;
        dibujarLinea(70, '=');
        cout << RESET << endl;
        
        cout << "  Velocidad: " << WHITE << velocidad/1000 << " segundo(s)" << RESET << endl;

        if (colaEspera.empty()) {
            Procesos p = generarProceso();
            cout << endl << CYAN << "  Nuevo proceso creado: " << RESET;
            mostrarProceso(p);
            cout << endl << "  Memoria antes de asignar: ";
            mostrarMemoria();
            cout << endl;

            bool asignado = asignarBuddy(p);
            if (!asignado) colaEspera.push(p);
            else colaEjecucion.push(p.getId());
        } else cout << endl << YELLOW << "  No se genero proceso nuevo (hay uno en espera)" << RESET << endl;

        cout << endl << "  Memoria actual: ";
        mostrarMemoria();
        dibujarMemoriaVisual();

        cout << endl << "  Procesos en cola Round Robin: ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre)
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                         << b.tam << ")," << b.cuantoRestante << "]";
            }
        } else {
            cout << GRAY << "vacia" << RESET;
        }

        if (!colaEjecucion.empty()) {
            int idProcesoActual = colaEjecucion.front();
            colaEjecucion.pop();
            int indiceBloque = -1;
            for (size_t i = 0; i < memoria.size(); i++)
                if (memoria[i].id == idProcesoActual && !memoria[i].libre) { indiceBloque = i; break; }

            if (indiceBloque != -1) {
                cout << endl << endl << MAGENTA << "  > Ejecutando proceso: " << RESET;
                cout << "[" << memoria[indiceBloque].id << "," 
                     << memoria[indiceBloque].tamProceso << ",("
                     << memoria[indiceBloque].tam << ")," 
                     << memoria[indiceBloque].cuantoRestante << "]";

                memoria[indiceBloque].cuantoRestante -= cuantoSistema;
                if (memoria[indiceBloque].cuantoRestante < 0) memoria[indiceBloque].cuantoRestante = 0;

                if (memoria[indiceBloque].cuantoRestante <= 0) {
                    cout << endl << endl << GREEN << "  > El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " termino su ejecucion" << RESET << endl;
                    memoria[indiceBloque].id = 0;
                    memoria[indiceBloque].libre = true;
                    memoria[indiceBloque].cuantoRestante = 0;
                    memoria[indiceBloque].tamProceso = 0;
                    procesosAtendidos++;

                    cout << BLUE << "  > Se libero el bloque del proceso ID [" << idProcesoActual << "]" << RESET << endl;

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
                    cout << endl << endl << YELLOW << "  > El proceso " << "[" << memoria[indiceBloque].id << ","
                         << memoria[indiceBloque].tamProceso << ",("
                         << memoria[indiceBloque].tam << ")," << memoria[indiceBloque].cuantoRestante
                         << "]" << " NO termino su ejecucion" << RESET
                         << endl << BLUE << "  > Reencolando proceso..." << RESET << endl;
                    colaEjecucion.push(idProcesoActual);
                }
            }
        }

        cout << endl << endl << YELLOW << "  > Fusionando bloques (Lazy)..." << RESET << endl;
        fusionarLazyBuddy();

        cout << endl << endl << "  Memoria final del ciclo: ";
        mostrarMemoria();
        dibujarMemoriaVisual();

        cout << endl << "  Procesos restantes: ";
        if (!colaEjecucion.empty()) {
            for (auto& b : memoria) {
                if (!b.libre)
                    cout << "[" << b.id << "," << b.tamProceso << ",(" 
                         << b.tam << ")," << b.cuantoRestante << "]";
            }
        } else {
            cout << GRAY << "ninguno" << RESET;
        }

        mostrarEstadisticas();
        Sleep(velocidad);
    }

    cout << endl;
    cout << RED;
    dibujarLinea(60, '=');
    cout << "  FIN DE SIMULACION LAZY BUDDY SYSTEM" << endl;
    dibujarLinea(60, '=');
    cout << RESET << endl;
    
    cout << BLUE << "  ESTADISTICAS FINALES - LAZY BUDDY SYSTEM" << RESET << endl;
    cout << CYAN;
    dibujarLinea(60, '=');
    cout << RESET << endl;
    
    cout << "  Procesos totales generados:      " << WHITE << procesosGenerados << RESET << endl;
    cout << "  Procesos atendidos completamente: " << GREEN << procesosAtendidos << RESET << endl;
    
    if (procesosGenerados > 0) {
        float tasaExito = (float)procesosAtendidos / procesosGenerados * 100;
        cout << "  Tasa de exito:                    " << CYAN << fixed << setprecision(1) 
             << tasaExito << "%" << RESET << endl;
    }
    
    cout << endl << "  Presiona Enter para continuar...";
    cin.get();
}