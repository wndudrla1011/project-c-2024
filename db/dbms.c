#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "db.h"
#include "table.h"
#include "domain.h"
#include "data.h"
#include "./hooks/create_table.h"
#include "./util/substring.h"

#define MAX_COLUMN 20      // 최대 속성 값 개수
#define MAX_INPUT 100      // 최대 입력 값 길이
#define MAX_CADINALITY 200 // 최대 튜플 개수

char op[] = {'<', '>', '=', '!'};
char *search_op[] = {"<", ">", "=", "!"};

int main(void)
{
    DB *db = NULL;
    DB *head = NULL; // DB head
    Table *table = NULL;
    Domain *domain = NULL;
    Data *data = NULL;
    char input[MAX_INPUT]; // 입력 값
    char *command;         // 명령어

    while (1)
    {
        fgets(input, MAX_INPUT, stdin);

        command = strtok(input, " ");

        if (!strcasecmp(command, "show"))
        {
            command = strtok(NULL, ";");

            if (!strcasecmp(command, "databases")) // Query > show databases
            {
                if (get_cnt_db(head) == 0)
                {
                    printf("No database exist\n");
                    continue;
                }

                else
                {
                    print_all_db(head); // 생성된 DB 출력
                }
            }

            else if (!strcasecmp(command, "tables")) // Query > show tables
            {
                if (db->tcnt == 0) // 생성한 Table이 없을 때
                {
                    printf("Entry empty\n");
                    continue;
                }
                else
                {
                    print_all_table(db); // 생성된 Table 출력
                }
            }
        }

        else if (!strcasecmp(command, "create"))
        {
            command = strtok(NULL, " ");

            if (!strcasecmp(command, "database")) // Query > create database
            {
                command = strtok(NULL, ";");

                if (get_cnt_db(head) == 0) // 첫 DB 생성
                {
                    db = init_db(); // DB 초기화
                    head = db;
                }

                if (read_db(db, command) != NULL) // 같은 이름의 DB가 이미 존재하는 경우
                {
                    printf("Can't create database '%s'; database exists\n", command);
                    continue;
                }

                add_db(db, command); // 연결 리스트 -> New DB
                printf("Query Success!\n");
            }

            else if (!strcasecmp(command, "table")) // Query > create table
            {
                command = strtok(NULL, "("); // table name

                if (db == head) // use database 를 하지 않은 상태
                {
                    printf("No database selected\n");
                    continue;
                }

                if (db->thead == NULL) // Table head 없음
                {
                    table = init_table(db); // Table 초기화
                    db->thead = table;
                }

                if (read_table(db->thead, command) != NULL) // 같은 이름의 Table이 이미 존재하는 경우
                {
                    printf("Table '%s' already exists\n", command);
                    continue;
                }

                create_table(command, db, table, domain);

                printf("Query Success!\n");
            }
        }

        else if (!strcasecmp(command, "use")) // Query > use database
        {
            command = strtok(NULL, ";"); // DB name

            db = read_db(head, command); // 찾은 DB로 이동

            if (db == NULL)
                printf("Unknown database '%s'\n", command);
            else
                printf("Database changed\n");
        }

        else if (!strcasecmp(command, "drop"))
        {
            command = strtok(NULL, " ");

            if (!strcasecmp(command, "database")) // Query > drop database
            {
                command = strtok(NULL, ";");

                if (read_db(head, command) == NULL)
                {
                    printf("Now current DB is NULL\nPlease use another DB\n");
                    continue;
                }

                delete_db(head, command);
                printf("Query Success!\n");
            }

            else if (!strcasecmp(command, "table")) // Query > drop table
            {
                command = strtok(NULL, ";");

                delete_table(db, command);
            }
        }

        else if (!strcasecmp(command, "desc")) // Query > desc table
        {
            command = strtok(NULL, ";"); // Table name

            if (db == head) // use database 를 하지 않은 상태
            {
                printf("No database selected\n");
                continue;
            }

            table = read_table(db->thead, command); // 찾는 테이블로 이동

            if (table == NULL) // Not found Table
            {
                printf("Table '%s' doesn't exist\n", command);
                continue;
            }

            print_all_domain(table);
        }

        else if (!strcasecmp(command, "insert")) // Query > insert table
        {
            if (db == head) // use database 하지 않은 상태
            {
                printf("No database selected\n");
                continue;
            }

            command = strtok(NULL, " "); // into

            int cnt = 0; // cnt: '(' 개수

            for (int i = 0; i < sizeof(input); i++) // insert 쿼리에 columns 목록 존재 여부 확인
            {
                if (input[i] == '(')
                    cnt++;
            }

            // command = 테이블명
            if (cnt > 1) // columns(fields) 입력
                command = strtok(NULL, "(");
            else // columns(fields) 생략
                command = strtok(NULL, " ");

            table = read_table(db->thead, command); // 테이블명으로 테이블 찾기

            if (table == NULL) // Not found Table
            {
                printf("Table '%s' doesn't exist\n", command);
                continue;
            }

            command = strtok(NULL, "(");

            int degree = 0;          // 속성 개수
            char *values[MAX_INPUT]; // 토큰화된 입력 데이터
            char *token;
            char *pos;    // 첫 single quote가 등장하는 위치 == 문자열 데이터 시작 위치
            char *unpack; // substring 결과

            while ((token = strtok(NULL, ",);")) != NULL) // 토큰화
            {
                values[degree++] = token;
            }

            table->degree = degree - 1; // 열 개수 입력
            table->cadinality++;        // 행 개수 추가

            for (int i = 0; i < table->degree; i++)
            {
                if ((pos = strstr(values[i], "\'")) != NULL) // 문자열 데이터
                {
                    unpack = substring(1, strlen(pos) - 2, pos); // 맨 앞뒤 single quote 제거
                    values[i] = unpack;
                }
            }

            // >>>>>>>>>>>>>>>>>>>> Parsing Data

            domain = table->dhead->next; // 첫 번째 column 이동

            for (int i = 0; i < table->degree; i++) // 각 속성에 값 넣기
            {
                if (i == 0)
                    add_bottom_data(domain->head, values[i]);
                else
                    add_right_data(find_bottom_data(domain->head), values[i]); // 데이터 추가 (열 방향)
            }

            printf("Query Success!\n");

            // >>>>>>>>>>>>>>>>>>>>> Insert Data
        }

        else if (!strcasecmp(command, "select")) // Query > select ~ from
        {
            if (db == head)
            {
                printf("No database selected\n");
                continue;
            }

            int cnt = 0;               // token count
            int cnt_cols = 0;          // column 개수
            int cnt_cons = 0;          // 조건 개수
            int pos_tname = 0;         // table name 위치
            int pos_cons = 0;          // conditions 시작 위치
            int flag = 0;              // 0: none, 1: and, 2: or
            char *columns[MAX_COLUMN]; // 모든 column
            char wheres[MAX_INPUT];    // 조건절
            char *wtokens[MAX_INPUT];  // 모든 조건 token
            char *tokens[MAX_INPUT];   // 모든 token
            char *token = NULL;

            wheres[0] = '\0'; // 조건절 초기화

            while ((token = strtok(NULL, ", ;")) != NULL) // Tokenizer
            {
                if (!strcmp(token, "from"))
                    pos_tname = cnt;

                if (!strcmp(token, "where"))
                    pos_cons = cnt;

                tokens[cnt++] = token;
            }

            for (int i = 0; i < pos_tname; i++) // save columns
            {
                columns[i] = tokens[i];
            }

            ++pos_tname;
            free(token);
            token = NULL;

            for (int i = pos_cons + 1; i < cnt - 1; i++) // create where clause
            {
                if (!strcmp(tokens[i], "or") || !strcmp(tokens[i], "and"))
                {
                    strcat(wheres, " ");
                    strcat(wheres, tokens[i]);
                    strcat(wheres, " ");
                }
                else
                    strcat(wheres, tokens[i]);
            }

            token = strtok(wheres, " ");
            wtokens[0] = token;
            cnt_cons++;

            while ((token = strtok(NULL, " ")) != NULL)
            {
                if (!strcmp(token, "or"))
                    flag = 2;
                else if (!strcmp(token, "and"))
                    flag = 1;

                wtokens[cnt_cons++] = token;
            }

            char *col1 = NULL; // 조건1 -> 속성
            char *val1 = NULL; // 조건1 -> 값
            char op1;          // 조건1 -> 연산자
            char *col2 = NULL; // 조건2 -> 속성
            char *val2 = NULL; // 조건2 -> 값
            char op2;          /// 조건2 -> 연산자

            if (flag > 0) // 조건문 2개
            {
                for (int i = 0; i < sizeof(search_op) / sizeof(char *); i++)
                {
                    if (strstr(wtokens[0], search_op[i]) != NULL)
                    {
                        op1 = op[i];
                        col1 = strtok(wtokens[0], search_op[i]);
                        val1 = strtok(NULL, search_op[i]);
                    }

                    if (strstr(wtokens[2], search_op[i]) != NULL)
                    {
                        op2 = op[i];
                        col2 = strtok(wtokens[2], search_op[i]);
                        val2 = strtok(NULL, search_op[i]);
                    }
                }
            }

            else // 조건문 1개
            {
                for (int i = 0; i < sizeof(search_op) / sizeof(char *); i++)
                {
                    if (strstr(wtokens[0], search_op[i]) != NULL)
                    {
                        op1 = op[i];
                        col1 = strtok(wtokens[0], search_op[i]);
                        val1 = strtok(NULL, search_op[i]);
                    }
                }
            }

            // >>>>>>>>>>>>>>>>>>>>> Parsing where

            cnt_cols = pos_tname - 1; // counting cols

            int result = 0;

            table = read_table(db->thead, tokens[pos_tname]); // find table

            if (!strcmp(columns[0], "*")) // select all
            {
                domain = table->dhead->next; // Move first column (head next)

                data = domain->head->next; // Move head data

                while (data != NULL)
                {
                    if (flag > 0) // 다중 조건
                    {
                        result = find_multi_data(table, domain, data, col1, val1, op1, col2, val2, op2, flag);
                    }

                    else // 단일 조건
                    {
                        result = find_single_data(table, domain, data, col1, val1, op1);
                    }

                    if (result) // 조건에 부합
                    {
                        print_tuple(data);
                    }

                    domain = table->dhead->next; // Move first column (head next)
                    data = data->next;           // next data (next tuple)
                }
            }

            // >>>>>>>>>>>>>>>>>>>>> Select all

            else // select cols
            {
                domain = table->dhead->next; // Move first column (head next)

                data = domain->head->next; // Move head data

                while (data != NULL)
                {
                    if (flag > 0) // 다중 조건
                    {
                        result = find_multi_data(table, domain, data, col1, val1, op1, col2, val2, op2, flag);
                    }

                    else // 단일 조건
                    {
                        result = find_single_data(table, domain, data, col1, val1, op1);
                    }

                    if (result) // 조건에 부합
                    {
                        printf("+--------------------------------------+\n");
                        for (int i = 0; i < cnt_cols; i++) // 조건에 부합하는 Tuple 출력
                        {
                            print_data(domain, data, columns[i]);
                        }
                        printf("\n+--------------------------------------+\n");
                    }

                    domain = table->dhead->next; // Move first column (head next)
                    data = data->next;           // next data (next tuple)
                }
            }

            // >>>>>>>>>>>>>>>>>>>>> Select cols
        }

    } // while(1)

    return 0;
}