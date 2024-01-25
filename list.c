#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "headers/list.h"
#include "headers/utils.h"



void insert(Node_t** head, Node_t* node){
   Node_t* prev = NULL;
   Node_t* curr = *head;

   // scorro la lista finché non trovo la posizione corretta
   while(curr!=NULL && curr->value < node->value){
      prev = curr;
      curr = curr->next;
   }

   if(prev==NULL){   // sono in cima
      node->next = curr;
      *head = node;
   }
   else{
      prev->next = node;
      node->next = curr;
   }
}

Deque_t* new_deque(int size){
   Deque_t* d = malloc(sizeof(Deque_t));
   if(d==NULL){
      perror("malloc");
      return NULL;
   }
      // inizializzazione
   d->head = NULL;
   d->tail = NULL;
   d->size = (size == 0) ? -1 : size;
   d->len = 0;
   pthread_cond_init(&(d->isFull), NULL);
   pthread_mutex_init(&(d->mtx), NULL);
   return d;
}

void push_t(Deque_t* d,char* fname){
   Node_t *newNode = malloc(sizeof(Node_t));
   
   // copio il filename nel nodo e inizializzo il resto
   newNode->filename = calloc(1, sizeof(char)*MAX_DIR_LEN);
   strncpy((*newNode).filename, fname, strlen(fname)+1);
   newNode->value = 0;  
   newNode->next = NULL;

   mtx_lock(&(d->mtx));
      while(d->len==d->size)   // se size è un numero negativo non si blocca mai
         pthread_cond_wait(&(d->isFull),&(d->mtx));   // aspetto signal da una pop
      
      if(d->len == 0) d->head = newNode;  // non ho altri nodi quindi è la testa
      else d->tail->next = newNode;       // lego la coda al nuovo nodo

      d->len++;   // incremento la lunghezza
      d->tail = newNode;
   mtx_unlock(&(d->mtx));
}

char* pop_h(Deque_t* d){
   mtx_lock(&(d->mtx));
      if(d->len == 0){     // non ho nodi nella coda
         mtx_unlock(&(d->mtx));
         //puts("No data to remove");
         return NULL;
      }
      Node_t *tmp = d->head;
      char* fname = calloc(1, sizeof(char)*MAX_DIR_LEN);
      strncpy(fname, tmp->filename, MAX_DIR_LEN);  // prelevo il nome del file

      if(d->head == d->tail){          // ho un solo nodo, quindi svuoto la queue
      d->head = NULL;                  // aggiornando i puntatori
      d->tail = NULL;
      }
      else d->head = d->head->next;    // altrimenti sposto il puntatore della testa
      d->len--;
   mtx_unlock(&(d->mtx));
   pthread_cond_signal(&(d->isFull));  // c'è almeno uno spazio nella coda

   // free del nodo prelevato
   free(tmp->filename);
   free(tmp);
   return fname; 
}

void print_list(Node_t *head){
   while(head != NULL){    // scorro la lista e stampo
      printf("%ld %s\n", head->value, head->filename);
      head = head->next; 
   }
}

void free_list(Node_t* head){
   Node_t* tmp;
   
   while(head != NULL){  // libero iterativamente la lista
      tmp = head;
      head = head->next;
      free(tmp->filename);
      free(tmp);
   }
}

void free_deque(Deque_t* d){
   mtx_lock(&(d->mtx));                   // blocco la struttura 
   free_list(d->head);                    // libero i nodi
   pthread_cond_destroy(&(d->isFull));    // distriggo la cond
   pthread_mutex_destroy(&(d->mtx));      // distruggo il mutex
   free(d);                               // libero la struttura
}