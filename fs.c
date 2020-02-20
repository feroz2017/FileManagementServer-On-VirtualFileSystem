#include "fs.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <unistd.h>

#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

char *FAT[100][5]; 
char physicalDir[100][MAX_BLOCK_SIZE];
char open_file_array[100][MAX_FILE_NAME_LEN];
FILE *disk_ptr;        
int startingIndex = 0; 
int x = 0;             
char **argv;           
int emptyIndex;
int emptyBlock;
char content[512];



#define MAX_CLIENTS 100
#define BUFFER_SZ 2048

int cli_count = 0;
static int uid = 10;


typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void ReplaceNewlineWithNull (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { 
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}


//net_ntoa() alternative
void printClientAddress(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}


void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg){
    char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;
	cli_count++;
	client_t *cli = (client_t *)arg;
	

	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} else{
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		send_message(buff_out, cli->uid);
	
	}

	
	sprintf(buff_out, "%s" , "We Implemented this FAT file data strcutrue\n");
	write(cli->sockfd,buff_out,strlen(buff_out));

	sprintf(buff_out, "%s" , "If You want specify a directory You can do it by specifying third Arguemnt:\n");
	write(cli->sockfd,buff_out,strlen(buff_out));
	
	sprintf(buff_out, "%s" , "Create a file with: create 'filename' and include its extension\n");
	write(cli->sockfd,buff_out,strlen(buff_out));
	
	sprintf(buff_out, "%s" , "Delete a file with: delete 'filename' and include its extension\n");
	write(cli->sockfd,buff_out,strlen(buff_out));
	
	sprintf(buff_out, "%s" , "Create a directory with: createDir 'directoryname\n");
	write(cli->sockfd,buff_out,strlen(buff_out));
	
	sprintf(buff_out, "%s" , "Delete a directory with: deleteDir 'directoryname'\n");
	write(cli->sockfd,buff_out,strlen(buff_out));
	
	sprintf(buff_out, "%s" , "Print the contents of a file with: cat 'filename'\n");
	write(cli->sockfd,buff_out,strlen(buff_out));


	sprintf(buff_out, "%s" , "Print the current contents of a directory with: printDir 'directoryname'\n");
	write(cli->sockfd,buff_out,strlen(buff_out));

	sprintf(buff_out, "%s" , "Print the directory hierarchy with: printHierarchy\n\n");
	write(cli->sockfd,buff_out,strlen(buff_out));

	bzero(buff_out, BUFFER_SZ);

        int content_flag = 0;
        char line[100];
        char *argv[8]={0};
        int argc;
	while(1){
		if (leave_flag) {
			break;
		}
        
		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
    


		if (receive > 0){
			if(strlen(buff_out) > 0){
				if(!content_flag){
                    send_message(buff_out, cli->uid);
			        ReplaceNewlineWithNull(buff_out, strlen(buff_out));
			        printf("%s\n", buff_out);  
                    strcpy(line,buff_out);
                  //  argv = parseCommand(line, argv, &argc); 
                                          char * token;
		                int i=0;
   		                char* strSplit = line;
   		                while ((token = strtok_r(strSplit , " ", &strSplit)))
       			        argv[i++] = token;
		                argc = i;
                        i=0;
                        fflush(stdin);
                   
                }
                
                if (strcmp(line, "create") == 0 )
                { 
                          pthread_mutex_lock(&clients_mutex);
                        content_flag = !content_flag ? 1:0;
                        pthread_mutex_unlock(&clients_mutex);

                        if(content_flag==1){
                            char prompt[100];
                            sprintf(prompt,"%s","Enter Content:");
                            write(cli->sockfd,prompt,strlen(prompt));
                            bzero(buff_out, BUFFER_SZ);
                            continue;
                        }
                    if (argc == 3)
                        { 
                            pthread_mutex_lock(&clients_mutex);
                            printf("\nTESTEST\n");
                            printf("\nFile %s\n",argv[1]);
                            printf("\nFolder %s\n",argv[2]);
                            createFile(argv[1],argv[2],buff_out);
                            pthread_mutex_unlock(&clients_mutex);
                        }
                    else if (argc == 2)
                        { 
                            pthread_mutex_lock(&clients_mutex);
                            createFile(argv[1],"/",buff_out);
                            pthread_mutex_unlock(&clients_mutex);
                        }
                     else
                    {                                                      
                        printf("An error occured creating your file.. try again!\n"); 
                    }

                }
                else if (strcmp(line, "createDir") == 0)
                {      

                    if (argc == 3)
                    { 
                        pthread_mutex_lock(&clients_mutex);
                        createDirectory(argv[1], argv[2]);
                        pthread_mutex_unlock(&clients_mutex);

                     }
                     else if (argc == 2)
                    { 
                        pthread_mutex_lock(&clients_mutex);
                        createDirectory(argv[1], "/");
                        pthread_mutex_unlock(&clients_mutex);
                    }
                    else
                    {                                                           
                    printf("An error occured creating your directory..\n"); 
                    }
                } 
                else if (strcmp(line, "delete") == 0)
                { 
                      
                    
                    pthread_mutex_lock(&clients_mutex);
                    deleteFile(argv[1]);
                    pthread_mutex_unlock(&clients_mutex);
                }
                else if (strcmp(line, "deleteDir") == 0)
                { 
                    
                    pthread_mutex_lock(&clients_mutex);
                    deleteDirectory(argv[1]);
                    pthread_mutex_unlock(&clients_mutex);
                }
                else if (strcmp(line, "cat") == 0)
                { 
                    
                   // pthread_mutex_lock(&clients_mutex);
                    printFileContents(argv[1],cli->sockfd);
                   // pthread_mutex_unlock(&clients_mutex);
                }
                else if (strcmp(line, "printHierarchy") == 0)
                {
                    
                    pthread_mutex_lock(&clients_mutex);
                    printDirHierarchy(); 
                    pthread_mutex_unlock(&clients_mutex);
                }
                else if (strcmp(line, "printDir") == 0)
                { 
                    
                    //pthread_mutex_lock(&clients_mutex);
                    
                    printDirContent(argv[1],cli->sockfd);
                    
                    //pthread_mutex_unlock(&clients_mutex);
                } 
                else if (strcmp(line, "cat") == 0)
                {
                    
                    pthread_mutex_lock(&clients_mutex);
                    printFileContents(argv[1],cli->sockfd);
                    pthread_mutex_unlock(&clients_mutex);
                }  
                else
                {
                    sprintf(buff_out,"%s","Enter Valid command");
                    write(cli->sockfd,buff_out,strlen(buff_out));
                }
			}
		}

  else if (receive == 0 || strcmp(buff_out, "exit") == 0){
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}


		bzero(buff_out, BUFFER_SZ);
	}

	close(cli->sockfd);
  queue_remove(cli->uid);
  free(cli);
  cli_count--;
  pthread_detach(pthread_self());

	return NULL;

}



