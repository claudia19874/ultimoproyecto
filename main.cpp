#prueba
#include <iostream> 
#include <fstream>
#include <cstring>
using namespace std;

//estructura para archivos
struct File {
    char* name; // Nombre del archivo
    char* content; // Contenido del archivo
    File* next; // Puntero al siguiente archivo
};
