#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ERROR_MSG 0x1000

/* Compile the regular expression described by "regex_text" into
   "r". */

static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);

    if (status != 0)
    {
    char error_message[MAX_ERROR_MSG];

    regerror (status, r, error_message, MAX_ERROR_MSG);

    printf ("Regex error compiling '%s': %s\n", regex_text, error_message);

    return 1;
    }

    return 0;
}

/*
  Match the string in "to_match" against the compiled regular
  expression in "r".
 */

static char match_regex (regex_t * r, const char * to_match)
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 100;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    while (1)
    {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        char data_buffer[16];

        if (nomatch)
        {
            printf ("No more matches.\n");
            return nomatch;
        }

        for (i = 0; i < n_matches; i++)
        {
            int start, finish;
            char sub_buffer[16];


            if (m[i].rm_so == -1)
            {
                break;
            }

            start  = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);

            if (i == 0) //주민번호 검사 통과한 부분
            {
                //printf("%.*s \n", (finish - start), to_match + start); //(finish - start), to_match + start <-이값을 변수에 저장해서 리턴보내
                for (int j = 0; j < 14; j++)
                {
                    sub_buffer[j] = *(to_match + start + j);
                    printf("j:%d  %s\n", j, sub_buffer);
                }
                printf("data: %s\n\n", sub_buffer);
                printf("data: %s\n\n", data_buffer);

                memset(sub_buffer, 0, 16);
            }
        }

        p += m[0].rm_eo;
    }

    return 0;
}

char check_file()
{
    FILE* fp = NULL;
    regex_t r;
    char buffer[255];
    const char * regex_text;
    const char * find_text;

    fp = fopen("/home/joeun/parsejumin/jumin.txt", "r");

    if(NULL == fp)
    {
        return 1;
    }

    regex_text = "([0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6})";

    while (feof(fp) == 0)
    {
        fread(buffer, sizeof(char), 255, fp);
        compile_regex (& r, regex_text);
        find_text = buffer; // 버퍼 크기만큼 읽어서 find_text에 넣고 검사
        match_regex (& r, find_text);
    }

    memset(buffer, 0, 255);
    regfree (& r);
    fclose(fp);


}

int main ()
{
    check_file();

    return 0;
}

/*
int main()
{
    regex_t    preg;
    char       *pattern = "/^01([0|1|6|7|8|9]?)-?([0-9]{3,4})-?([0-9]{4})$/g";
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
*/

