#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>

#define NAME_FILE_SIZE 256     /*size for name of the file*/
#define SIZE 128     /*size for to store size of the file*/
#define LENGTH 16            /*length in bytes for buffer argument*/
#define true 1

void send_file(int s);          
struct sockaddr_in client_address; /*Structure for handling internet addresses*/
char action[LENGTH];            /*buffer where the message should be stored for recvfrom()*/

int main()
{
   memset(action, 0, LENGTH); /*memset(void *s, int c, size_t n) - memset() fills n bytes pointed by s with constant byte c*/

   int s = socket(AF_INET, SOCK_DGRAM, 0);  /*socket creation*/
   if (s == -1){
	perror("Client Socket creation failed!\n");
	exit(0);
   }

   memset(&client_address, 0, sizeof(struct sockaddr_in));
 
   client_address.sin_family = AF_INET;                      /*AF_INET - IP4 protocol*/
   client_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /*converts the unsigned integer from host byte order to network byte order*/
   client_address.sin_port = htons(10000);

   size_t filesize;
   printf("Give action: (type: send): ");
   scanf("%s", action);

   int ret_bytes;
                                                                         /*send action to server using sendto()*/
   if (sendto(s, action, strlen(action), 0,(struct sockaddr *) &client_address,
		sizeof(struct sockaddr)) < 0){
	perror("Sendto() action failed!\n");
    exit(1);
   }
   else{
	perror("action OK");
   }
    if (strncmp(action, "send", strlen("send")) == 0){
      send_file(s);                                   /*function send_file called*/
      printf("Action: send\n");
   }
   close(s);
   return 0;
}

void send_file(int s)
{
   int ret_bytes;                      /*for storing fread - reading bytes of file result*/
   size_t size_of_file;                /*for storing size of file position*/
   char f_name[NAME_FILE_SIZE];        /*file name*/

   memset(f_name, 0, sizeof(f_name));  /*memory allocated for file name*/
   char buf_file[1500];                /*char array for buf_file*/
   char size_str[SIZE];        /*char array for string size*/

   FILE* ptr_file;                     /*File pointer*/

   printf("Type requested filename (uploaded): ");
   scanf("%s", f_name);

                                      /* Check to see if file exists*/
   if (!(access(f_name, F_OK) != -1)){       /*check for file access, F_OK - existence of file*/
        printf("File doesn't exist\n");
        exit(1);
   }
   else{
    printf("File exists\n");
   }

                            /* send filename to server to be downloaded*/
   if ( sendto(s, f_name, strlen(f_name), 0,(struct sockaddr *) &client_address,
		sizeof(struct sockaddr)) < 0 ){
        perror("sendto() filename failed!\n");
        exit(1);
   }
  
   ptr_file = fopen(f_name, "r");        /*open file name for read operation*/
                                         /* Determine the file size */
   fseek(ptr_file, 0, SEEK_END);          /*seek-end - end of file*/
   size_of_file = ftell(ptr_file);             /*ftell returns the current file position of the given stream*/
   printf("File size: %lu\n", size_of_file);

   int sizefile;

                                    /* Convert fileSize integral # to string:*/
   memset(size_str, 0, SIZE);
   snprintf(size_str, SIZE, "%d", (int) size_of_file); /*prints most size bytes*/

   if (sendto(s, size_str, strlen(size_str), 0,(struct sockaddr *)&client_address,
              sizeof (struct sockaddr)) < 0){
        perror("sendto() a file size failed!\n");
        exit(1);
   }
                             /*fseek - sets the file position of the stream to the given offset*/
   fseek(ptr_file, 0, SEEK_SET);              /*0 - current position of pointer, SEEK_SET - begining of the file*/
   memset(buf_file, 0, sizeof(buf_file));   /*memset returns to the memory area buf_file */

                                      /*Begin to send data to server*/
   while(true)
   {
        ret_bytes = fread(buf_file, 1, 1500, ptr_file);      /*read into the array pointed to by ptr*/
        if (ret_bytes == 0){
           break;
         }
        else if (ret_bytes < 0){
            perror("fread() failed to copy file to file_buf!\n");
            exit(1);
        }

        if (sendto(s, buf_file, ret_bytes, 0, (struct sockaddr *)&client_address, sizeof (struct sockaddr)) < 0){
            perror("Sending file to client failed!\n");
           exit(1);
        }

        memset(buf_file, 0, sizeof(buf_file));
   }
  fclose(ptr_file);
}
