#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define MAX_SIZE 100

int op[MAX_SIZE];      // 연산자 대기 공간
int postfix[MAX_SIZE]; // 후위 표기식
int stack[MAX_SIZE];   // 연산 스택
int top = -1;
int top_postfix = -1;
int top_op = -1;
int head = 0;

bool isEmpty_postfix();
bool isEmpty_op();
void push(int value);
int pop();
void push_postfix(int value);
int pop_postfix();
void push_op(int value);
int pop_op();
void moveBack(int start_idx);

int main(void)
{
    char exp[20];

    printf("계산식을 입력한 후 Enter를 눌러주세요!\n");
    scanf("%s", exp);

    int idx = 0;

    while (exp[idx] != '\0')
    {
        if (exp[idx] - 48 > 0) // digit
        {
            push_postfix(exp[idx] - 48);
        }

        else // !digit
        {
            if (exp[idx] - 48 == -6 || exp[idx] - 48 == -1) // 곱셈 또는 나눗셈
            {
                int add_sub_idx = -1; // 덧셈 또는 뺄셈이 존재 여부 체크

                if (isEmpty_op()) // 대기 중인 연산자가 없으면
                {
                    push_op(exp[idx] - 48);
                    continue;
                }

                // 덧셈 또는 뺄셈 검색
                for (int i = 0; i <= top_op; i++)
                {
                    if (op[i] == -5 || op[i] == -3)
                    {
                        add_sub_idx = i;
                        break;
                    }
                }

                if (add_sub_idx == -1) // 덧셈 또는 뺄셈 존재x
                    op[++top_op] = exp[idx] - 48;

                else // 덧셈 또는 뺄셈 존재
                {
                    moveBack(add_sub_idx);
                    op[add_sub_idx] = exp[idx];
                }
            }

            else if (exp[idx] - 48 == -5 || exp[idx] - 48 == -3) // 덧셈 또는 뺄셈
            {
                push_op(exp[idx] - 48);
            }

            else if (exp[idx] - 48 == -8) // open bracket
            {
                push_op(exp[idx] - 48);
            }

            else if (exp[idx] - 48 == -7) // close bracket
            {
                // op -> postfix
                while (op[top_op] != -8)
                    push_postfix(pop_op());
                pop_op();
            }

            else
            {
                printf("수식 입력이 잘못되었습니다.\n");
            }
        }

        idx++;
    }

    if (!isEmpty_op())
        push_postfix(pop_op());

    for (int i = 0; i <= top_postfix; i++)
    {
        printf("%d\n", postfix[i]);
    }

    for (int i = 0; i <= top_postfix; i++)
    {
        if (postfix[i] >= 0) // digit
            push(postfix[i]);
        else // op
        {
            int rv = pop();
            int lv = pop();

            if (postfix[i] == -5)
                push(lv + rv);
            else if (postfix[i] == -3)
                push(lv - rv);
            else if (postfix[i] == -6)
                push(lv * rv);
            else
                push(lv / rv);
        }
    }

    printf("계산결과 : \t%d\n", stack[top]);

    return 0;
}

bool isEmpty_postfix()
{
    if (top_postfix < 0) // empty
        return true;
    else
        return false;
}

bool isEmpty_op()
{
    if (top_op < 0) // empty
        return true;
    else
        return false;
}

void push(int value)
{
    stack[++top] = value;
}

int pop()
{
    return stack[top--];
}

void push_postfix(int value)
{
    postfix[++top_postfix] = value;
}

int pop_postfix()
{
    return postfix[top_postfix--];
}

void push_op(int value)
{
    op[++top_op] = value;
}

int pop_op()
{
    return op[top_op--];
}

void moveBack(int start_idx)
{
    for (int i = top_op; i >= start_idx; i--)
    {
        op[i + 1] = op[i];
    }
}