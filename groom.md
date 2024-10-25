# 

## Create

- nombre de archivo
- listas de archivos

dado los dos argumentos anteriores crea un archivo con nombre de archivo y en el guarda la lista de archivos

por cada archivo lo abré y


# estructura de archivo en disco

## tabla de archivos

- nombre de archivo, maximo 100 bytes
- número del primer bloque, int
```
typedef struct {
    char name[100];
    int first; // num of the first block
} FileHeader;

typedef struct {
    FileHeader files[250];
    FileHeader freeBlock;
} FileTable;
```

## file

creo que esto no es necesario que esté instanceado

```
typedef struct FileBlock
{
    char data[BLOCK_SIZE];
    size_t size;
    struct FileBlock *next;
    int blockNum; // unique block number
} FileBlock;

typedef struct
{
    struct FileBlock *head;
    struct FileBlock *tail;
} File;
```

