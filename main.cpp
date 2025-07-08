#include <iostream>
#include <fstream>
#include <cstring>
#include <limits> //

using namespace std;

// Constante para el tamaño máximo de una ruta o línea de comando
const int LONGITUD_MAX_RUTA = 1024;
const int LONGITUD_MAX_CONTENIDO = 4096; // Para el editor de texto y contenido de archivo

// Estructura para Archivos
struct Archivo {
    char* nombre;       // Nombre del archivo
    char* contenido;    // Contenido del archivo
    Archivo* siguiente;       // Puntero al siguiente archivo en la lista del directorio
};

// Estructura para Directorios
struct Directorio {
    char* nombre;            // Nombre del directorio
    Directorio* padre;      // Directorio padre
    Directorio* subdirectorios;     // Lista de subdirectorios (primer hijo)
    Directorio* siguienteDirectorio;        // Siguiente hermano en la lista de subdirectorios del padre
    Archivo* archivos;            // Lista de archivos en este directorio (primer archivo)
};

// --- Funciones auxiliares y de gestión de sistema de archivos ---

// Función para crear un nuevo archivo
Archivo* crearArchivo(const char* nombre, const char* contenido = nullptr) {
    Archivo* nuevoArchivo = new Archivo;
    nuevoArchivo->nombre = new char[strlen(nombre) + 1];
    strcpy(nuevoArchivo->nombre, nombre);

    if (contenido) {
        nuevoArchivo->contenido = new char[strlen(contenido) + 1];
        strcpy(nuevoArchivo->contenido, contenido);
    } else {
        nuevoArchivo->contenido = nullptr;
    }
    nuevoArchivo->siguiente = nullptr;
    return nuevoArchivo;
}

// Función para crear un nuevo directorio
Directorio* crearDirectorio(const char* nombre, Directorio* padre = nullptr) {
    Directorio* nuevoDirectorio = new Directorio;
    nuevoDirectorio->nombre = new char[strlen(nombre) + 1];
    strcpy(nuevoDirectorio->nombre, nombre);

    nuevoDirectorio->padre = padre;
    nuevoDirectorio->subdirectorios = nullptr;
    nuevoDirectorio->siguienteDirectorio = nullptr;
    nuevoDirectorio->archivos = nullptr;

    return nuevoDirectorio;
}

// Función para liberar memoria de un archivo
void eliminarArchivo(Archivo* archivo) {
    if (archivo) {
        delete[] archivo->nombre;
        if (archivo->contenido) delete[] archivo->contenido;
        delete archivo;
    }
}

// Función para liberar memoria de un directorio y su contenido
void eliminarDirectorio(Directorio* directorio) {
    if (directorio) {
        delete[] directorio->nombre;

        // Eliminar subdirectorios recursivamente
        Directorio* subDirectorioActual = directorio->subdirectorios;
        while (subDirectorioActual) {
            Directorio* siguienteSubDirectorio = subDirectorioActual->siguienteDirectorio;
            eliminarDirectorio(subDirectorioActual); // Llamada recursiva
            subDirectorioActual = siguienteSubDirectorio;
        }

        // Eliminar archivos
        Archivo* archivoActual = directorio->archivos;
        while (archivoActual) {
            Archivo* siguienteArchivo = archivoActual->siguiente;
            eliminarArchivo(archivoActual);
            archivoActual = siguienteArchivo;
        }
        delete directorio;
    }
}

// Encontrar archivo en un directorio
Archivo* buscarArchivo(Directorio* directorio, const char* nombre) {
    Archivo* actual = directorio->archivos;
    while (actual) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return actual;
        }
        actual = actual->siguiente;
    }
    return nullptr;
}

// Función para buscar un subdirectorio
Directorio* buscarDirectorio(Directorio* directorio, const char* nombre) {
    Directorio* actual = directorio->subdirectorios;
    while (actual) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return actual;
        }
        actual = actual->siguienteDirectorio;
    }
    return nullptr;
}

