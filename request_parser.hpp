#ifndef _request_parser_hpp_
#define _request_parser_hpp_

#include <map>
#include <string>
#include "http_parser.h"

using std::string;
using std::map;

void request_parser_init();
int request_parser_excute(http_parser* parser, char* buf, char n); // return 0 if complete
map<string, string> get_headers_map();
map<string, string> get_params_map();
int is_multipart();

#endif