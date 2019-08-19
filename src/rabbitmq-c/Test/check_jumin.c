#include <stdio.h>
#include <errno.h>

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

//*******************************주민번호 유효성 검사*************************************//

    for(i = 0;i < 13;i++)                    //index를 12까지 반복문을 수행
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
         /******올바른 주민 등록 번호******/
         /* amqp_rpc_sendstring_client.c 에서 messeage보내는 부분에 jumin[15] 넣어서 전송*/
         return jumin[15];
     }
     else
     {
         /******잘못된 주민 등록 번호******/
         /* 처리하기 */
     }
}
/*/////////////////////////////////////////////////////////////////////////////////////////*/
int main()
{
    char jumin[15];
    check_Jumin(jumin);
}
