#ifndef UTILS_H
#define UTILS_H

/* include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

/* lunghezza massima delle directories */
#ifndef MAX_DIR_LEN
#define MAX_DIR_LEN 255
#endif

/* nome della socket */
#ifndef SOCKNAME
#define SOCKNAME "farm.sck"
#endif

/* typedef */
typedef struct sockaddr Sockaddr_t;
typedef struct sigaction Sigact_t;



/**
 *  @brief Riceve una stringa e controlla se il valore ricevuto è un intero (base 10).
 * 
 *  @param nptr la stringa da controllare
 *  @returns l'intero on success, -1 on failure (sets errno)
 */
int parse_int(const char* nptr);

/**
 * @brief controlla se il file ricevuto ha estensione .dat
 * 
 * @param file il file da controllare
 * @returns 1 se il file è .dat, 0 altrimenti
*/
int is_dat(char file[]);

/**
 * @brief effettua una sleep per il numero di ms indicati 
 * 
 * @param ms il numero di millisecondi
*/
void sleep_ms(float ms);

/**
 * @brief funzioni di controllo per lock e unlock di mutex, creazione e join di thread.
 * 
 * returns on success, exits on failure
*/
void mtx_lock(pthread_mutex_t *mtx_ptr);
void mtx_unlock(pthread_mutex_t *mtx_ptr);
void thread_create(pthread_t *tid,const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
void thread_join(pthread_t tid, void **retval);

/**
 * @brief funzione di controllo per l'operazione di bind
 * 
 * @return 0 on success, -1 on failure (sets errno)
*/
int bind_check(int* sfd, Sockaddr_t* sa);

/** funzioni tratte da assignment 11 - conn.h. 
 * @brief evitano letture e scritture parziali
 * 
 * @returns 1 on success, 0 per EOF, -1 on error (sets errno) 
 */
int readn(long fd, void *buf, size_t size);
int writen(long fd, void *buf, size_t size);

#endif
