s/** 
 * CSS 432 - HW1 
 * Professor Yang Peng
 * Created by Milton Melson
 * 
 * Client.cpp 
      The client will create a new socket, connect to the server, and send data 
      using three different ways of writing data (data transferring). It will then wait for 
      a response and output the response.
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
#include <cstring>        // memset 

using namespace std;

const int DATABUFFSIZE = 1500;   // current max data buffer size

// Helper functions
void printStats(int type, long DTT, long RTT, int reads);
long elapsedTime(timeval start, timeval stop);


int main(int argc, char* argv[]) {
   if (argc < 7) {
      perror("Too Few Arguments passed in need 6");
      return -1;
   }
   if (atoi(argv[6]) > 3 || atoi(argv[6]) < 1) {
      perror("Transfer Type is Invalid, can only accept types 1-3");
      return -1;
   }
   
   char* serverPort = argv[1];         // Servers port number
   char* serverName = argv[2];         // Servers IP address or Host name
   int iterations = atoi(argv[3]);     // number of iterations client performs on data transmission using "single write", "writev" or "multiple writes"
   int nbufs = atoi(argv[4]);          // number of data buffers
   int bufSize = atoi(argv[5]);        // size of each buffer in bytes
   int type = atoi(argv[6]);           // type of transfer scenario (1-3)

   // check for data buffer size error
   if (nbufs*bufSize != DATABUFFSIZE) {
      perror("Data Buffer size needs to be equal to 1500 (nbufs * bufSize == 1500).");
      return -1;
   }

   // Displays the parameters
   cout << "ServerPort: " << serverPort << "\nServerName: " << serverName << "\nInteration: " << iterations
   << "\nnbufs: " << nbufs << "\nbuffSize: " << bufSize << "\nType: " << type << endl;

   struct addrinfo hints;             
   struct addrinfo *servInfo;          // Points to the results
   memset(&hints, 0, sizeof(hints));   // makes sure the struct hints is empty
   hints.ai_family = AF_INET;          // Address Family Internet (IPv4 or IPv6)
   hints.ai_socktype = SOCK_STREAM;    // TCP socket type

   int status = getaddrinfo(serverName, serverPort, &hints, &servInfo); 

   // check for error after getaddrinfo
   if (status < 0) {
      cout << "getaddrinfo error: " << gai_strerror(status) << endl;
      return -1;
   }

   // get socket descriptor
   int clientSd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);

   // check for error
   if (clientSd < 0) {
      perror("Socket descriptor error");
      close(clientSd);
      return -1;
   } 
   cout << "Client socket created..." << endl;

   status = 0;
   // connect socket
   // check for errors
   status = connect(clientSd, servInfo->ai_addr, servInfo->ai_addrlen);
   if (status < 0) {
      cerr << "Connection error!";
      close(clientSd);
      return -1;
   }

   char databuf[nbufs][bufSize]; // data buffer where nbufs * bufSize = 1500
   struct timeval start, lap, stop; // timval variables for output statistics

   // Get the starting time
   gettimeofday(&start, NULL);
   for (int i = 0; i < iterations; i++) {
      // Scenario 1
      if (type == 1) {
         for (int j = 0; j < nbufs; j++) 
            write(clientSd, databuf[j], bufSize);  // write databuf[j] to clientSd
      }
      // Scenario 2
      else if (type == 2) {
         struct iovec vector[nbufs]; 
         for (int j = 0; j < nbufs; j++) {   // allocates data and len in iovec from databuf
            vector[j].iov_base = databuf[j]; 
            vector[j].iov_len = bufSize; 
         } 
         writev(clientSd, vector, nbufs); // writes entire iovec at once
      }
      // Scenario 3
      else {
         write(clientSd, databuf, nbufs * bufSize);   // write entire databuf at once
      }
   }

   gettimeofday(&lap, NULL);

   long dataTransTime, roundTripTime;
   int reads = 0;

   read(clientSd, &reads, sizeof(reads));
   gettimeofday(&stop, NULL);
   dataTransTime = elapsedTime(start, lap);
   roundTripTime = elapsedTime(start, stop);
   printStats(type, dataTransTime, roundTripTime, reads);
   close(clientSd);
   freeaddrinfo(servInfo); // free the linked list
   return 0;
}


// Helper functions


/** elapsedTime
 * @brief Takes a starting and stopping timeval and returns the difference
 * @param start The starting timeval
 * @param stop The stopping timeval
 * @return Returns number of milliseconds as long
*/
long elapsedTime(timeval start, timeval stop) {
   return (((stop.tv_sec - start.tv_sec) * 1000000L) + (stop.tv_usec - start.tv_usec));
}

/** printStats
 * @brief Prints out the statistics
 * @param type The type of transfer scenario 
 * @param DTT Data transmission time
 * @param RTT round trip time
 * @param reads the number of reads performed on server side
*/
void printStats(int type, long DTT, long RTT, int reads) {
   cout << "Test " << type << ": Data-Transmission Time = " << DTT << " usec, " <<
   "Round-Trip Time = " << RTT << " usec, " << "#Reads = " << reads << endl;
}
