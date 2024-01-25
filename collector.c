#include "headers/collector.h"
#include "headers/utils.h"
#include "headers/list.h"
#include "headers/master.h"

Msg_t *msg;    // struttura messaggi ricevuti dai worker
Node_t *head;  // linked list dei file 
int lfd;       // listening socket
int isExit;    // extern

int collector(int lfd){
      // inizializzazione strutture
   head = NULL;
   msg = (Msg_t*)calloc(1,sizeof(Msg_t));
   msg->filename = calloc(1, sizeof(char)*MAX_DIR_LEN);
   isExit=0;

   int cfd;          // client_fd
   int fd;
   int fd_num = lfd; // massimo fd attivo trovato, inizializzo a lfd
   fd_set set; 
   fd_set rdset;

      // ascolto connessioni sul listening socket
   if(listen(lfd, SOMAXCONN) == -1){
      perror("collector - listen");
      return cleanup(-1);
   }
   FD_ZERO(&set);
   FD_ZERO(&rdset);
   FD_SET(lfd,&set);

   while(!isExit){
      rdset=set;  // maschera per la select
      if (select(FD_SETSIZE, &rdset, NULL,NULL,NULL) == -1){ 
         perror("collector - select");
         return cleanup(-1);
      }
         // select ok
      for(fd = 0; fd<=fd_num && !isExit; fd++){
         if(FD_ISSET(fd,&rdset)){
            if(fd == lfd){    // socket connect pronto
               if((cfd=accept(lfd,NULL,0))==-1){
                  perror("collector - accept");
                  return cleanup(-1);
               }
               FD_SET(cfd, &set);
               if(cfd > fd_num) fd_num = cfd; 
            }
            else{    // I/O socket pronto
               cfd = fd;
               int nread;
                  // lettura da worker
               if((nread=read_msg(msg, cfd))==-1){
                  //perror("collector - read");
                  return cleanup(-1);
               }
                  // tratto end e interrupt allo stesso modo, effettuo la stessa operazione
               if(!strcmp(msg->filename, "END") || !strcmp(msg->filename, "INTERRUPT")){  
                  print_list(head); 
                  isExit=1;
               }
                  // segnale di stampa
               else if(!strcmp(msg->filename, "SIGNAL")){
                  print_list(head); 
               }
               else{    // aggiungo nella list
                  Node_t *new = (Node_t*) calloc(1,sizeof(Node_t));
                  new->filename = (char*) calloc(1,sizeof(char)*MAX_DIR_LEN);
                     // ho bisogno di ottenere il relative path
                  cut_string(msg->filename, new->filename);
                  new->value = msg->sum;
                  insert(&head,new);    // inserimento ordinato
               }
            }
         }
      }
   }
   return cleanup(0);
}


void cut_string(char* fullpath, char* result){

      //salvataggio directory
   char currpath[MAX_DIR_LEN];
   getcwd(currpath, sizeof(currpath)); 
   
   int i = strlen(currpath)+1;   // arrivo all'indice della cwd
   int j;
   // copio nel risultato i caratteri oltre la cwd, ottenendo il relative path
   for(j=0; fullpath[i]!= '\0'; j++){
      result[j]=fullpath[i]; 
      i++;
   }  // termino la stringa del risultato
   result[j+1]='\0';

}

int read_msg(Msg_t* msg, int fd){
   int tmp;
   int len = -1;
   if((tmp = readn(fd, &len, sizeof(int)))==-1){
      perror("collector - readn length");
      return -1;
   }
   char* buf = (char*) calloc(1,sizeof(char)*MAX_DIR_LEN);
   if((tmp = readn(fd,buf,sizeof(char)*len))==-1){
      perror("collector - readn file");
      free(buf);
      return -1;
   }
   strncpy(msg->filename, buf, len); // copio quello che ho ricevuto dal buffer
   free(buf);
   if((tmp = readn(fd,&(msg->sum),sizeof(long)))==-1){
      perror("collector - readn sum");
      return -1;
   }
   //puts("collector - read done!");
   return 0;
}

int cleanup(int ret_code){
   if(head) free_list(head);
   if(msg->filename) free(msg->filename);
   if(msg) free(msg);
   close(lfd);
   unlink(SOCKNAME);
   return ret_code;
}
