#ifndef LIST_H_
#define LIST_H_

#include <pthread.h>

/**
 * @brief struttura dati nodo, usata sia per lista ordinata che in nella deque
 * 
 * @param value valore contenente la somma dei long nel file abbinato
 * @param filename il relative path del rispettivo file 
 * @param next puntatore al nodo successivo nella struttura dati
*/
typedef struct node{
   long value;
   char* filename;
   struct node* next;
}Node_t;

/**
 * @brief struttura dati double-ended queue, thread-safe
 * 
 * @param head puntatore alla testa della queue
 * @param tail puntatore alla coda della queue
 * @param size dimensione della queue, <=0 per rimuovere il limite
 * @param len lunghezza attuale della queue
 * @param isFull segnale che comunica la pienezza o meno della queue
 * @param mtx il mutex legato alla queue
*/
typedef struct deque{
   Node_t *head;
   Node_t *tail;
   int size;
   unsigned int len;
   pthread_cond_t isFull;
   pthread_mutex_t mtx;
}Deque_t;

/**
 * @brief inserimento ordinato in lista
 * 
 * @param head puntatore alla testa della lista
 * @param node il nodo da aggiungere nella lista
*/
void insert(Node_t** head, Node_t* node);

/**
 * @brief crea e inizializza una nuova double-ended queue.
 * 
 * @param size la dimensione della queue
 * @returns la nuova queue on success, NULL on error
*/
Deque_t* new_deque(int size);

/**
 * @brief crea il nodo in cui contenere il filename e lo inserisce in fondo alla coda.
 * 
 * @param d la double-ended queue in cui inserire il nodo
 * @param fname il nome del file per cui creare il nodo da inserire
*/
void push_t(Deque_t* d, char* fname);

/**
 * @brief rimuove l'elemento in testa alla queue e aggiorna i rispettivi puntatori.

 * @param d la double-ended queue da cui rimuovere la testa
 * @returns filename on success, NULL if empty
*/
char* pop_h(Deque_t* d);

/**
 * @brief stampa in modo ordinato la lista ricevuta
 * @param head puntatore alla testa della lista
*/
void print_list(Node_t *head);

/**
 * @brief effettua la free iterativa della lista ricevuta
 * @param head puntatore alla testa della lista
*/
void free_list(Node_t *head);

/**
 * @brief effettua la free della double-ended queue ricevuta. Chiama free_list per liberare 
 *       la queue effettiva, poi effettua la destroy del mutex e la free della struttura stessa.
 * @param d la double ended queue da liberare
*/
void free_deque(Deque_t* d);


#endif
