#ifndef COLLECTOR_H_
#define COLLECTOR_H_


/**
 * @brief struttura dati del messaggio che il collector 
 *       riceve sulla connessione socket dai worker threads
 * 
 * @param filename l'absolute path del file su cui è eseguita la somma
 * @param sum la somma di tipo long dei valori contenuti nel file
*/
typedef struct msg{
   char* filename;
   long sum;
}Msg_t;

/**
 * @brief riceve un absolute path di un file e ne ricava il relative path
 * 
 * @param fullpath l'absolute path del file da tagliare
 * @param result il buffer contenente il relative path alla fine della funzione
*/
void cut_string(char* fullpath, char* result);

/**
 * @brief effettua listen e accept delle connessioni sul socket,
 *       utilizzando una select per selezionare i canali pronti. 
 *       Alla ricezione di END o INTERRUPT viene stampata la lista dei file ricevuti,
 *       ordinati in modo crescente in base alla somma, e effettuata l'uscita.
 *       In caso di ricezione di SIGUSR1 stampa la lista dei file ricevuti senza uscire. 
 * 
 * @param lfd la listening socket su cui il collector aspetta le comunicazioni
 * @returns 0 on success, -1 on failure
*/
int collector(int lfd);

/**
 * @brief libera la memoria allocata e chiude il socket
 * 
 * @param ret_code il codice di return (0 on success, -1 on failure)
*/
int cleanup(int ret_code);

/**
 * @brief legge tre messaggi secondo il protocollo di comunicazione
 *       stabilito tra collector e worker
 * 
 * @param msg il messaggio ricevuto dal worker ricostruito
 * @param sockfd la socket su cui è connesso il worker in comunicazione
   @returns 0 on success, -1 on error (sets errno)
*/
int read_msg(Msg_t* msg, int sockfd);

#endif
