#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "net.hpp"
#include "util.hpp"
#include "http_request.hpp"

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr;

    if (argc < 2)
    {
        error("ERROR, no port provided\n");
    }

    clilen = sizeof(cli_addr);

    sockfd = net_bind_socket(atoi(argv[1]));

    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    Http_request request(newsockfd);
    request.http_request_excute();

    // size_t n, nparsed;

    // char buffer[LENGTH + 1];
    // bzero(buffer, LENGTH);
    // while (n = read(newsockfd, buffer, LENGTH))
    // {

    //     if (n < 0) error("ERROR reading from socket");

    //     nparsed = http_parser_execute(&parser, &settings, buffer, n);
    //     if (nparsed != n)
    //     {
    //         error("http parser error");
    //     }
    //     //multipart_parser_execute(parser_body, buffer, n);


    // }
    // multipart_parser_free(parser_body);

    close(newsockfd);
    close(sockfd);
    return 0; /* we never get here */
}

