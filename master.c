#include "headers/utils.h"
#include "headers/master.h"
#include "headers/worker.h"
#include "headers/list.h"

int isExit=0;     // extern variable 
Deque_t* jobs;    // extern variable
Master_args_t* args;

void* master(void* arg){
   char buf[MAX_DIR_LEN];
   args = (Master_args_t*) arg;
      // inizializzo la queue master-worker
   jobs = new_deque(args->queue_length);
   
   char** argv = args->argv;
   int dir_i = 0;    // se non ho una directory non chiamo il parsing

      // scorro argv per ottenere file e directory
   for(int i=1;(i<(args->argc))&&(!isExit) ;i++){

         // ignora le opzioni con i dati già analizzati
      if(strcmp(argv[i],"-n")==0 || strcmp(argv[i],"-q")==0 || strcmp(argv[i],"-t")==0) i++;

      else if(strcmp(argv[i], "-d")==0){  // caso directory
         i++;
         if(strlen(argv[i])>MAX_DIR_LEN)
            fprintf(stderr, "Directory troppo lunga. Non verrà considerata nessuna directory.\n");
         else dir_i=i;     // in caso la directory è valida da analizzare salva l'indice
      }
      else{    // caso file
            // controllo se il file è .dat e in caso genero absolute path e passo sulla queue
         if(!is_dat(argv[i])) printf("file <%s> ignorato - non è un file .dat.\n", argv[i]);
         else{
            char* filepath = realpath(argv[i], buf);
            push_t(jobs,filepath);
            //printf("master - realpath: %s\n", filepath);
            addToThreadPool(*args->workers, worker, (void*)args->sockdir);
            sleep_ms(args->wait_time);
         }
      }
   }
      // directory parsing dopo file per generare gli absolute paths correttamente
   if(!isExit && dir_i) parsedir(argv[dir_i]);
   
   if(!isExit){   // ho finito, invio segnale di end
      push_t(jobs,"END\0");
      addToThreadPool(*args->workers, worker, (void*)args->sockdir);
      sleep_ms(args->wait_time);
   }
   return (void*)0;
}


int parsedir(char* dirname){
   char buf[MAX_DIR_LEN];
   if(strlen(dirname)>MAX_DIR_LEN){
      fprintf(stderr, "Directory <%s> troppo lunga. Non verrà visitata.\n", dirname);
      return 1;       
   }
   DIR* dir = opendir(dirname);
   if(dir == NULL){ 
      fprintf(stderr, "Directory <%s> non valida. Non verrà visitata.\n", dirname);
      return 1; 
   }

   if(chdir(dirname)==-1){ 
      fprintf(stderr,"chdir <%s>: %s\n", dirname, strerror(errno));
      return -1;
   }
   struct dirent *entry;
   struct stat statbuf;
   while((entry=readdir(dir))!= NULL && !isExit){
            // ottengo informazioni sulla entry nella directory
         lstat(entry->d_name,&statbuf);

            // se è una directory effettuo una chiamata ricorsiva per visitarla
         if(S_ISDIR(statbuf.st_mode)){
               // ignoro . e ..
               if(strcmp(".",entry->d_name)==0 || strcmp("..",entry->d_name)==0) continue;
               else{
                  if(parsedir(entry->d_name) != -1){  // chiamata ricorsiva
                     if (chdir("..") == -1) return 1;
                  }
               }   
         } 
            // se è un file controllo se è .dat e in caso aggiungo alla queue
         else if(S_ISREG(statbuf.st_mode)){
            if(is_dat(entry->d_name)){
               char* filepath = realpath(entry->d_name, buf);
               push_t(jobs,filepath);
               //printf("master - realpath: %s\n", filepath); 
               addToThreadPool(*(args->workers), worker, (void*)args->sockdir);
               sleep_ms(args->wait_time);
            }
            else printf("file <%s> ignorato - non è un file .dat.\n", entry->d_name);
            
         }
   }
   closedir(dir);
   return 0;
}

