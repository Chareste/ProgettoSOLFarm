#ifndef WORKER_H_
#define WORKER_H_


/**
 * @brief funzione eseguita dai worker thread. Apre il file binario 
 *       in cima alla coda master-worker, effettua la somma ponderata
 *       dei long all'interno e invia il risultato al collector
 * 
 * @param arg (char*)sockdir, l'absolute path del socket
*/
void worker(void* arg);

/**
   @brief invia tre messaggi  secondo il protocollo di comunicazione 
         stabilito tra collector e worker
   
   @param filename l'absolute path del file 
   @param sum la somma dei long contenuti nel file
   @param sockfd il socket aperto dal worker
   @returns 0 on success, -1 on error (sets errno)
*/
int write_msg(char* filename, long sum, int sockfd);

#endif
