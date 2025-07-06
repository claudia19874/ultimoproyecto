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

// Función para liberar memoria de un archivo
void deleteFile(File* file) {
    if (file) {
        delete[] file->name;
        if (file->content) delete[] file->content;
        delete file;
    }
}

// Función para liberar memoria de un directorio y su contenido
void deleteDirectory(Directory* dir) {
    if (dir) {
        delete[] dir->name;
        
        // Eliminar subdirectorios
        Directory* currentChild = dir->childDir;
        while (currentChild) {
            Directory* nextChild = currentChild->nextDir;
            deleteDirectory(currentChild);
            currentChild = nextChild;
        }
        
        // Eliminar archivos
        File* currentFile = dir->files;
        while (currentFile) {
            File* nextFile = currentFile->next;
            deleteFile(currentFile);
            currentFile = nextFile;
        }
        
        delete dir;
    }
}


//encontrar archivo en un directorio
File* findFile(Directory* dir, const char* name) {
    File* current = dir->files;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}


// Función para buscar un subdirectorio
Directory* findDirectory(Directory* dir, const char* name) {
    Directory* current = dir->childDir;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->nextDir;
    }
    return nullptr;
}

// Función para añadir un archivo a un directorio
void addFile(Directory* dir, File* file) {
    if (dir->files == nullptr) {
        dir->files = file;
    } else {
        File* last = dir->files;
        while (last->next) {
            last = last->next;
        }
        last->next = file;
    }
    file->next = nullptr;
}
