

    #include <stdio.h> 
    #include <stdlib.h> 
    #include <string.h> 
    #include <time.h> 
    #include <sys/stat.h> 
    #include <errno.h> 

    #define MAX_BLOCK_SIZE 512 
    #define MIN_FILE_SIZE 32768 
    #define MIN_FILE_NAME_LEN 12 
    #define MAX_FILE_NAME_LEN 19 
   
    void createFile(char *filename, char *parent,char*content); 
    void deleteFile(char* filename); 
    int openFile(char* filename); 
    int closeFile(char* filename); 
    void createDirectory(char* dirname, char* parent); 
    void deleteDirectory(char* dirname); 
    void readFile(char* filename); 
    void printFileContents(char* filename,int id); 
    void writeToFile(char* filename); 
    void printDirHierarchy(); 
    void printDirContent(char* dirname,int sockfd); 
    char** parseCommand(char* command, char** argv, int *argc);

    char* getTime(); 
    int findEmptyEntryFAT(); 
    int findEmptyBlock(); 
    int printNumberOfAvailableBlocks();
    int findParent(char* parent);
    void insertEntry(int emptyIndex, char* filename, char* fileOrDir, char* indexNumber, char* parent, char* timestamp); 