// Función para añadir un archivo a un directorio
void anadirArchivo(Directorio* directorio, Archivo* archivo) {
    if (directorio->archivos == nullptr) {
        directorio->archivos = archivo;
    } else {
        Archivo* ultimo = directorio->archivos;
        while (ultimo->siguiente) {
            ultimo = ultimo->siguiente;
        }
        ultimo->siguiente = archivo;
    }
    archivo->siguiente = nullptr; // Asegurar que el nuevo archivo sea el último en su lista
}

// Función para eliminar un archivo de un directorio
bool removerArchivo(Directorio* directorio, const char* nombre) {
    Archivo* actual = directorio->archivos;
    Archivo* previo = nullptr;

    while (actual) {
        if (strcmp(actual->nombre, nombre) == 0) {
            if (previo) {
                previo->siguiente = actual->siguiente;
            } else {
                directorio->archivos = actual->siguiente;
            }
            eliminarArchivo(actual);
            return true;
        }
        previo = actual;
        actual = actual->siguiente;
    }
    return false;
}

// Función para eliminar un subdirectorio
bool removerDirectorio(Directorio* padre, const char* nombre) {
    Directorio* actual = padre->subdirectorios;
    Directorio* previo = nullptr;
    
    while (actual) {
        if (strcmp(actual->nombre, nombre) == 0) {
            if (previo) {
                previo->siguienteDirectorio = actual->siguienteDirectorio;
            } else {
                padre->subdirectorios = actual->siguienteDirectorio;
            }
            eliminarDirectorio(actual);
            return true;
        }
        previo = actual;
        actual = actual->siguienteDirectorio;
    }
    return false;
}

// Valida nombres de archivos/directorios (no pueden ser . o .. ni contener /)
bool esNombreValido(const char* nombre) {
    if (!nombre || strlen(nombre) == 0) return false;
    if (strcmp(nombre, ".") == 0 || strcmp(nombre, "..") == 0) return false;
    if (strchr(nombre, '/') != nullptr) return false;
    return true;
}

// Añade un directorio a la lista de subdirectorios de un padre
void anadirDirectorioALista(Directorio* padre, Directorio* nuevoDirectorio) {
    if (padre->subdirectorios == nullptr) {
        padre->subdirectorios = nuevoDirectorio;
    } else {
        Directorio* actual = padre->subdirectorios;
        while (actual->siguienteDirectorio) {
            actual = actual->siguienteDirectorio;
        }
        actual->siguienteDirectorio = nuevoDirectorio;
    }
    nuevoDirectorio->padre = padre; // Asegurar el enlace al padre
    nuevoDirectorio->siguienteDirectorio = nullptr; // Asegurar que sea el último
}

// Función auxiliar para navegar una ruta (absoluta o relativa)
// Retorna el directorio al final de la ruta, o nullptr si no se encuentra
Directorio* navegarRuta(Directorio* directorioInicio, const char* ruta, Directorio* raiz) {
    if (!ruta || strlen(ruta) == 0) return directorioInicio;

    char copiaRuta[LONGITUD_MAX_RUTA];
    strncpy(copiaRuta, ruta, LONGITUD_MAX_RUTA - 1);
    copiaRuta[LONGITUD_MAX_RUTA - 1] = '\0';

    Directorio* directorioActualNav = directorioInicio;

    if (ruta[0] == '/') {
        directorioActualNav = raiz;
        if (strlen(ruta) == 1) return raiz;
        strcpy(copiaRuta, ruta + 1);
    }
    
    if (strcmp(copiaRuta, ".") == 0 && ruta[0] != '/') return directorioInicio;

    char* token = strtok(copiaRuta, "/");

    while (token != nullptr) {
        if (strcmp(token, ".") == 0) {
            // No hacer nada
        } else if (strcmp(token, "..") == 0) {
            if (directorioActualNav->padre != nullptr) {
                directorioActualNav = directorioActualNav->padre;
            }
        } else {
            Directorio* siguienteDir = buscarDirectorio(directorioActualNav, token);
            if (siguienteDir == nullptr) {
                return nullptr; // Componente de la ruta no encontrado
            }
            directorioActualNav = siguienteDir;
        }
        token = strtok(nullptr, "/");
    }
    return directorioActualNav;
}

