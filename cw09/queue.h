#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct Node{
    Cart* cart;
    Node* next;
} typedef Node;

struct Queue{
    Node* head;
    Node* tail;
    int size;
}typedef Queue;


Queue createEmptyQueue();
void push(Queue* queue, Cart* cart);
Cart* head(Queue* queue);
void pop(Queue* queue);
