#include <iostream> 
#include <cstdlib>        // atoi
#include <sys/time.h>     // gettimeofday
#include <sys/types.h>    // socket, bind 
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa 
#include <netinet/in.h>   // htonl, htons, inet_ntoa 
#include <arpa/inet.h>    // inet_ntoa 
#include <netdb.h>        // gethostbyname 
#include <unistd.h>       // read, write, close 
#include <strings.h>      // bzero 
#include <netinet/tcp.h>  // SO_REUSEADDR 
#include <sys/uio.h>      // writev 
#include <pthread.h>      // pthread
#include <cstring>

int main(int argc, char* argv[]) {
   int bufsize = 1500;
   char buffer[bufsize];
   bool isExit = false;

   struct addrinfo hints, *res;
   int clientSd;

   // first, load up address structs with getaddrinfo():

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

   getaddrinfo(NULL, "1140", &hints, &res);

   // make a socket:

   clientSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

   // bind it to the port we passed in to getaddrinfo():

   bind(clientSd, res->ai_addr, res->ai_addrlen);

   // listen for connections
   if (listen(clientSd, 10) < 0) {
      perror("Can't listen...");
      exit(1);
   }
   std::cout << "Socket is listening for clients..." << std::endl;
   

   // now accept an incoming connection
   struct sockaddr_storage clientAddr;
   socklen_t clientAddrSize = sizeof(clientAddr);

   int server = accept(clientSd, (struct sockaddr *)&clientAddr, &clientAddrSize);
   // first check if it is valid or not
   if (server < 0) 
      std::cout << "Error on accepting..." << std::endl;

   while (server > 0) {
      /* 
         Note that we would only get to this point after a 
         client has successfully connected to our server. 
         This reads from the socket. Note that the read() 
         will block until there is something for it to read 
         in the socket, i.e. after the client has executed a 
         the send().
         It will read either the total number of characters 
         in the socket or 1500
      */
      std::cout << "Client: ";
      do {
         recv(server, buffer, bufsize, 0);
         std::cout << buffer << " ";
         if (*buffer == '#') {
            *buffer = '*';
            isExit = true;
         }
      } while (*buffer != '*');

      do {
         std::cout << "\nServer: ";
         do {
            std::cin >> buffer;
            send(server, buffer, bufsize, 0);
            if (*buffer == '#') {
               send(server, buffer, bufsize, 0);
               *buffer = '*';
               isExit = true;
            }
         } while (*buffer != '*');

         std::cout << "Client: ";
         do {
            recv(server, buffer, bufsize, 0);
            std::cout << buffer << " ";
            if (*buffer == '#') {
               *buffer == '*';
               isExit = true;
            }
         } while (*buffer != '*');
      } while (!isExit);

      /* 
         Once a connection has been established, both ends 
         can both read and write to the connection. Naturally, 
         everything written by the client will be read by the 
         server, and everything written by the server will be 
         read by the client.
      */

      close(server);
      std::cout << "\nGoodbye..." << std::endl;
      isExit = false;
      exit(1);
   }

   close(clientSd);
   return 0;
}