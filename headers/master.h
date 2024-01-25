#ifndef MASTER_H_
#define MASTER_H_


#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <signal.h>

#include "list.h"
#include "threadpool.h"

/** @brief extern variable contenente la coda master-worker */
extern Deque_t* jobs; 

/** @brief extern variable che segna l'uscita */
extern sig_atomic_t isExit; 


/** @brief struttura dati contenente i parametri passati al master thread
 * 
 * @param argc il numero di argomenti passati da riga di comando
 * @param argv gli argomenti passati da riga di comando
 * @param queue_length la lunghezza della bounded queue tra master e workers
 * @param wait_time il tempo di attesa tra una richiesta ai workers e la successiva
 * @param sockdir la directory del socket, da passare ai workers per effettuare la connessione
 * @param workers puntatore alla worker threadpool
*/
typedef struct master_args{
   int argc;
   char** argv;
   int queue_length;
   int wait_time;
   char* sockdir;
   threadpool_t** workers;
}Master_args_t;

/**
 * @brief funzione eseguita dal master thread. Invia i file passati per argomento alla pool di workers
 *       e esplora l'eventuale directory, passando i file contenuti all'interno.
 *       Alla fine invia un messaggio END, segnalando alla pool che il lavoro è finito.
 * 
 * @param arg (Master_args_t*) gli argomenti passati alla funzione (si fa riferimento al tipo per i dettagli)
*/
void* master(void* arg);

/** @brief esplora la directory ricevuta in ingresso alla ricerca di file .dat e li invia alla pool di workers.
 * 
 * @param dirname il nome della directory da visitare (si assume sia subdirectory della root)
 * @returns 0 success, 1 directory non visitata, -1 error
 */
int parsedir(char* dirname);

#endif