// Obtiene la ruta completa del directorio actual para el prompt
void obtenerRutaCompleta(Directorio* directorio, char* bufferRuta, int tamanoBuffer) {
    if (!directorio) {
        strncpy(bufferRuta, "/", tamanoBuffer);
        bufferRuta[tamanoBuffer - 1] = '\0';
        return;
    }

    char componentesRutaTemp[50][100]; // Max 50 componentes de 100 chars cada uno
    int contadorComponentes = 0;

    Directorio* actual = directorio;
    while (actual != nullptr) {
        if (actual->padre == nullptr && strcmp(actual->nombre, "/") == 0) {
            strncpy(componentesRutaTemp[contadorComponentes++], "/", 100);
        } else {
            strncpy(componentesRutaTemp[contadorComponentes++], actual->nombre, 100);
        }
        actual = actual->padre;
    }

    int posBufferActual = 0;
    if (contadorComponentes > 0 && strcmp(componentesRutaTemp[contadorComponentes - 1], "/") == 0) {
        if (tamanoBuffer > 0) {
            bufferRuta[posBufferActual++] = '/';
        }
        contadorComponentes--;
    }

    for (int i = contadorComponentes - 1; i >= 0; --i) {
        if (posBufferActual > 0 && bufferRuta[posBufferActual - 1] != '/') {
            if (posBufferActual < tamanoBuffer - 1) {
                bufferRuta[posBufferActual++] = '/';
            }
        }
        int len = strlen(componentesRutaTemp[i]);
        if (posBufferActual + len < tamanoBuffer) {
            strcpy(bufferRuta + posBufferActual, componentesRutaTemp[i]);
            posBufferActual += len;
        } else {
            break;
        }
    }
    if (posBufferActual == 0) {
        if (tamanoBuffer > 0) bufferRuta[posBufferActual++] = '/';
    }
    bufferRuta[posBufferActual] = '\0';
}

void imprimirPrompt(Directorio* directorioActual) {
    char bufferRuta[LONGITUD_MAX_RUTA];
    obtenerRutaCompleta(directorioActual, bufferRuta, LONGITUD_MAX_RUTA);
    cout << bufferRuta << " $ ";
}

// --- Implementación de Comandos ---

void comando_cd(Directorio*& directorioActual, const char* ruta, Directorio* raiz) {
    Directorio* directorioDestino = navegarRuta(directorioActual, ruta, raiz);
    if (directorioDestino) {
        directorioActual = directorioDestino;
    } else {
        cout << "cd: " << ruta << ": No existe el archivo o directorio" << endl;
    }
}

void comando_ls(Directorio* directorio) {
    if (!directorio) return;

    // Listar subdirectorios
    Directorio* subDirectorioActual = directorio->subdirectorios;
    while (subDirectorioActual) {
        cout << subDirectorioActual->nombre << "/\t";
        subDirectorioActual = subDirectorioActual->siguienteDirectorio;
    }

    // Listar archivos
    Archivo* archivoActual = directorio->archivos;
    while (archivoActual) {
        cout << archivoActual->nombre << "\t";
        archivoActual = archivoActual->siguiente;
    }
    cout << endl; // Nueva línea al final
}

void comando_mkdir(Directorio* directorioActual, const char* nombre) {
    if (!esNombreValido(nombre)) {
        cout << "mkdir: '" << nombre << "': Nombre de directorio inválido." << endl;
        return;
    }
    if (buscarDirectorio(directorioActual, nombre) || buscarArchivo(directorioActual, nombre)) {
        cout << "mkdir: '" << nombre << "': El archivo ya existe" << endl;
        return;
    }
    Directorio* nuevoDirectorio = crearDirectorio(nombre, directorioActual);
    if (nuevoDirectorio) {
        anadirDirectorioALista(directorioActual, nuevoDirectorio);
    } else {
        cout << "mkdir: Error al crear el directorio." << endl;
    }
}

