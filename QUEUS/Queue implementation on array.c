#include<stdio.h>
#define max 5

int queue[max];
int front=-1;
int rear=-1;

int isfull()
{
    if(rear == max-1)
    return 1;
    else 
    return 0;
}

int isempty()
{
    if(front == -1 || front>rear )
    return 1;
    else
    return 0;
}

void enqueue(int data)
{
    if(isfull()){
    printf("Queue overflow: ");
    return;
    }
    if(front = -1)
    front =0;
    
    rear++;
    queue[rear]=data;
}

void dequeue()
{
    if(isempty()){
    printf("Queue Underflow:");
    return;
    }
    front++;
}
void peek()
{
    if (front == -1)
    {
        printf("Queue is empty\n");
    } 
    else
    {
        printf("The front is %d: ",queue[front]);
    }
}

void display()
{
    if (front == -1)
    {
        printf("Queue is empty\n");
    } 
    else
    {
        printf("Queue elements: ");
        for (int i=front;i<=rear;i++) 
        {
            printf("%d ",queue[i]);
        }
        printf("\n");
    }
}

int main()
{
    enqueue(10);
    enqueue(20);
    enqueue(30);
    enqueue(40);
    display();
    printf("\nafter dequeueing: ");
    
    dequeue();
    dequeue();
    display();
    
    peek();
    
    return 0;
}
