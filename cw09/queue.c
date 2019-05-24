#include "queue.h"
#include "rollercoaster.h"

Queue createEmptyQueue(){
    Queue queue;
    queue.head = NULL;
    queue.tail = NULL;
    queue.size = 0;
    return queue;
}

void push(Queue* queue, Cart* cart){
    Node* node = malloc(sizeof(node));
    node->cart = cart;
    node->next = NULL;

    if(queue->size == 0){
        queue->head = node;
        queue->tail = node;
        queue->size++;
    }
    else{
        queue->tail->next = node;
        queue->size++;
    }
}

Cart* head(Queue* queue){
    return queue->head->cart;
}

void pop(Queue* queue){
    if(queue->size == 0) return;
    Node* tmp = queue->head;
    queue->head = queue->head->next;
    free(tmp);
    queue->size--;
}

