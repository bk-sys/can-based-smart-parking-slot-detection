#include<stdio.h>
#include<stdlib.h>

struct node
{
    int data;
    struct node *next;
};

struct node *front = NULL;
struct node *rear = NULL;

void enqueue(int data)
{
    struct node *newnode;

    newnode = (struct node*)malloc(sizeof(struct node));
    newnode->data = data;
    newnode->next = NULL;

    if(front == NULL)
    {
        front = rear = newnode;
    }
    else
    {
        rear->next = newnode;
        rear = newnode;
    }
}

void dequeue()
{
    struct node *temp;
    if(front == NULL)
    {
        printf("Queue Underflow\n");
        return;
    }
    temp = front;
    front = front->next;

    free(temp);
}

void peek()
{
    if(front == NULL)
    {
        printf("Queue is empty\n");
    }
    else
    {
        printf("Front element: %d\n", front->data);
    }
}

void display()
{
    struct node *temp;

    if(front == NULL)
    {
        printf("Queue is empty\n");
        return;
    }

    temp = front;
    printf("Queue elements: ");

    while(temp != NULL)
    {
        printf("%d ", temp->data);
        temp = temp->next;
    }

    printf("\n");
}

int main()
{
    enqueue(10);
    enqueue(20);
    enqueue(30);
    enqueue(40);
    enqueue(50);
    enqueue(60);
    
    display();

    dequeue();
    dequeue();
    display();

    peek();

    return 0;
}

