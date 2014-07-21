#ifndef _body_multipart_parser_hpp_
#define _body_multipart_parser_hpp_

#include <map>
#include <string>
#include "multipart_parser.h"

using std::string;
using std::map;

namespace body_multipart_parser
{

multipart_parser* body_multipart_parser_init(const char *boundary);
void body_multipart_parser_excute(multipart_parser *parser, const char *buf, char n); // return 0 if complete
//map<string, string> get_body_params_map();

}



#endif