void comando_rm(Directorio* directorioActual, const char* rutaAEliminar, Directorio* raiz) {
    if (!rutaAEliminar || strlen(rutaAEliminar) == 0) {
        cout << "rm: falta un operando" << endl;
        cout << "Uso: rm <ruta_archivo_o_directorio>" << endl;
        return;
    }

    char copiaRuta[LONGITUD_MAX_RUTA];
    strncpy(copiaRuta, rutaAEliminar, LONGITUD_MAX_RUTA - 1);
    copiaRuta[LONGITUD_MAX_RUTA - 1] = '\0';

    char* ultimaBarra = strrchr(copiaRuta, '/');
    char* nombreElemento;
    char rutaPadre[LONGITUD_MAX_RUTA];
    rutaPadre[0] = '\0';

    Directorio* directorioPadreDestino = nullptr;

    if (ultimaBarra == nullptr) { // Es un nombre en el directorio actual
        nombreElemento = copiaRuta;
        directorioPadreDestino = directorioActual;
    } else { // Es una ruta (absoluta o relativa)
        *ultimaBarra = '\0';
        nombreElemento = ultimaBarra + 1;

        if (strlen(copiaRuta) == 0) {
            strcpy(rutaPadre, "/");
        } else {
            strcpy(rutaPadre, copiaRuta);
        }
        
        directorioPadreDestino = navegarRuta(directorioActual, rutaPadre, raiz);
    }

    if (!directorioPadreDestino) {
        cout << "rm: no se puede eliminar '" << rutaAEliminar << "': No existe el archivo o directorio" << endl;
        return;
    }

    if (directorioPadreDestino == raiz && strcmp(nombreElemento, "") == 0 && strcmp(rutaAEliminar, "/") == 0) {
        cout << "rm: no se puede eliminar '/': Es un directorio" << endl;
        return;
    }
    if (strcmp(nombreElemento, ".") == 0 || strcmp(nombreElemento, "..") == 0) {
        cout << "rm: no se puede eliminar '" << nombreElemento << "': Permiso denegado" << endl;
        return;
    }

    if (removerArchivo(directorioPadreDestino, nombreElemento)) {
        cout << "rm: '" << nombreElemento << "' eliminado." << endl;
    }
    else if (removerDirectorio(directorioPadreDestino, nombreElemento)) {
        cout << "rm: '" << nombreElemento << "' eliminado (incluyendo su contenido)." << endl;
    }
    else {
        cout << "rm: no se puede eliminar '" << rutaAEliminar << "': No existe el archivo o directorio" << endl;
    }
}

void comando_touch(Directorio* directorioActual, const char* nombre) {
    if (!esNombreValido(nombre)) {
        cout << "touch: '" << nombre << "': Nombre de archivo inválido." << endl;
        return;
    }
    if (buscarDirectorio(directorioActual, nombre)) {
        cout << "touch: '" << nombre << "': Es un directorio" << endl;
        return;
    }
    if (buscarArchivo(directorioActual, nombre)) {
        cout << "touch: '" << nombre << "': El archivo ya existe. No se realizó ninguna acción." << endl;
        return;
    }

    Archivo* nuevoArchivo = crearArchivo(nombre);
    if (nuevoArchivo) {
        anadirArchivo(directorioActual, nuevoArchivo);
        cout << "touch: '" << nombre << "' creado." << endl;
    } else {
        cout << "touch: Error al crear el archivo." << endl;
    }
}

