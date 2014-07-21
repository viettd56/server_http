#include <iostream>
#include <unistd.h>
#include "http_request.hpp"
#include "util.hpp"
#include "request_parser.hpp"

const int BUFFER_SIZE = 80;
char buffer[BUFFER_SIZE];

Http_request::Http_request(int sk): sock(sk)
{

}

Http_request::~Http_request()
{

}

void Http_request::http_request_excute()
{
    http_parser parser;
    http_parser_init(&parser, HTTP_REQUEST);

    request_parser_init();

    int n, nparsed;

    while (n = read(sock, buffer, BUFFER_SIZE))
    {

        if (n < 0) error("ERROR reading from socket");

        if (request_parser_excute(&parser, buffer, n) == 0)
        {
            headers_map = get_headers_map();
            params_map = get_params_map();
            break;
        }
    }

    // std::cout << "complete" << "\n";
    // for (std::map<string,string>::iterator it=headers_map.begin(); it!=headers_map.end(); ++it)
    // std::cout << it->first << " => " << it->second << '\n';
}