int main(int argc, char **argv)
{
char line[100]; 
    char *fatCols = "File Name | FileOrDir | Index Number | Parent | Date and Time\n";

    if (argc >= 2 && argc < 4)
    { 
          struct stat stat_record; 
        if (stat(argv[1], &stat_record))
        {                                                                                      
            printf("%s\n", strerror(errno));                                                    
            printf("Please enter a valid filename within this directory to use next time..\n"); 
            exit(0);                                                                            
        }
        else if (stat_record.st_size <= 1)
        {                                    
            printf("Empty virtual disk! Creating root directory..\n\n"); 

            disk_ptr = fopen(argv[1], "a"); 
            if (!disk_ptr)
            {                                     
                printf("Failed to open disk!\n"); 
                exit(0);                          
            }
           
            int i, j = 0;
            for (i = 0; i < 100; i++)
            {
                for (j = 0; j < 5; j++)
                {
                     for (i = startingIndex; i < 100; i++)
                    {
                        for (j = 0; j < 5; j++)
                        {
                            FAT[i][j] = ".";             
                            strcpy(physicalDir[i], "."); 
                   
                        }
               
                    }  
                    FAT[i][j] = ".";             
                    strcpy(physicalDir[i], "."); 
                }
                
            }

            fputs(fatCols, disk_ptr);     
            createDirectory("/", "none"); 
            for (i = startingIndex; i < 100; i++)
            {
                for (j = 0; j < 5; j++)
                {
                    FAT[i][j] = ".";             
                    strcpy(physicalDir[i], "."); 
                   
                }
               
            }
        }
        else
        {                                                            
            
            printf("Virtual disk has content already stored within it!\n"); 
            disk_ptr = fopen(argv[1], "a+"); 
              if (!disk_ptr)
            {                                     
                printf("Failed to open disk!\n"); 
                exit(0);                          
            }
        

            char fileLine[2048];                         
            rewind(disk_ptr);                            
            fgets(fileLine, sizeof(fileLine), disk_ptr); 

            while (!feof(disk_ptr)) 
            {
                fgets(fileLine, sizeof(fileLine), disk_ptr);
               
                char *rest;                            
                char *token;                           
                x = 0;                                 
                
                for (token = strtok_r(fileLine, "|\t\r\a", &rest), x = 0; ((token != NULL) && (x < 5)); token = strtok_r(NULL, "|\t\r\a", &rest), x++) 
                {
                    FAT[startingIndex][x] = (char *)malloc(sizeof(char) * sizeof(fileLine)); 
                    strcpy(FAT[startingIndex][x], token);                                    
                 
                }
                
                startingIndex++; 
                
            }

            
            int i, j = 0;
            for (i = startingIndex; i < 100; i++)
            {
                for (j = 0; j < 5; j++)
                {
                    FAT[i][j] = ".";             
                    strcpy(physicalDir[i], "."); 
                   
                }
               
            }

            
        }


       
    }
     
     	if(argc != 3){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
    int port = atoi(argv[2]);
    int option = 1;
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

 
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

 
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}


  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR: Socket binding failed");
    return EXIT_FAILURE;
  }

 
  if (listen(listenfd, 10) < 0) {
    perror("ERROR: Socket listening failed");
    return EXIT_FAILURE;
	}

	printf("=== WELCOME TO Virtual File System Via as a Server ===\n");

	while(1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

	
		if((cli_count + 1) == MAX_CLIENTS){
			printf("Max clients reached. Rejected: ");
			printClientAddress(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

	
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

	
		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

	
		
	}

	return EXIT_SUCCESS;


     printf("Yes Hurrah!\n");

        return 0;
    }
   

void createFile(char *filename, char *parent ,char *content)
{
  
    if ((!filename) || (!parent)) { 
        printf("Error: Either no filename name was specified or no parent directory was specified..try again!\n\n");
        return;
    }

    
    printf("Check#2");
    
    if((strstr(filename, ".txt")) || strstr(filename, ".")) { 
    printf("Check#3");
        if (findParent(parent)) 
        {
            printf("Check#4");
            
            emptyIndex = findEmptyEntryFAT();
          
            
            emptyBlock = findEmptyBlock();
            
            
            
           
            
            char indexNumber[sizeof(int) * 4 + 1]; 
            sprintf(indexNumber, "%d", emptyBlock);
            
            
            insertEntry(emptyIndex, filename, "file", indexNumber, parent, getTime());
            startingIndex++;

            

            strcpy(physicalDir[emptyBlock], content); 
          
        }
        else
        {
            printf("Parent for file you wish to create was not found! Try again!\n\n");
        }
    } else {
        printf("Please specify an extension for the file to be created..\n\n");
        return;
    }
}



void deleteFile(char *filename)
{
    printf("Deleting %s from file system..\n\n", filename);
    int i = 0;
    int j = 0;
    startingIndex = 0;
    disk_ptr = fopen("vsdisk", "r+"); 
    for (i = 0; i < 100; i++)
    {                                                                                     
        char *temp = (char *)malloc(sizeof(char) * strlen(FAT[i][0]));
        temp = strdup(FAT[i][0]);      
        startingIndex++; 
        if (strcmp(temp, filename) == 0)
        { 
            rewind(disk_ptr);                                   
            char *newLine = (char *)malloc(sizeof(char) * 140);
            while (startingIndex >= 0)
            {                                              
                fgets(newLine, sizeof(newLine), disk_ptr); 
                startingIndex--;                           
            }
            
            for (j = 0; j < 5; j++)
            {               
                strcpy(FAT[i][j], ".");    
            }
            break; 
        }
    }
}






void createDirectory(char *dirname, char *parent)
{
    if (strcmp(dirname, "/") == 0) 
    {
        insertEntry(0, dirname, "dir", "0", "none", getTime());
        startingIndex++;
    }
    else
    {
        
        int emptyIndex = findEmptyEntryFAT();
        
        
        int emptyBlock = findEmptyBlock();
     
        
        char indexNumber[sizeof(int) * 4 + 1];
        sprintf(indexNumber, "%d", emptyBlock);
        
        
        insertEntry(emptyIndex, dirname, "dir", indexNumber, parent, getTime());

        
    }
}


void deleteDirectory(char *dirname)
{
    printf("Deleting %s directory from file system..\n\n", dirname);
    int i = 0;
    int j = 0;
    int count = 0;
    disk_ptr = fopen("vsdisk", "r+"); 
    for (i = 0; i < 100; i++)
    { 
        char *temp = (char *)malloc(sizeof(char) * strlen(FAT[i][0]));
        temp = strdup(FAT[i][0]); 
        count++; 
        if (strcmp(temp, dirname) == 0)
        {    
            rewind(disk_ptr); 
            char *newLine = (char *)malloc(sizeof(char) * 140);
            while (count >= 0)
            {                                              
                fgets(newLine, sizeof(newLine), disk_ptr); 
                count--;                           
            }
            for (j = 0; j < 5; j++)
            {   
                strcpy(FAT[i][j], ".");     
            }
            break; 
        }
    }
}


void printFileContents(char* filename,int sockfd) {
    if(!filename) {
        printf("No filename entered.. try again!\n");
        return;
    }
    char buf[510];
    strcpy(buf,"Going to read");
    
    printf("%s contents below:\n", filename);
    int i = 0;
    for (i = 0; i < 100; i++)
    { 
        if (strcmp(FAT[i][0], filename) == 0)
        {             
            int indexToPrint = atoi(FAT[i][2]);
            //printf("%s", physicalDir[indexToPrint]);
            
            write(sockfd,physicalDir[indexToPrint],strlen(physicalDir[indexToPrint]));
        }
    }
    printf("\n");
}


void printDirContent(char* dirname, int sockfd)
{
    printf("Check#3\n");
    if (!dirname) {
        printf("No directory name specified..try again!\n\n");
        return;
    }

    printf("Check#4\n");

    printf("Below are the contents of directory %s:\n", dirname);

    /*for (int i=0;i<100;i++){
        for (int j=0;j<5;j++){
            if(j == 4)
            printf("%s\n",FAT[i][j]);
            else
            printf ("%s |",FAT[i][j]);
        }
    }

    pthread_mutex_lock(&clients_mutex);
    int empty = findEmptyEntryFAT();
    for (int i =2;i< empty;i++){
        printf("\nTEst %d\n",empty);
         printf("%s | ", FAT[i][0]);
         printf("%s | ", FAT[i][1]);
         printf("%s | ", FAT[i][2]);
         printf("%s | ", FAT[i][3]);
         printf("%s\n", FAT[i][4]);
    }
    pthread_mutex_unlock(&clients_mutex);


   printf("Check#5\n");*/
    int empty = findEmptyEntryFAT();
    for (int i = 2; i < 100; i++)
    { 
        char *temp = (char *)malloc(sizeof(char) * strlen(FAT[i][3]));
        temp = strdup(FAT[i][3]); 
        char *tempType = (char *)malloc(sizeof(char) * strlen(FAT[i][1]));
        tempType = strdup(FAT[i][1]); 

        if(strcmp(temp, dirname) == 0) {
             
            for (x = 0; x < 5; x++) 
            {   
                // write(sockfd,physicalDir[indexToPrint],strlen(physicalDir[indexToPrint]));
                if (x == 4) {
                    //printf("%s", FAT[i][x]); 
                    write(sockfd,FAT[i][x],strlen(FAT[i][x]));
                } else {
                //    printf("%s | ", FAT[i][x]); 
                    write(sockfd,FAT[i][x],strlen(FAT[i][x]));
                    char *kta;
                    strcpy(kta," |");
                    write(sockfd,kta,strlen(kta));
                }
            }
            int j = 0;
            while(j != 1) {
                printf("\n");
                j++;
            }
        }
    }
    printf("\n");
}

void printDirHierarchy()
{
    printf("Test#1");
    printf("Printing directory hierarchy below:\n");
    printf("File Name | FileOrDir | Index Number | Parent | Date and Time\n");
    int i = 0;
    for (i = 2; i < 100; i++)
    { 
        for (x = 0; x < 5; x++)
        {
            if (x == 4) 
            {
                printf("%s", FAT[i][x]); 
            } else 
            {
                printf("%s | ", FAT[i][x]); 
            }
        }
        printf("\n"); 
    }
}


char **parseCommand(char *command, char **argv, int *argc)
{
    int i = 0;
    *(argc) = 0; 
    for (i = 0; i < 100; i++)
    {
        argv[i] = strtok_r(command, " \t\r\a", &command); 

        if (argv[i] == NULL)
            break;
        (*argc)++; 
    }
    argv[strlen(*argv) + 1] = NULL; 
    return argv;                    
}

char *getTime()
{
    time_t newTime = time(NULL);  
    char *time = ctime(&newTime); 
    time[strlen(time) - 1] = 0;   
    return time;                  
}

int findEmptyEntryFAT()
{
    int i = 0;
    for (i = 0; i < 100; i++)
    { 
        if (strcmp(FAT[i][0], ".") == 0)
        {             
            startingIndex++;
            return i; 
        }
    }
    printf("Never found an empty entry in the FAT..\n"); 
    return -1;                                           
}

int findEmptyBlock()
{
    int i = 0;
    for (i = 0; i < 100; i++)
    { 
          if (strcmp(physicalDir[i], ".") == 0)
        {             
            startingIndex++;
            return i; 
        }
    }
    printf("Never found an empty entry in the FAT..\n"); 
    return -1;                                           
}

int printNumberOfAvailableBlocks()
{
    int i = 0;
    int j = 0;
    int emptyBlocks = 0;
    for (i = 0; i < 100; i++)
    { 
          if (strcmp(FAT[i][0], ".") == 0)
        {   
            emptyBlocks++;
        }
    }
    return emptyBlocks;
}

int findParent(char *parent)
{
    int i = 0;
    int j = 0;
    startingIndex = 0;
    for (i = 2; i < 100; i++)
    { 
        char *temp = strdup(FAT[i][0]);     
        char *tempType = strdup(FAT[i][1]);
        startingIndex++; 
        if ((strcmp(temp, parent) == 0) && (strcmp(tempType, "dir") == 0))
        { 
            return 1; 
            break;
        }
    }
    return 0; 
}

void insertEntry(int emptyIndex, char *name, char *fileOrDir, char *indexNumber, char *parent, char *timestamp)
{
    
    FAT[emptyIndex][0] = (char *)malloc(sizeof(char) * 300); 
    FAT[emptyIndex][1] = (char *)malloc(sizeof(char) * 300); 
    FAT[emptyIndex][2] = (char *)malloc(sizeof(char) * 300); 
    FAT[emptyIndex][3] = (char *)malloc(sizeof(char) * 300); 
    FAT[emptyIndex][4] = (char *)malloc(sizeof(char) * 300); 

    strcpy(FAT[emptyIndex][0], name);
    strcpy(FAT[emptyIndex][1], fileOrDir);
    strcpy(FAT[emptyIndex][2], indexNumber);
    strcpy(FAT[emptyIndex][3], parent);
    strcpy(FAT[emptyIndex][4], timestamp);

    printf("New file/dir created: ");
    printf("%s | ", FAT[emptyIndex][0]);
    printf("%s | ", FAT[emptyIndex][1]);
    printf("%s | ", FAT[emptyIndex][2]);
    printf("%s | ", FAT[emptyIndex][3]);
    printf("%s", FAT[emptyIndex][4]);

  
    if (startingIndex == 0)
    {
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][0]);
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][1]);
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][2]);
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][3]);
        fprintf(disk_ptr, "%s", FAT[emptyIndex][4]);
    }
    else
    {
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][0]);
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][1]);
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][2]);
        fprintf(disk_ptr, "%s|", FAT[emptyIndex][3]);
        fprintf(disk_ptr, "%s", FAT[emptyIndex][4]);
    }

}
