#include <dirent.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>

#include "headers/utils.h"
#include "headers/threadpool.h"
#include "headers/master.h"
#include "headers/worker.h"
#include "headers/list.h"
#include "headers/collector.h"

/**
 * @brief funzione del thread sighandler, riceve una maschera di segnali, ignora sigpipe
 *       e resta in attesa di segnali tramite sigwait. 
 *       In caso di SIGUSR1 invia segnale di stampa al collector,
 *       mentre in caso di SIGINT, SIGQUIT, SIGTERM, SIGHUP invia un segnale di INTERRUPT 
 *       alla threadpool dei worker per avviare l'uscita.
 * 
 * @param arg (sigset_t*) mask, la maschera di segnali
*/
void* sighandler(void* arg);

/**
 * @brief funzione di cleanup e uscita dal programma. Libera lo spazio allocato, effettua
 *       la disconnessione dalla socket, invia un segnale di interrupt al collector se necessario.
 * 
 * @param status [EXIT_FAILURE | EXIT_SUCCESS], indica il comportamento da eseguire a seconda della 
 *             corretta terminazione o meno del programma
*/
void quit(int status);

/* variabili globali */
pid_t pid;                 // process id necessario per la fork
int sock_fd;               // fd relativo alla listening socket del collector
threadpool_t* worker_pool; // threadpool dei worker
char* sock_path;           // path del socket

/* debug */
int debug = 0;
void debug_args(int* argc, char* argv[]){
   argv[1]="-n"; argv[2]="1";
   argv[3]="-q"; argv[4]="1";
   argv[5]="-d"; argv[6]="testdir";
   argv[7]= "file18.dat"; argv[8]= "file20.dat";
   argv[9]= "file100.dat"; *argc=10;
}

int main(int argc, char* argv[]){
   if(debug) debug_args(&argc, argv);
   else if(argc == 1){  // non ho passato argomenti
      printf("usage <%s> [-n -q -t -d] file* \n", argv[0]);
      exit(EXIT_FAILURE);
   }

   // valori di default
   int nthread=4;
   int qlen=8;
   int wtime=0; 
   isExit=0;

   // creazione signal handler
   pthread_t signal_th;
   sigset_t mask;
   sigemptyset(&mask);           // set mask null

   sigaddset(&mask, SIGINT);     // aggiungo i segnali che voglio gestire
   sigaddset(&mask, SIGQUIT);
   sigaddset(&mask, SIGTERM);
   sigaddset(&mask, SIGHUP);
   sigaddset(&mask, SIGUSR1);

   if(pthread_sigmask(SIG_BLOCK, &mask, NULL)!= 0){   // applico la mask
      perror("sigmask");
      quit(EXIT_FAILURE);
   }
      // creo thread signal handler
   thread_create(&signal_th, NULL, sighandler, (void*)&mask); 

      // getopt - gestisco gli argomenti ricevuti
   int opt, failure =0;
   while((opt = getopt(argc,argv,":n:q:t:d:"))!=-1){ 
      switch(opt) {
         case 'n': 
            nthread = parse_int(optarg);
            break;
         case 'q': 
            qlen = parse_int(optarg);
            break;
         case 't': 
            wtime= parse_int(optarg);
            break;
         case 'd': 
            if(strlen(optarg) > MAX_DIR_LEN){
               fprintf(stderr, "error: directory too long");
               failure++;
               break;
            }
            char buf[MAX_DIR_LEN];
            char* directory = realpath(optarg, buf);
            //printf("directory: %s\n", directory); 
            errno =0;
            DIR* dir= opendir(directory);    // vedo se la directory è valida
            if(!dir){
               fprintf(stderr, "errore all'apertura di <%s>: %s\n",optarg,strerror(errno));
               failure++;
            }
            closedir(dir); 
            break;
         case ':':      // qualora non ci sia l'argomento per le opzioni che lo richiedono
            printf("l'opzione '-%c' richiede un argomento\n", optopt);
            failure++;
            break;
         case '?':      // restituito se getopt trova una opzione non riconosciuta
            printf("l'opzione '-%c' non e' riconosciuta\n", optopt);
            failure++;
           break;
         default:;
      }
   }  // se ho errori da getopt esco
   if(failure) quit(EXIT_FAILURE);

      /* creazione socket */
   sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
   if(sock_fd == -1){
      perror("socket_create");
      quit(EXIT_FAILURE);
   }
   int option = 1;
   setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
   
   struct sockaddr_un sa;  
   memset(&sa, '0', sizeof(sa));
   sa.sun_family= AF_LOCAL;
   strncpy(sa.sun_path, SOCKNAME,strlen(SOCKNAME)+1);
   
      // bind
   if(bind_check(&sock_fd, (Sockaddr_t*)&sa) == -1) quit(EXIT_FAILURE);

      // ottengo absolute path del socket
   char dirbuf[MAX_DIR_LEN];
   getcwd(dirbuf,sizeof(dirbuf));
   strcat(dirbuf, "/");
   strcat(dirbuf, SOCKNAME);
   strcat(dirbuf, "\0");
   sock_path=dirbuf;
   
      // fork
   pid = 0;
   pid=fork();
   if(pid==-1){
      perror("fork_create");
      quit(EXIT_FAILURE);
   }

   if(pid==0){    // figlio: collector
      int check = collector(sock_fd);
      if(check){
         fprintf(stderr, "FATAL: collector\n");
         quit(EXIT_FAILURE);
      }
   }
   else{    // padre
         // creo threadpool figli
      worker_pool = createThreadPool(nthread, qlen); 
      if (worker_pool == NULL){
         perror("worker_pool");
         quit(EXIT_FAILURE);
      }

         // lancio thread master
      pthread_t master_th;
      Master_args_t params= {argc, argv, qlen, wtime, sock_path, &worker_pool};
      thread_create(&master_th, NULL, master, (void*)&params);
   
      thread_join(master_th, NULL);     // aspetto la terminazione del master

      int status;
      if(waitpid(pid,&status,0)==-1){   // aspetto la terminazione del collector
         puts("main - errore nella terminazione del collector");
         quit(EXIT_FAILURE);
      }
         // termino il signal handler e lo aspetto
      pthread_kill(signal_th, SIGINT); 
      thread_join(signal_th, NULL);

         // distruggo la threadpool ed esco
      destroyThreadPool(worker_pool,0);
      quit(EXIT_SUCCESS);
   }
}

