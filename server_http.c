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
#include "headers_map.h"
#include "string_util.h"
#include "attrs_map.h"

typedef enum { false, true } bool;

multipart_parser_settings parser_body_settings;

const int LENGTH = 10;
int last_pos = 0;
int size_body = 0;
char temp[800];
char temp_body[800];
multipart_parser *parser_body;
char *boundary;
headers_map *my_header_map;
attrs_map *my_attrs_map;

enum header_element {FIELD, VALUE};
enum body_element {NAME_BODY, VALUE_BODY};

enum header_element last_header_element = FIELD;
enum body_element last_body_element = NAME_BODY;
char last_field[80];
char last_value[80];
bool is_multipart = false;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int on_header_field(http_parser *p, const char *at, size_t length)
{
    //printf("field\n");
    printf("----------H:%d\n", parser_body_settings.on_header_field);

    if (last_header_element != FIELD)
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
    //printf("F: %s\n", temp);
    last_header_element = FIELD;
    return 0;
}

int on_header_value(http_parser *p, const char *at, size_t length)
{
    //printf("value\n");
    printf("----------V:%d\n", parser_body_settings.on_header_field);
    if (last_header_element != VALUE)
    {
        strcpy(last_field, temp);
        //bzero(temp, 80 * 1024);

        // if (strcmp(last_field, "Content-Type") == 0)
        // {
        //     memcpy(last_value, at, length);
        //     last_pos = length;
        //     last_value[last_pos] = 0;
        // }
        // else
        // {
        //     memcpy(temp, at, length);
        //     last_pos = length;
        //     temp[last_pos] = 0;
        // }

        memcpy(temp, at, length);
        last_pos = length;
        temp[last_pos] = 0;
        headers_map_add(my_header_map, last_field, temp);
        //printf("field: %s\n", last_field);
        //printf("value:%s\n", temp);

    }
    else
    {
        // if (strcmp(last_field, "Content-Type") == 0)
        // {
        //     memcpy(last_value + last_pos, at, length);
        //     last_pos += length;
        //     last_value[last_pos] = 0;
        //     //printf("%d\n", last_pos);
        //     //printf("%s\n", last_value);
        // }
        // else
        // {
        //     memcpy(temp + last_pos, at, length);
        //     last_pos += length;
        //     temp[last_pos] = 0;
        // }

        memcpy(temp + last_pos, at, length);
        last_pos += length;
        temp[last_pos] = 0;
        headers_map_add(my_header_map, last_field, temp);
        //printf("value:%s\n", temp);
    }

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

    //printf("%s\n", boundary);
    if (is_multipart)
    {
        //printf("B:%s\n", at);
        int nparsed = multipart_parser_execute(parser_body, at, length);

        if (nparsed != length)
        {
            error("parser body error");
        }
        //printf("-----------Multipart------\n");
    }

    //printf("Body: %s\n", at);
    return 0;
}

// int on_headers_complete(http_parser* p)
// {
//   printf("on_headers_complete\n");
//   return 0;
// }


int on_message_begin(http_parser *p)
{
    printf("on_message_begin\n");
    return 0;
}

int on_message_complete(http_parser *p)
{
    printf("on_message_complete\n");
    return 0;
}

int on_headers_complete(http_parser *p)
{
    // if (strstr(last_value, "multipart/form-data") != NULL)
    // {
    //     is_multipart = true;
    //     //get boundary
    //     char *pos = strstr(last_value, "boundary");
    //     if (pos != NULL)
    //     {
    //         strcpy(boundary, "--");
    //         strcat(boundary, pos + 9);
    //         //printf("%s\n", boundary);
    //     }

    //     printf("boundary: %s\n", boundary);
    //     parser_body = multipart_parser_init(boundary, &parser_body_settings);
    // }

    char *value_content_type = headers_map_get(my_header_map, "Content-Type");
    printf("content type:%s\n", value_content_type);
    if (str_starts_with(value_content_type, "multipart/form-data;"))
    {
        attrs_map_parse(my_attrs_map, value_content_type + strlen("multipart/form-data;"));
        boundary = str_concat("--", attrs_map_get(my_attrs_map, "boundary"));
        //printf("Boundary: %s\n", boundary);

        parser_body = multipart_parser_init(boundary, &parser_body_settings);
        printf("----------2:%d\n", &parser_body_settings);
        printf("----------1:%d\n", parser_body_settings.on_header_field);
        is_multipart = true;
    }
    return 0;
}


int read_header_name(multipart_parser *p, const char *at, size_t length)
{
    printf("%s\n", at);
    if (last_body_element != NAME_BODY)
    {
        //bzero(temp, 80 * 1024);
        memcpy(temp_body, at, length);
        last_pos = length;
    }
    else
    {
        memcpy(temp_body + last_pos, at, length);
        last_pos += length;
    }
    //printf("%d\n", length);
    temp_body[last_pos] = 0;
    printf("Name: %s\n", temp_body);
    last_body_element = NAME_BODY;
    return 0;
}

int read_header_value(multipart_parser *p, const char *at, size_t length)
{
    printf("%s\n", at);
    if (last_body_element != VALUE_BODY)
    {
        //bzero(temp, 80 * 1024);
        memcpy(temp_body, at, length);
        last_pos = length;
    }
    else
    {
        memcpy(temp_body + last_pos, at, length);
        last_pos += length;
    }
    //printf("%d\n", length);
    temp_body[last_pos] = 0;
    printf("Value: %s\n", temp_body);
    last_body_element = VALUE_BODY;
    return 0;
}

int on_part_data(multipart_parser *p, const char *at, size_t length)
{
    printf("Data: %s\n", at);
    return 0;
}

int on_part_data_end(multipart_parser *p)
{
    printf("-------end data\n");
    return 0;
}

int on_body_end(multipart_parser *p)
{
    printf("-------end body\n");
    return 0;
}


int on_part_data_begin(multipart_parser *p)
{
    printf("-------begin data\n");

    return 0;
}

// int on_headers_complete(multipart_parser *p)
// {
//     printf("-------header complete\n");
//     return 0;
// }

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    my_header_map = headers_map_init();
    my_attrs_map = attrs_map_init();

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
    settings.on_headers_complete = on_headers_complete;
    settings.on_body = on_body;
    settings.on_message_complete = on_message_complete;
    http_parser parser;
    http_parser_init(&parser, HTTP_REQUEST);
    //parser.data = &newsockfd;

    memset(&parser_body_settings, 0, sizeof(multipart_parser_settings));

    parser_body_settings.on_header_field = read_header_name;

    parser_body_settings.on_header_value = NULL;
    parser_body_settings.on_part_data = on_part_data;
    parser_body_settings.on_part_data_end = on_part_data_end;
    parser_body_settings.on_part_data_begin = on_part_data_begin;
    parser_body_settings.on_body_end = on_body_end;
    // parser_body_settings.on_headers_complete = on_headers_complete;
    printf("----------1:%d\n", &parser_body_settings);
    printf("----------1:%d\n", parser_body_settings.on_header_field);
    while (n = read(newsockfd, buffer, LENGTH))
    {

        if (n < 0) error("ERROR reading from socket");

        nparsed = http_parser_execute(&parser, &settings, buffer, n);
        if (nparsed != n)
        {
            error("http parser error");
        }
        //multipart_parser_execute(parser_body, buffer, n);


    }
    multipart_parser_free(parser_body);

    close(newsockfd);
    close(sockfd);
    return 0; /* we never get here */
}

