#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define NAME_FILE_SIZE 256     /*size for name of the file*/
#define LENGTH	16         /*length in bytes for buffer argument*/
#define SIZE 128         /*size for to store size of the file*/
#define true 1

unsigned client_length;    /*contains size of client_address for recvfrom()*/
void send_file(int s);       /*function for file operation  */
struct sockaddr_in client_address; /*Structure for handling internet addresses*/

int main()
{
   char action[LENGTH];       /*buffer where the message should be stored for recvfrom()*/
   struct sockaddr_in server_address; /*Structure for handling internet addresses*/
                                /*Initialize arrays*/
   memset(action, 0, LENGTH);  /*memset(void *s, int c, size_t n) - memset() fills n bytes pointed by s with constant byte c*/
   
   int s = socket(AF_INET, SOCK_DGRAM, 0);    /*socket creation*/
   if (s == -1){
	perror("Socket creation failed!\n");
	exit(0);
   }

   memset(&server_address, 0, sizeof(server_address));

   server_address.sin_family = AF_INET;      /*AF_INET - IP4 protocol*/
   server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  /*converts the unsigned integer from host byte order to network byte order*/
													/*INADDR_LOOPBACK - address of this machine*/
   server_address.sin_port = htons(10000);
   size_t fileSize;
   if (bind(s, (struct sockaddr *) &server_address, sizeof(server_address)) != 0){      /*socket binding*/
	perror("Socket binding failed!\n");
	exit(0);
   }

   client_length = sizeof(client_address);        /*contains size of client_address for recvfrom()*/

   while(true)
   {
	/*Receive the action type from client*/
	recvfrom(s, action, LENGTH, 0, (struct sockaddr *) &client_address, &client_length);

   	if (!(strncmp(action, "send", strlen("send")) == 0)){      /*action - send*/
        printf("Invalid command.\n");
   	}
   	else{
	   printf("Action: send\n");
	   send_file(s);
   	}
   }
   return 0;
}

void send_file(int s)
{
   struct sockaddr_in client_address; /*Structure for handling internet addresses*/
   char f_name[NAME_FILE_SIZE];   /*file*/
   char size_file_str[SIZE];    /*buffer to store file length size*/
   off_t size_of_file;        /*off_t - used for file size used for files*/
   int recv_len;             /* store received file from recvfrom()*/
   FILE *file_new_ptr;           /*file pointer to access file*/
   client_length = sizeof(client_address); /*stored size from recvfrom()*/
   unsigned char *data;                    /*data in a file*/
   unsigned bytes_total = 0, bytes_recev = 0;
   unsigned char *buf_file;               /*data stored*/


   /* Receive the name of file from client*/
   recvfrom(s, f_name, NAME_FILE_SIZE, 0, (struct sockaddr *) &client_address, &client_length);
   printf("Filename: %s\n", f_name);
   int ret_bytes;

   /*Receive the file size*/
   memset(size_file_str, 0, SIZE);           
   recv_len = recvfrom(s, size_file_str, SIZE, 0,
                (struct sockaddr *) &client_address, &client_length);
   int temp;
   if (recv_len <= 0)
   {
           perror("Error in recvfrom() file size\n");
           close(s);
           exit(1);
   }
   temp=1;

   size_of_file = atoi(size_file_str);      /*conversion from char to size of file*/ 
   printf("Received size from client: %lu\n", size_of_file);
   int file_s;
   data = malloc(size_of_file);            /*allocates memory*/
   file_s=1;
   buf_file = data;
   while(size_of_file>bytes_total){              /*while loop to recieve file byte by byte*/
           bytes_recev = recvfrom(s, buf_file, (size_of_file - bytes_total), 0, NULL, 0);  /*receving bytes from client for a file*/
           if (!(bytes_recev < 0)) {
           }else{
              perror("Error in recvfrom() file contents\n");
              close(s);
              exit(1);
           }
           buf_file=buf_file+bytes_recev;
           bytes_total=bytes_total+bytes_recev;
   }

   file_new_ptr = fopen(f_name, "w+");            /*opening file pointer with write operation*/

   if(fwrite(data, 1, size_of_file, file_new_ptr) < 0)   /*writing of data into file*/
   {
        perror("Error writing to new file.\n");
        exit(1);
   }

   free(data);                /*free the memory for data*/
   data = NULL;
   fclose(file_new_ptr);       /*pointer close*/
}
