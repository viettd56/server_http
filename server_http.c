/* A simple server in the internet domain using TCP
The port number is passed as an argument
This version runs forever, forking off a separate
process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "http_parser.h"
#include "multipart_parser.h"

const int LENGTH = 10;
int last_pos = 0;
int size_body = 0;
char temp[80];
multipart_parser *parser_body;
char boundary[80];

enum header_element {FIELD, VALUE};
enum body_element {NAME_BODY, VALUE_BODY};

enum header_element last_header_element = FIELD;
enum body_element last_body_element = NAME_BODY;


int on_header_field(http_parser *p, const char *at, size_t length)
{
    //printf("field\n");

    if (last_header_element != FIELD)
    {
        //get boundary
        char *p = strstr(temp, "boundary");
        if (p != NULL)
        {
            strcpy(boundary, p + 9);
            printf("%s\n", boundary);
        }

        //bzero(temp, 80 * 1024);
        memcpy(temp, at, length);
        last_pos = length;
    }
    else
    {
        memcpy(temp + last_pos, at, length);
        last_pos += length;
    }
    //printf("%d\n", length);
    temp[last_pos] = 0;
    //printf("F: %s\n", temp);
    last_header_element = FIELD;
    return 0;
}

int on_header_value(http_parser *p, const char *at, size_t length)
{
    //printf("value\n");
    if (last_header_element != VALUE)
    {
        //bzero(temp, 80 * 1024);
        memcpy(temp, at, length);
        last_pos = length;

    }
    else
    {
        memcpy(temp + last_pos, at, length);
        last_pos += length;
    }
    temp[last_pos] = 0;
    //printf("V: %s\n", temp);
    last_header_element = VALUE;
    return 0;
}

int on_url(http_parser *p, const char *at, size_t length)
{
    //printf("url: %s\n", at);
    return 0;
}


int on_body(http_parser *p, const char *at, size_t length)
{

    multipart_parser_execute(parser_body, at, length);
    return 0;
}

// int on_headers_complete(http_parser* p)
// {
//   printf("on_headers_complete\n");
//   return 0;
// }


int on_message_begin(http_parser *p)
{
    //printf("on_message_begin\n");
    return 0;
}


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int read_header_name(multipart_parser *p, const char *at, size_t length)
{
    if (last_body_element != NAME_BODY)
    {
        //bzero(temp, 80 * 1024);
        memcpy(temp, at, length);
        last_pos = length;
    }
    else
    {
        memcpy(temp + last_pos, at, length);
        last_pos += length;
    }
    //printf("%d\n", length);
    temp[last_pos] = 0;
    printf("Name: %s\n", temp);
    last_body_element = NAME_BODY;
    return 0;
}

int read_header_value(multipart_parser *p, const char *at, size_t length)
{
     if (last_body_element != VALUE_BODY)
    {
        //bzero(temp, 80 * 1024);
        memcpy(temp, at, length);
        last_pos = length;
    }
    else
    {
        memcpy(temp + last_pos, at, length);
        last_pos += length;
    }
    //printf("%d\n", length);
    temp[last_pos] = 0;
    printf("Value: %s\n", temp);
    last_body_element = VALUE_BODY;
    return 0;
}



int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    /* server */

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket server control");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    size_t n, nparsed;

    char buffer[LENGTH + 1];
    bzero(buffer, LENGTH);

    http_parser_settings settings;
    settings.on_header_field = on_header_field;
    settings.on_header_value = on_header_value;
    settings.on_message_begin = on_message_begin;
    settings.on_url = on_url;
    settings.on_status = NULL;
    settings.on_headers_complete = NULL;
    settings.on_body = on_body;
    settings.on_message_complete = NULL;
    http_parser parser;
    http_parser_init(&parser, HTTP_REQUEST);
    //parser.data = &newsockfd;

    multipart_parser_settings parser_body_settings;

    memset(&parser_body_settings, 0, sizeof(multipart_parser_settings));

    parser_body_settings.on_header_field = read_header_name;
    parser_body_settings.on_header_value = read_header_value;



    parser_body = multipart_parser_init(boundary, &parser_body_settings);



    while (n = read(newsockfd, buffer, LENGTH))
    {

        if (n < 0) error("ERROR reading from socket");

        nparsed = http_parser_execute(&parser, &settings, buffer, n);


    }
    multipart_parser_free(parser_body);

    close(newsockfd);
    close(sockfd);
    return 0; /* we never get here */
}

