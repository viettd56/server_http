#include "request_parser.hpp"
#include "http_parser.h"
#include "multipart_parser.h"
#include "body_multipart_parser.hpp"
#include "util.hpp"
#include <iostream>
#include <cstring>

using std::cout;

string last_header_field = "";
string last_header_value = "";
enum state {FIELD, VALUE};
state last_state;
http_parser_settings settings;
string boundary;
bool is_complete;
multipart_parser *parser_body = NULL;


map<string, string> headers_map_parsed;
map<string, string> params_map_parsed;

// http parser callbacks
int request_url_cb(http_parser *p, const char *buf, size_t len);
int header_field_cb(http_parser *p, const char *buf, size_t len);
int header_value_cb(http_parser *p, const char *buf, size_t len);
int body_cb(http_parser *p, const char *buf, size_t len);
int body_multipart_cb(http_parser *p, const char *buf, size_t len);
int headers_complete_cb(http_parser *p);
int message_complete_cb(http_parser *p);

int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    return 0;
}


int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    
    if (last_state == FIELD)
    {
        last_header_field = str_nconcat_char(last_header_field, buf, len);
    }
    else
    {
        last_header_field.clear();
        last_header_value.clear();
        last_header_field = str_nconcat_char(last_header_field, buf, len);
    }
    last_state = FIELD;
    return 0;
}

int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    last_header_value = str_nconcat_char(last_header_value, buf, len);
    if (headers_map_parsed.count(last_header_field))
    {
        headers_map_parsed.erase(last_header_field);
    }
    headers_map_parsed.insert (std::pair<string, string>(last_header_field, last_header_value));
    //cout << last_header_field << ":" << last_header_value << std::endl;
    last_state = VALUE;
    return 0;
}

int body_cb(http_parser *p, const char *buf, size_t len)
{
    return 0;
}

int body_multipart_cb(http_parser *p, const char *buf, size_t len)
{
    body_multipart_parser::body_multipart_parser_excute(parser_body, buf, len);
    return 0;
}

int headers_complete_cb(http_parser *p)
{
    if (is_multipart())
    {
        attrs_map_str_parse(params_map_parsed,
                            headers_map_parsed.find("Content-Type")->second.c_str() + strlen("multipart/form-data;"));
        boundary = "--" + params_map_parsed.find("boundary")->second;
        //std::cout << boundary;
        parser_body = body_multipart_parser::body_multipart_parser_init(boundary.c_str());
        //std::cout << parser_body;
        settings.on_body = body_multipart_cb;
    }
    return 0;
}


int message_complete_cb(http_parser *p)
{
    // for (std::map<string,string>::iterator it=headers_map_parsed.begin(); it!=headers_map_parsed.end(); ++it)
    // std::cout << it->first << " => " << it->second << '\n';
    multipart_parser_free(parser_body);
    is_complete = true;
    return 0;
}

int is_multipart()
{
    if (headers_map_parsed.count("Content-Type"))
    {
        if (str_starts_with(headers_map_parsed.find("Content-Type")->second.c_str(), "multipart/form-data"))
        {
            //std::cout << "true" << "\n";
            return 1;
        }
    }
    return 0;
}

void request_parser_init()
{

    settings.on_header_field = header_field_cb;
    settings.on_header_value = header_value_cb;
    settings.on_url = request_url_cb;
    settings.on_body = body_cb;
    settings.on_headers_complete = headers_complete_cb;
    settings.on_message_complete = message_complete_cb;

    last_state = FIELD;
    boundary = "";
    is_complete = false;

    headers_map_parsed.clear();
    params_map_parsed.clear();
}
int request_parser_excute(http_parser *parser, char *buf, char n)
{
    int nparsed = http_parser_execute(parser, &settings, buf, n);
    if (nparsed != n)
    {
        error("http parser error");
    }

    if (is_complete)
    {
        return 0;
    }

    return 1;
}
map<string, string> get_headers_map()
{
    return headers_map_parsed;
}
map<string, string> get_params_map()
{
    return params_map_parsed;
}