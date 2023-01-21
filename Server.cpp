/** 
 * CSS 432 - HW1 
 * Professor Yang Peng
 * Created by Milton Melson
 * 
 * Server.cpp
      The server will create a TCP socket that listens on a port (the last 4 digits 
      of your student ID number unless it is < 1024, in which case, add 1024 to your ID 
      number). The server will accept an incoming connection and then create a new 
      thread (use the pthreads library) that will handle the connection. The new thread 
      will read all the data from the client and respond back to it (acknowledgement). 
      The response detail will be provided in Server.cpp. 
*/

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

using namespace std;

const int BUFSIZE = 1500;
const int N = 9;

struct thread_data {
   int repetition;
   int sd;
};

void* myFunction(void* arg);

int main(int argc, char* argv[]) {
   char* port = argv[1];  // the last 4 digits of your student id 
   int repetition = atoi(argv[2]); 

   // load up address structs with getaddrinfo()
   struct addrinfo hints, *res; 
   memset(&hints, 0, sizeof (hints)); 
   hints.ai_family = AF_UNSPEC; 
   hints.ai_socktype = SOCK_STREAM; 
   hints.ai_flags = AI_PASSIVE; 

   int status = getaddrinfo(NULL, port, &hints, &res); 
   if (status != 0) {
      cout << "getaddrinfo error: " << gai_strerror(status) << endl;
      return -1;
   }

   // make a socket
   int serverSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if (serverSd == -1) {
      perror("Can't create server socket!");
      return -1;
   } else {
      cout << "Server socket created..." << endl;
   }

   /**
    * loss the pesky "Address already in use" error message
    * Set the SO_REUSEADDR option. (Note this option is useful to prompt OS to 
      release the server port as soon as your server process is terminated.)
   */  
   const int yes = 1;
   setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes));

   // bind the socket
   if ((bind(serverSd, res->ai_addr, res->ai_addrlen)) == -1) {
      perror("Can't bind to IP/port");
      return -2;
   } else {
      cout << "Binding complete..." << endl;
   }

   // listen for N request
   if (listen(serverSd, N) == -1) {
      perror("Can't listen!");
      return -3;
   } else {
      cout << "Socket is listening..." << endl;
   }
   

   // now accept an incoming connection
   struct sockaddr_storage clientAddr;
   socklen_t clientAddrSize = sizeof(clientAddr);
   while (true) {
      int newSd = accept(serverSd, (struct sockaddr *)&clientAddr, &clientAddrSize);
      if (newSd == -1) {
         cerr << "Problem with client connecting!";
         return -4;
      }
      cout << "Connected to sd: " << newSd << endl;
      // create a new thread
      pthread_t tid;
      struct thread_data *data = new thread_data;
      data->repetition = repetition;
      data->sd = newSd;
      int iret1 = pthread_create(&tid, NULL, myFunction, (void*)data);
   }
   close(serverSd);
   return 0;
}

void *myFunction(void *arg) {
   struct thread_data* ptr = (struct thread_data*)arg;
   struct timeval start;
   struct timeval stop;
   char databuf[BUFSIZE];

   // get starting time
   gettimeofday(&start, NULL); 

   // read data and count number of reads
   int count;
   for (int i = 0; i < ptr->repetition; i++) {
      for(int nRead = 0; (nRead += read(ptr->sd, databuf, BUFSIZE - nRead)) < BUFSIZE; count++);
   }
   // get the stopping time
   gettimeofday(&stop, NULL); 

   // write the count back to client side
   write(ptr->sd, &count, sizeof(count));
   
   // get the total time
   int elapsedTime = (((stop.tv_sec - start.tv_sec) * 1000000L) + (stop.tv_usec - start.tv_usec));

   // print out message
   cout << "Data-Receiving time = " << elapsedTime << " usec" << endl;
   close(ptr->sd);
   return arg;
}