void* sighandler(void* arg){
   sigset_t* mask = (void*)arg; // passo mask

      // ignoro sigpipe
   Sigact_t s; 
   memset(&s, 0, sizeof(s));  
   s.sa_handler = SIG_IGN;
   if ((sigaction(SIGPIPE, &s, NULL)) == -1){
      perror("signal handler - sigaction sigpipe");
      quit(EXIT_FAILURE);    
   }
    
   while(!isExit){
      int signal = 0;
      if(sigwait(mask, &signal) != 0){  // resto in attesa di segnali
         perror("signal handler - sigwait");
      }
      switch (signal){
      case SIGUSR1:{
         /* inizializzo connessione con collector */
         struct sockaddr_un sa;
         int sfd;
         if((sfd = socket(AF_UNIX,SOCK_STREAM,0)) == -1){
		      perror("socket");
		      quit(EXIT_FAILURE);
	      }
         int size =strlen(sa.sun_path);
         while ((connect(sfd, (Sockaddr_t *)&sa, sizeof(sa))) == -1){
            if(errno == ENOENT && size > 0){ // bug fix per path socket
               size--;
               sa.sun_path[size] = '\0';
            }
            else{
               perror("sighandler - connect:");
               quit(EXIT_FAILURE);
            }
	      }
         /* scrivo sulla socket */
         write_msg("SIGNAL\0",0, sfd); 
      }
         break;
      case SIGINT:
      case SIGQUIT:
      case SIGTERM:
      case SIGHUP:{
         if(!isExit){   // se non è stato terminato dal main mando il segnale di interruzione 
            if(jobs) push_t(jobs,"INTERRUPT\0");
            addToThreadPool(worker_pool, worker, (void*)sock_path);
            isExit=1;
         }
      }
         break;
      default:;
      }

   }
   return (void*)0;
}



// gestione cleanup
void quit(int status){
   switch(status){ 
   case EXIT_FAILURE:   // Terminazione del programma con fallimento

      if(pid) kill(pid, SIGINT);             // segnalo terminazione al collector
      if (worker_pool != NULL) 
         destroyThreadPool(worker_pool,1);   // distruggo threadpool
      if(sock_fd) close(sock_fd);
      unlink(SOCKNAME);
      puts("quit - exit failure");
      _exit(EXIT_FAILURE);

   case EXIT_SUCCESS:   // Terminazione del programma con successo

      close(sock_fd);
      unlink(SOCKNAME);
      //puts("quit - exit success!");
      _exit(EXIT_SUCCESS);
   }

}
