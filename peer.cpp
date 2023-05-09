#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h> 
#include <string>
#include <fstream>
#include <pthread.h>

#define PORT 3000
#define CLIENT_FILE "client.txt"

pthread_mutex_t clientMutex = PTHREAD_MUTEX_INITIALIZER;

class Server_data {
    public:
    int socket;
    Server_data(int serverSock) {
        this->socket = serverSock;
    }
};

void writeInFile(const char msg[]) {
    pthread_mutex_lock(&clientMutex);
    std::ofstream client(CLIENT_FILE, std::ios::app);
    client << msg << std::endl;
    client.close();
    pthread_mutex_unlock(&clientMutex);
}

void* sending(void* arg) {
    Server_data* server = (Server_data*) arg;
    int serverSocket = server->socket;
    char data[4096];
    while(true) {
        memset(data, 0, 4096);
        std::cin.getline(data, 4096);
        if((data[0] == 'q' && data[1] == '\0') || data[0]=='\0') {
            break;
        }
        send(serverSocket, data, strlen(data), 0);
        std::cout << "Message sent!\n";
        writeInFile("Message sent!");
    }
    pthread_exit(NULL);    
}

void* receiving(void* arg) {
    Server_data* server = (Server_data*)arg;
    int serverSocket = server->socket;
    char msg[2048];
    while(true) {
        memset(msg, 0, 2048);
        int receiveBytes = recv(serverSocket, msg, 2048, 0);
        if(receiveBytes == 0) {
            std::cout << "The server disconnected\n";
            writeInFile("The server disconnected\n");
            exit(0);
        }
        std::cout << msg << std::endl << std::endl;
        writeInFile(msg);
    }
    pthread_exit(NULL);
}

int main() {
    
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    // saddr.sin_addr.s_addr = INADDR_ANY;
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == 0) {
        std::cout << "Socket error\n";
        return -1;
    }
    
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);
    
    if(connect(serverSocket, (sockaddr*)&saddr, sizeof(saddr)) < 0) {
        std::cout << "Connection error\n";
        return -1;
    }
    
    Server_data* serverdata = new Server_data(serverSocket);
    char received[4096] = {0};
    
    read(serverSocket, received, 4096);
    
    std::cout << "Connected on port "<< received <<"\n\nRegister | Login\n\n";
    writeInFile(("Connected on port "+ std::string(received)+"\n\nRegister | Login\n").c_str());
    
    pthread_t sendThread, receiveThread;
    if(pthread_create(&sendThread, NULL, sending, (void*)serverdata)) {
        std::cout << "Error creating sending thread!\n";
    }
    if(pthread_create(&receiveThread, NULL, receiving, (void*)serverdata)) {
        std::cout << "Error creating receiving thread\n";
    }
    
    pthread_join(sendThread, NULL);
    close(serverSocket);
    
    return 0;
}