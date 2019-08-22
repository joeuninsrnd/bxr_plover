#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    regex_t    preg;
    char       *pattern = "\b(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-([1-4][0-9]{6})\b";
    int        rc;
    size_t     nmatch = 2;
    regmatch_t pmatch[2];
    char       err_buffer[100];
    FILE* fp = NULL;
    char       buffer[255];

    fp = fopen("/home/joeun/parsejumin/jumin.txt", "r");

    if(NULL == fp)
    {
        return 1;
    }

    while (feof(fp) == 0)
    {
        fread(buffer, sizeof(char), 255, fp);

        if (0 != (rc = regcomp(&preg, pattern, 0)))
        {
            printf("regcomp() failed, returning nonzero (%d)\n", rc);
            exit(EXIT_FAILURE);
        }

        if (0 != (rc = regexec(&preg, buffer, nmatch, pmatch, 0)))
        {
            regerror(rc, &preg, err_buffer, 100);

            printf("regcomp() failed with '%s'\n", err_buffer);
            printf("Failed to match '%s' with '%s',returning %d.\n", buffer, pattern, rc);

            exit(EXIT_FAILURE);
        }

        else
        {
            printf(" \"%.*s\" \n", pmatch[0].rm_eo - pmatch[0].rm_so, &buffer[pmatch[0].rm_so]);
            printf("&buffer[pmatch[0].rm_eo]:%s\n", &buffer[pmatch[0].rm_eo]);
            printf("&buffer[pmatch[0].rm_so]:%s\n", &buffer[pmatch[0].rm_so]);
        }

        memset(buffer, 0, 255);
    }


    fclose(fp);
    regfree(&preg);
}

/*
char file_Parsing(char jumin[])
{
    FILE *fp = fopen("/home/joeun/parsejumin/jumin.txt", "r");

    if(fp == NULL)
    {
        printf("Can not open files. \n");
    }

    fscanf(fp, "%s", jumin);

    fclose(fp);
    return jumin[15];
}

char check_Jumin(char jumin[])
{
    int i, result, sum = 0;

    file_Parsing(jumin);

//-/////////////////////주민번호 유효성 검사//////////////////////////////////////-//

    for(i = 0; i < 13; i++)                    //index를 12까지 반복문을 수행
    {
        if(i < 6)                            //index 0~5까지는 index+2를 해서 곱하고 더한다
        {
            sum += (jumin[i]-'0') * (i+2);
        }
        if(i > 6 && i < 9)                   //inedx 7~8까지는 index+1를 해서 곱하고 더한다
        {
            sum += (jumin[i]-'0') * (i+1);
        }
        if(i > 8)                            //index 9~12까지는 index-7를 해서 곱하고 더한다
        {
            sum += (jumin[i]-'0') * (i-7);
        }
    }
        result = 11 - (sum % 11);

     if(result >= 10)
     {
         result -= 10;
     }
     else
         result;

     if(result == (jumin[13]-'0'))           //result값이 주민번호 마지막과 같은지 확인.
     {
         //-//////올바른 주민 등록 번호//////-//
         // amqp_rpc_sendstring_client.c 에서 messeage보내는 부분에 jumin[15] 넣어서 전송//
         printf("%s\n", jumin);
         return jumin[15];
     }
     else
     {
         //-/////잘못된 주민 등록 번호/////-//
         //- 처리하기 -//
     }

}
//-///////////////////////////////////////////////////////////////////////////////////////-//
int main()
{
    char jumin[15];
    check_Jumin(jumin);
}
*/