void comando_editar(Archivo* archivo) {
    if (!archivo) {
        cout << "editar: No hay archivo válido para editar." << endl;
        return;
    }

    cout << "--- Editando: " << archivo->nombre << " ---" << endl;
    cout << "Contenido actual:\n" << (archivo->contenido ? archivo->contenido : "(vacío)") << endl;
    cout << "Ingrese nuevo contenido (presione Enter en una línea vacía para finalizar):\n";

    char bufferLinea[LONGITUD_MAX_CONTENIDO];
    char bufferNuevoContenido[LONGITUD_MAX_CONTENIDO * 2];
    bufferNuevoContenido[0] = '\0';
    int longitudActual = 0;

    // Limpiar el buffer de entrada antes de leer líneas
    cin.clear();
    // Consumir cualquier caracter de nueva línea pendiente
    if (cin.peek() == '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }


    while (cin.getline(bufferLinea, sizeof(bufferLinea))) {
        if (strlen(bufferLinea) == 0) {
            break;
        }
        
        // Verificar si hay espacio suficiente antes de concatenar
        if (longitudActual + strlen(bufferLinea) + 1 + 1 > sizeof(bufferNuevoContenido)) {
            cout << "Advertencia: Contenido demasiado largo, se truncará." << endl;
            break;
        }
        strcat(bufferNuevoContenido, bufferLinea);
        strcat(bufferNuevoContenido, "\n");
        longitudActual += strlen(bufferLinea) + 1;
    }

    if (archivo->contenido) {
        delete[] archivo->contenido;
    }
    if (longitudActual > 0) {
        // Asegúrate de que el último caracter no sea un salto de línea si el usuario terminó con una línea vacía
        // Y el bufferNuevoContenido ya tiene un '\n' adicional del strcat, ajustamos
        if (bufferNuevoContenido[longitudActual-1] == '\n' && longitudActual > 0) {
            bufferNuevoContenido[longitudActual-1] = '\0'; // Elimina el último salto de línea
            longitudActual--;
        }
        archivo->contenido = new char[longitudActual + 1];
        strcpy(archivo->contenido, bufferNuevoContenido);
    } else {
        archivo->contenido = nullptr;
    }

    cout << "Contenido de '" << archivo->nombre << "' actualizado." << endl;
}

void comando_renombrar(Directorio* directorioActual, const char* nombreAntiguo, const char* nombreNuevo) {
    if (!esNombreValido(nombreNuevo)) {
        cout << "renombrar: '" << nombreNuevo << "': Nuevo nombre inválido." << endl;
        return;
    }
    if (strcmp(nombreAntiguo, ".") == 0 || strcmp(nombreAntiguo, "..") == 0) {
        cout << "renombrar: No se pueden renombrar los directorios especiales '.' o '..'." << endl;
        return;
    }

    if (buscarArchivo(directorioActual, nombreNuevo) || buscarDirectorio(directorioActual, nombreNuevo)) {
        cout << "renombrar: '" << nombreNuevo << "': El archivo ya existe" << endl;
        return;
    }

    Archivo* archivoARenombrar = buscarArchivo(directorioActual, nombreAntiguo);
    if (archivoARenombrar) {
        delete[] archivoARenombrar->nombre;
        archivoARenombrar->nombre = new char[strlen(nombreNuevo) + 1];
        strcpy(archivoARenombrar->nombre, nombreNuevo);
        cout << "Archivo '" << nombreAntiguo << "' renombrado a '" << nombreNuevo << "'." << endl;
        return;
    }

    Directorio* directorioARenombrar = buscarDirectorio(directorioActual, nombreAntiguo);
    if (directorioARenombrar) {
        delete[] directorioARenombrar->nombre;
        directorioARenombrar->nombre = new char[strlen(nombreNuevo) + 1];
        strcpy(directorioARenombrar->nombre, nombreNuevo);
        cout << "Directorio '" << nombreAntiguo << "' renombrado a '" << nombreNuevo << "'." << endl;
        return;
    }

    cout << "renombrar: '" << nombreAntiguo << "': No existe el archivo o directorio" << endl;
}

// --- Carga Inicial del Sistema de Archivos ---

Directorio* cargarSistemaArchivos(const char* nombreArchivo, Directorio*& raiz) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de configuración del sistema de archivos: " << nombreArchivo << endl;
        return nullptr;
    }

    if (!raiz) {
        raiz = crearDirectorio("/");
        if (!raiz) {
            cerr << "Error: No se pudo crear el directorio raíz." << endl;
            return nullptr;
        }
    }

    char bufferLinea[LONGITUD_MAX_RUTA + LONGITUD_MAX_CONTENIDO];
    while (archivo.getline(bufferLinea, sizeof(bufferLinea))) {
        char comando[5];
        char bufferRutaStr[LONGITUD_MAX_RUTA];
        
        // Usar sscanf_s para mayor seguridad en Windows, si no, sscanf normal
        #ifdef _MSC_VER
            if (sscanf_s(bufferLinea, "%s %s", comando, (unsigned int)sizeof(comando), bufferRutaStr, (unsigned int)sizeof(bufferRutaStr)) < 2) {
        #else
            if (sscanf(bufferLinea, "%s %s", comando, bufferRutaStr) < 2) {
        #endif
            cerr << "Advertencia: Línea mal formada en el archivo de configuración: " << bufferLinea << endl;
            continue;
        }

        if (strcmp(comando, "DIR") == 0) {
            char* ultimaBarra = strrchr(bufferRutaStr, '/');
            if (ultimaBarra == nullptr) {
                    cerr << "Advertencia: Formato de ruta DIR inválido: " << bufferRutaStr << endl;
                    continue;
            }
            
            char nombreDir[100];
            strcpy(nombreDir, ultimaBarra + 1);
            *ultimaBarra = '\0'; // Corta la ruta para obtener solo el padre

            char rutaPadre[LONGITUD_MAX_RUTA];
            if (strlen(bufferRutaStr) == 0) { // Si la ruta original era solo "/nombre", padre es "/"
                strcpy(rutaPadre, "/");
            } else {
                strcpy(rutaPadre, bufferRutaStr);
            }

            Directorio* padreParaNuevoDir = navegarRuta(raiz, rutaPadre, raiz);

            if (padreParaNuevoDir) {
                if (buscarDirectorio(padreParaNuevoDir, nombreDir) || buscarArchivo(padreParaNuevoDir, nombreDir)) {
                    // Advertencia: El directorio ya existe, se omite.
                } else {
                    Directorio* nuevoDir = crearDirectorio(nombreDir);
                    if (nuevoDir) {
                        anadirDirectorioALista(padreParaNuevoDir, nuevoDir);
                    }
                }
            } else {
                cerr << "Error: No se encontró el directorio padre para DIR: " << rutaPadre << endl;
            }

        } else if (strcmp(comando, "FILE") == 0) {
            char* ultimaBarra = strrchr(bufferRutaStr, '/');
            if (ultimaBarra == nullptr) {
                cerr << "Advertencia: Formato de ruta FILE inválido: " << bufferRutaStr << endl;
                continue;
            }
            char nombreArchivo[100];
            strcpy(nombreArchivo, ultimaBarra + 1);
            *ultimaBarra = '\0'; // Corta la ruta para obtener solo el padre

            char rutaPadre[LONGITUD_MAX_RUTA];
            if (strlen(bufferRutaStr) == 0) { // Si la ruta original era solo "/nombre", padre es "/"
                strcpy(rutaPadre, "/");
            } else {
                strcpy(rutaPadre, bufferRutaStr);
            }

            char* inicioContenido = strstr(bufferLinea, bufferRutaStr);
            if (inicioContenido) {
                inicioContenido += strlen(bufferRutaStr); // Mueve el puntero después de la ruta
                while (*inicioContenido == ' ') inicioContenido++; // Ignora espacios en blanco
            }
            
            const char* contenido = (inicioContenido && *inicioContenido != '\0') ? inicioContenido : nullptr;

            Directorio* padreParaNuevoArchivo = navegarRuta(raiz, rutaPadre, raiz);

            if (padreParaNuevoArchivo) {
                if (buscarDirectorio(padreParaNuevoArchivo, nombreArchivo) || buscarArchivo(padreParaNuevoArchivo, nombreArchivo)) {
                    // Advertencia: El archivo ya existe, se omite.
                } else {
                    Archivo* nuevoArchivo = crearArchivo(nombreArchivo, contenido);
                    if (nuevoArchivo) {
                        anadirArchivo(padreParaNuevoArchivo, nuevoArchivo);
                    }
                }
            } else {
                cerr << "Error: No se encontró el directorio padre para FILE: " << rutaPadre << endl;
            }
        } else {
            cerr << "Advertencia: Comando desconocido en el archivo de configuración: " << comando << endl;
        }
    }
    archivo.close();
    cout << "Sistema de archivos inicial cargado desde '" << nombreArchivo << "'." << endl;
    return raiz;
}

