#include "headers/worker.h"
#include "headers/master.h"
#include "headers/utils.h"
#include "headers/list.h"

void worker(void* arg){
   char* sockdir = (char*)arg;   // ricavo socket directory

   // connessione socket
   int wfd;
   if((wfd = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
		perror("socket");
		exit(EXIT_FAILURE);
	}
   struct sockaddr_un saddr;
   memset(&saddr, '0', sizeof(saddr));
   saddr.sun_family= AF_LOCAL;
   strncpy(saddr.sun_path, sockdir,strlen(sockdir)+1);
   
   int size=strlen(saddr.sun_path);
   while ((connect(wfd, (Sockaddr_t *)&saddr, sizeof(saddr))) == -1){
		if(errno == ENOENT && size > 0){    // bug fix per path del socket
         size--;
         saddr.sun_path[size] = '\0';
      }
      else if(errno == ECONNREFUSED){     // socket non pronto per la listen
         sleep_ms(1); 
      }
      else{ // altri errori
         perror("worker - connect");
         unlink(saddr.sun_path);
         exit(EXIT_FAILURE);
      }
   }

   // prelevo il file su cui devo lavorare
   char* filename = pop_h(jobs);
   //printf("worker - filename: %s\n", filename);
   long sum=0;

   // non ho necessità di differenziare i casi - scrivo direttamente il messaggio 
   // e avviso dell'uscita aggiornando la variabile
   if(!strcmp(filename, "END") || !strcmp(filename, "INTERRUPT")){ 
      isExit = 1; 
      if(write_msg(filename, sum, wfd)== -1)
         fprintf(stderr,"worker - write %s: %s\n", filename, strerror(errno)); 
      
      free(filename);
      free_deque(jobs);
      close(wfd);
      unlink(saddr.sun_path);
      return; 
   }

   FILE* file = NULL;
   file = fopen(filename, "rb");
   if(file == NULL)
      fprintf(stderr,"Error, cannot open <%s>: %s\n", filename, strerror(errno)); 

   else{    // effettuo l'operazione di "somma ponderata"
      long curr_num =0;
      for(int i=0; fread(&curr_num,sizeof(long),1,file)==1; i++){ 
         sum += (i*curr_num);
      }
            // scrivo somma e file sulla socket
      if(write_msg(filename, sum, wfd)== -1)
         perror("worker - write_msg");
   }
   fclose(file);
   free(filename);
   return;
}


int write_msg(char* filename, long sum, int wfd){
   
   int len = strlen(filename)+1; 
   if(writen(wfd, &len, sizeof(int))==-1){      // scrittura lunghezza filename
      perror("write - length");
      return -1;
   }

   char* buf = (char*) calloc(1, sizeof(char)*MAX_DIR_LEN);
	sprintf(buf, "%s", filename);
   buf[len]= '\0';
   if(writen(wfd,buf,sizeof(char)*len)==-1){    // scrittura filename
      perror("write - file");
      free(buf);
      return -1;
   }

   if(writen(wfd,&sum,sizeof(long))==-1){       // scrittura somma
      perror("write - sum");
      free(buf);
      return -1;
   }
   free(buf);
   return 0;
}