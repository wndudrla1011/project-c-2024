#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX_SIZE 100

int priority(char op);

struct stack
{
    int top;
    int size;
    int *arr;
    void (*push)(struct stack *, int);
    int (*pop)(struct stack *);
    bool (*isEmpty)(struct stack *);
};

bool isEmpty(struct stack *s)
{
    if (s->top < 0)
        return true;
    else
        return false;
}

void push(struct stack *s, int value)
{
    s->arr[++(s->top)] = value;
}

int pop(struct stack *s)
{
    if (!s->isEmpty(s))
        return s->arr[(s->top)--];
    return -1;
}

int priority(char op)
{
    switch (op)
    {
    case '(':
    case ')':
        return 0;
    case '+':
    case '-':
        return 1;
    case '*':
    case '/':
        return 2;
    }
    return -1;
}

int main(void)
{
    char *exp; // 입력
    char postfix[MAX_SIZE] = {'\0'};
    int result;
    exp = (char *)malloc(sizeof(char) * 100);

    printf("계산식을 입력한 후 Enter를 눌러주세요!\n");
    scanf("%s", exp);

    return 0;
}