// --- ESTRUCTURA DE PILA MANUAL ---
// Definimos una estructura para cada elemento de nuestra pila
struct StackNode {
    Directorio* dir;
    char* path; // Ruta completa hasta este directorio
};

// Una pila simple implementada con un array dinámico
const int MAX_STACK_SIZE = 1000; // Un tamaño máximo razonable para la pila
StackNode customStack[MAX_STACK_SIZE];
int stackTop = -1; // Índice del tope de la pila, -1 indica vacía

void push(Directorio* d, char* p) {
    if (stackTop < MAX_STACK_SIZE - 1) {
        stackTop++;
        customStack[stackTop].dir = d;
        customStack[stackTop].path = p;
    } else {
        cout << "Error: Desbordamiento de pila en guardarSistemaArchivos." << endl;
        // Considerar manejo de errores o redimensionamiento del array
    }
}

StackNode pop() {
    if (stackTop >= 0) {
        return customStack[stackTop--];
    }
    // Deberías manejar este caso de error si la pila está vacía y se intenta pop
    // Para este contexto, se asume que solo se llama pop si la pila no está vacía
    return {nullptr, nullptr}; 
}

bool isEmpty() {
    return stackTop == -1;
}

// --- FIN ESTRUCTURA DE PILA MANUAL ---

