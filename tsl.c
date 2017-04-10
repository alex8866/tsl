/*
 * Copyright (C), 2012-2013, Alex
 *
 * Filename: tsl.c
 *
 * Author: Alex
 *
 * Version: 0.0
 *
 * Date: 2013-10-23
 *
 * Description: This C program is used to translate chinese to english.
 *
 * Others:
 *
 * Function list: 
 *
 * History:
 *   1. Author: 
 *      Date:
 *      Modification:
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include <sys/stat.h>
#include <sys/types.h>

#include<curl/curl.h>
#include<curl/easy.h>
#include<iconv.h>

#define BUFFER 5000

char result[BUFFER];
char src[24]="en";
char dest[24]="zh_CN";
short  translate_flag=0;

int get_content(char *html_content)
{
    char *ss_start = NULL;
    char *se_end = NULL;

    char *ss = "Color='#fff'\">";
    char *se = "</span>";

    ss_start = strstr(html_content, ss);

    if(ss_start != NULL&&translate_flag==0)
    {
        se_end = strstr(ss_start, se);
        ss_start += strlen(ss);
        strncat(result, ss_start, se_end - ss_start); 
        translate_flag++;
        return 0;
    }

    return -1;
}

size_t write_data(void *ptr,size_t size,size_t nmemb,void *stream)
{
    FILE *fp;
    int rs;
    get_content((char *)ptr);
    fp=fopen("/dev/null","w");
    rs=fwrite(ptr,size,nmemb,fp);
    fclose(fp);

    return rs;
}

int URLEncode(const char* str, const int strSize, char* result, const int resultSize) 
{
    int i;
    int j = 0;
    char ch;

    if ((str == NULL) || (result == NULL) || (strSize <= 0) || (resultSize <= 0)) {
        return 0;
    }

    for (i=0; (i<strSize) && (j<resultSize); i++) {
        ch = str[i];
        if ((ch >= 'A') && (ch <= 'Z')) {
            result[j++] = ch;
        } else if ((ch >= 'a') && (ch <= 'z')) {
            result[j++] = ch;
        } else if ((ch >= '0') && (ch <= '9')) {
            result[j++] = ch;
        } else if(ch == ' ' || ch == '\n' || ch == '\r'|| ch == '\t'){
            result[j++] = '+';
        } else {
            if (j + 3 < resultSize) {
                sprintf(result+j, "%%%02X", (unsigned char)ch);
                j += 3;
            } else {
                return 0;
            }
        }
    }

    result[j] = '\0';
    return j;
} 

int translate_engine(char *inputText)
{
    char urlstr[BUFFER];
    char langpair[256];
    char engineUrl[256];
    CURL *curl;

    memset(urlstr, 0, BUFFER);

    int inputTextLen = sizeof(char) * 3 * strlen(inputText);
    char *inputText_encoded = malloc(inputTextLen);

    sprintf(langpair, "%s|%s", src, dest);
    sprintf(engineUrl, "%s", "http://translate.google.cn/translate_t");

    URLEncode(inputText, strlen(inputText), inputText_encoded, inputTextLen + 2);
    sprintf(urlstr, "%s?text=%s&langpair=%s", engineUrl, inputText_encoded, langpair);
    //  fprintf(stdout, "urlstr: %s\n", urlstr);

    curl_global_init (CURL_GLOBAL_ALL);
    curl = curl_easy_init ();
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
    curl_easy_setopt (curl, CURLOPT_URL, urlstr);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_data);	
    curl_easy_perform (curl);
    curl_easy_cleanup (curl);

    return 0;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t *inlen,char *outbuf,size_t *outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,BUFFER);
    if (iconv(cd,pin,inlen,pout,outlen)==-1) return -1;
    iconv_close(cd);
    return 0;
}

int main(int argc,char *argv[])
{

    if (argc >1 && !strcmp(argv[1], "--help"))
    {
        printf("Usage: tsl [OPTIONS] english-word\n");
        printf("Translate english to chinese.\n");
        printf("\n");
        printf("--help  print this usage\n");
        printf("for more infomation type \"man tsl\" on your shell");
        printf("\n");
        exit(0);
    }

    char result_converted[BUFFER] = {0};
    int pfds[2];
    int ret = 0;
    char buf[BUFFER] = {0};
    size_t insize=50;
    size_t outsize=BUFFER;

    if (pipe(pfds) == 0 && argc == 1)
    {
        ret = fork();
        if (ret == 0)
        {
            close(1);
            dup2(pfds[1], 1);
            close(pfds[0]);
            execlp("xclip", "xclip", "-o", NULL);
        }
        else if (ret > 0)
        {
            close(0);
            dup2(pfds[0], 0);
            close(pfds[1]);
            read(pfds[0], buf, BUFFER);

        }
        else
        {
            exit(-1);
        }
    }
    else
    {
        strcpy(buf, argv[argc-1]);
    }

    translate_engine(buf);
    code_convert("gb2312","utf-8",result,&insize,result_converted,&outsize);

    char *homedir = getenv("HOME");
    char *wordlist = NULL;
    wordlist = (char*)malloc(sizeof(char)*(strlen(homedir)+strlen("/.wordlist.txt")));
    memset(wordlist, 0, sizeof(char)*(strlen(homedir)+strlen("/.wordlist.txt")));
    strcpy(wordlist, homedir);
    strcat(wordlist, "/.wordlist.txt");

    FILE *fw = NULL;
    fw = fopen(wordlist, "a+");

    char *writ = NULL;
    writ = (char*)malloc(strlen(buf) + strlen(result_converted) + 10);
    memset(writ, 0, strlen(buf) + strlen(result_converted) + 4);
    sprintf(writ, "%s   %s\n", buf, result_converted);

    fwrite(writ, strlen(writ), 1, fw);

	char cmd[100] = {0};
	sprintf(cmd, "echo %s | xclip -selection clipboard", result_converted);
	system(cmd);

    if (argc >= 2)
    {
        printf("%s",writ);
    }




    return 0;
}
