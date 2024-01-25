#include "headers/utils.h"
#include <stdint.h>


int parse_int(const char* nptr){
    errno = 0;
    char* p; // buffer

    long n = strtol(nptr, &p,10);

    if(p==nptr || '\0'!= *p){ // non è (solo) un numero
        errno = EINVAL;
        fprintf(stderr, "Error: %s is NaN\n", nptr);
        return 0;
    }
    if(errno || n>INT32_MAX || n< INT32_MIN){ // overflow per int value
        if(errno == 0) errno = EOVERFLOW;
        perror("parse_int");
        return 0;
    }
    return (int)n;
}


int is_dat(char file[]){
   char* dat = ".dat";

    // file è più corto dell'estensione
   if(strlen(file)<strlen(dat)) return 0;

   int j = strlen(dat);

    // controllo dalla coda se gli ultimi caratteri 
    // corrispondono all'estensione
   for(int i= strlen(file); j>=0; i--){
      if(file[i]!=dat[j]) return 0;  // trovato carattere diverso
      j--;
   }
   return 1;
}

void sleep_ms(float ms){
    int ns = (int)(ms*1000000);
    struct timespec t = {0,ns};
    nanosleep(&t, NULL);
}

/* funzioni di controllo */

void mtx_lock(pthread_mutex_t *mtx_ptr){
    int err;
    if((err = pthread_mutex_lock(mtx_ptr)) != 0){
        puts("mutex_lock");
        errno = err;
        pthread_exit(&errno);
    }
    //else puts("mutex locked");     
}
void mtx_unlock(pthread_mutex_t *mtx_ptr){
    int err;
    if((err = pthread_mutex_unlock(mtx_ptr)) != 0){
        perror("mutex_unlock");
        errno = err;
        pthread_exit(&errno);
    }
    //else puts("mutex unlocked");
        
}
void thread_create(pthread_t *tid,const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg){
    int err;
    if((err = pthread_create(tid, attr, start_routine, arg)) != 0){
        fprintf(stderr,"FATAL_ERROR: thread_create\n");
        errno = err;
        pthread_exit(&errno);
    }
    //else printf("thread(%d) created", *tid);
}
void thread_join(pthread_t tid, void **retval){
    int err;
    if((err = pthread_join(tid, retval)) != 0){
        fprintf(stderr,"FATAL_ERROR: thread_join\n");
        errno = err;
        pthread_exit(&errno);
    }
    //else printf("thread(%ld) joined\n", (long)tid);
}
int bind_check(int* sfd, Sockaddr_t* sa){
    if((bind(*sfd,sa,sizeof(sa)))==-1){
        perror("bind_check");
        return -1;
    }
    return 0;
    //else puts("bind done");
}

/** Evita letture parziali,
 *  da assignment_11/conn.h.
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la lettura da fd leggo EOF
 *   \retval size se termina con successo
 */
int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=read((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;   // EOF
        left    -= r;
	bufptr  += r;
    }
    return size;
}

/** Evita scritture parziali,
 * da assignment_11/conn.h.
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la scrittura la write ritorna 0
 *   \retval  1   se la scrittura termina con successo
 */
int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;  
        left    -= r;
	bufptr  += r;
    }
    return 1;
}