// Función para guardar el sistema de archivos a un archivo de texto
void guardarSistemaArchivos(const char* nombreArchivo, Directorio* raiz) {
    ofstream archivoSalida(nombreArchivo);
    if (!archivoSalida.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para guardar el sistema de archivos: " << nombreArchivo << endl;
        return;
    }

    // Reiniciar la pila para un nuevo guardado
    stackTop = -1; 
    char* rutaRaiz = new char[2]; // Asignar dinámicamente para la raíz también
    strcpy(rutaRaiz, "/");
    push(raiz, rutaRaiz); // Empieza el recorrido desde la raíz

    while (!isEmpty()) { // Mientras haya elementos en la pila, sigue recorriendo
        StackNode currentNode = pop(); // Obtiene el elemento del tope
        Directorio* actualDir = currentNode.dir;
        char* rutaActual = currentNode.path;
        
        // Guardar directorios
        Directorio* subDir = actualDir->subdirectorios;
        while (subDir) {
            char nuevaRuta[LONGITUD_MAX_RUTA];
            if (strcmp(rutaActual, "/") == 0) {
                snprintf(nuevaRuta, LONGITUD_MAX_RUTA, "/%s", subDir->nombre);
            } else {
                snprintf(nuevaRuta, LONGITUD_MAX_RUTA, "%s/%s", rutaActual, subDir->nombre);
            }
            archivoSalida << "DIR " << nuevaRuta << endl;
            
            // Crea una copia dinámica de la ruta para el elemento de la pila
            char* rutaParaPila = new char[strlen(nuevaRuta) + 1];
            strcpy(rutaParaPila, nuevaRuta);
            push(subDir, rutaParaPila); // Agrega el subdirectorio a la pila para visitarlo luego
            subDir = subDir->siguienteDirectorio;
        }

        // Guardar archivos
        Archivo* file = actualDir->archivos;
        while (file) {
            char rutaCompletaArchivo[LONGITUD_MAX_RUTA];
            if (strcmp(rutaActual, "/") == 0) {
                snprintf(rutaCompletaArchivo, LONGITUD_MAX_RUTA, "/%s", file->nombre);
            } else {
                snprintf(rutaCompletaArchivo, LONGITUD_MAX_RUTA, "%s/%s", rutaActual, file->nombre);
            }

            archivoSalida << "FILE " << rutaCompletaArchivo;
            if (file->contenido) {
                archivoSalida << " " << file->contenido;
            }
            archivoSalida << endl;
            file = file->siguiente;
        }

        // Libera la memoria de la ruta que sacamos de la pila
        delete[] rutaActual;
    }

    archivoSalida.close();
    cout << "Sistema de archivos guardado en '" << nombreArchivo << "'." << endl;
}


// --- Bucle Principal de la Terminal ---

