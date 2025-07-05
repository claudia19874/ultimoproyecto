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

struct Directory {
    char* name;           
    Directory* parent;    // Directorio padre
    Directory* childDir;  // Lista de subdirectorios 
    Directory* nextDir;   
    File* files;          // Lista de archivos 
};

// Función para crear un nuevo archivo
File* createFile(const char* name, const char* content = nullptr) {
    File* newFile = new File;
    newFile->name = new char[strlen(name) + 1];
    strcpy(newFile->name, name);
    
    if (content) {
        newFile->content = new char[strlen(content) + 1];
        strcpy(newFile->content, content);
    } else {
        newFile->content = nullptr;
    }
    
    newFile->next = nullptr;
    return newFile;
}

// Función para crear un nuevo directorio
Directory* createDirectory(const char* name, Directory* parent = nullptr) {
    Directory* newDir = new Directory;
    newDir->name = new char[strlen(name) + 1];
    strcpy(newDir->name, name);
    
    newDir->parent = parent;
    newDir->childDir = nullptr;
    newDir->nextDir = nullptr;
    newDir->files = nullptr;
    
    return newDir;
}
