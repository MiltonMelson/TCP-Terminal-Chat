#include <iostream>
#include <cstdlib>        // atoi
#include <sys/types.h>    // socket, bind 
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa 
#include <netinet/in.h>   // htonl, htons, inet_ntoa 
#include <arpa/inet.h>    // inet_ntoa 
#include <netdb.h>        // gethostbyname 
#include <unistd.h>       // read, write, close 
#include <strings.h>      // bzero 
#include <netinet/tcp.h>  // SO_REUSEADDR 
#include <sys/uio.h>      // writev 
#include <cstring>        // memset 

using namespace std;


int main(int argc, char* argv[]) {   
   char* serverName = (char*)"127.0.0.1";
   int bufsize = 1500;
   char buffer[bufsize];
   bool isExit = false;

   struct addrinfo hints, *res;
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   getaddrinfo(serverName, "1140", &hints, &res);

   // make a socket:

   int serverSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

   // connect!

   connect(serverSd, res->ai_addr, res->ai_addrlen);
   cout << "\n\n=> Enter # to end the connection\n" << endl;

    // Once it reaches here, the client can send a message first.

    do {
        cout << "Client: ";
        do {
            cin >> buffer;
            send(serverSd, buffer, bufsize, 0);
            if (*buffer == '#') {
                send(serverSd, buffer, bufsize, 0);
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');

        cout << "Server: ";
        do {
            recv(serverSd, buffer, bufsize, 0);
            cout << buffer << " ";
            if (*buffer == '#') {
                *buffer = '*';
                isExit = true;
            }

        } while (*buffer != '*');
        cout << endl;

    } while (!isExit);

    cout << "\n=> Connection terminated.\nGoodbye...\n";

    close(serverSd);
    return 0;
}