void procesarComando(char* lineaComando, Directorio*& directorioActual, Directorio* raiz, const char* nombreArchivoGuardado) {
    char* token = strtok(lineaComando, " ");

    if (token == nullptr) return;

    if (strcmp(token, "cd") == 0) {
        char* ruta = strtok(nullptr, " ");
        if (ruta) {
            comando_cd(directorioActual, ruta, raiz);
        } else {
            cout << "cd: falta un operando" << endl;
            cout << "Uso: cd <ruta_directorio>" << endl;
        }
    }
    else if (strcmp(token, "ls") == 0) {
        char* ruta = strtok(nullptr, " ");
        if (ruta == nullptr) {
            comando_ls(directorioActual);
        } else {
            Directorio* directorioDestino = navegarRuta(directorioActual, ruta, raiz);
            if (directorioDestino) {
                comando_ls(directorioDestino);
            } else {
                cout << "ls: no se puede acceder a '" << ruta << "': No existe el archivo o directorio" << endl;
            }
        }
    }
    else if (strcmp(token, "mkdir") == 0) {
        char* nombre = strtok(nullptr, " ");
        if (nombre) {
            comando_mkdir(directorioActual, nombre);
        } else {
            cout << "mkdir: falta un operando" << endl;
            cout << "Uso: mkdir <nombre_carpeta>" << endl;
        }
    }
    else if (strcmp(token, "rm") == 0) {
        char* rutaAEliminar = strtok(nullptr, " ");
        if (rutaAEliminar) {
            comando_rm(directorioActual, rutaAEliminar, raiz);
        } else {
            cout << "rm: falta un operando" << endl;
            cout << "Uso: rm <ruta_archivo_o_directorio>" << endl;
        }
    }
    else if (strcmp(token, "touch") == 0) {
        char* nombre = strtok(nullptr, " ");
        if (nombre) {
            comando_touch(directorioActual, nombre);
        } else {
            cout << "touch: falta un operando" << endl;
            cout << "Uso: touch <nombre_archivo>" << endl;
        }
    }
    else if (strcmp(token, "edit") == 0) {
        char* nombreArchivo = strtok(nullptr, " ");
        if (nombreArchivo) {
            Archivo* archivoAEditar = buscarArchivo(directorioActual, nombreArchivo);
            if (archivoAEditar) {
                comando_editar(archivoAEditar);
            } else {
                cout << "editar: '" << nombreArchivo << "': No existe tal archivo" << endl;
            }
        } else {
            cout << "editar: falta un operando" << endl;
            cout << "Uso: editar <nombre_archivo>" << endl;
        }
    }
    else if (strcmp(token, "rename") == 0) {
        char* nombreAntiguo = strtok(nullptr, " ");
        char* nombreNuevo = strtok(nullptr, " ");
        if (nombreAntiguo && nombreNuevo) {
            comando_renombrar(directorioActual, nombreAntiguo, nombreNuevo);
        } else {
            cout << "renombrar: falta un operando" << endl;
            cout << "Uso: renombrar <nombre_antiguo> <nombre_nuevo>" << endl;
        }
    }
    else if (strcmp(token, "save") == 0) {
        guardarSistemaArchivos(nombreArchivoGuardado, raiz);
    }
    else if (strcmp(token, "exit") == 0) {
        cout << "Saliendo de la terminal." << endl;
        guardarSistemaArchivos(nombreArchivoGuardado, raiz); // Guardar antes de salir
        eliminarDirectorio(raiz); // Liberar memoria al salir
        exit(0);    
    }
    else {
        cout << lineaComando << ": comando no encontrado" << endl;
    }
}

int main() {
    Directorio* raiz = nullptr;
    Directorio* directorioActual = nullptr;
    const char* nombreArchivoConfig = "filesystem.txt";

    raiz = cargarSistemaArchivos(nombreArchivoConfig, raiz);

    if (!raiz) {
        cout << "Error al inicializar el sistema de archivos. Saliendo." << endl;
        return 1;
    }

    directorioActual = raiz;

    char lineaComando[LONGITUD_MAX_RUTA + LONGITUD_MAX_CONTENIDO];

    while (true) {
        imprimirPrompt(directorioActual);
        cin.getline(lineaComando, sizeof(lineaComando));
        
        char copiaLineaComando[sizeof(lineaComando)];
        strcpy(copiaLineaComando, lineaComando);

        procesarComando(copiaLineaComando, directorioActual, raiz, nombreArchivoConfig);
    }

    return 0;
}
//balatro 2
