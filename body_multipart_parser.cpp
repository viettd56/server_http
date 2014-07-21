#include "body_multipart_parser.hpp"
#include "util.hpp"
#include <cstring>
#include <iostream>
#include <cstdio>

namespace body_multipart_parser
{
multipart_parser_settings parser_body_settings;

string last_name = "";
string last_value = "";
enum state {NAME, VALUE};
state last_state;
bool is_complete = false;
string name_file = "";
FILE *fr = NULL;

map<string, string> headers_map_parsed;
map<string, string> params_map_parsed;

void write_data(FILE *fr,const char *buffer, const int n)
{
    fflush(fr);
    fwrite(buffer, sizeof(char), n, fr); /* write data to file */
}


int read_header_name(multipart_parser *p, const char *buf, size_t len)
{

    if (last_state == NAME)
    {
        last_name = str_nconcat_char(last_name, buf, len);
    }
    else
    {
        last_name.clear();
        last_value.clear();
        last_name = str_nconcat_char(last_name, buf, len);
    }
    last_state = NAME;
    return 0;
}

int read_header_value(multipart_parser *p, const char *buf, size_t len)
{
    last_value = str_nconcat_char(last_value, buf, len);
    if (headers_map_parsed.count(last_name))
    {
        headers_map_parsed.erase(last_name);
    }
    headers_map_parsed.insert (std::pair<string, string>(last_name, last_value));
    last_state = VALUE;
    std::cout << last_name << ":" << last_value << "\n";
    return 0;
}

int on_part_data(multipart_parser *p, const char *at, size_t length)
{
    //std::cout << "Data: " <<  at;
    //std::cout << "Data" << "\n";
    write_data(fr, at, length);
    return 0;
}

int on_part_data_end(multipart_parser *p)
{
    std::cout << "-------end data\n";
    fclose(fr);
    return 0;
}

int on_body_end(multipart_parser *p)
{

    // for (std::map<string, string>::iterator it = headers_map_parsed.begin(); it != headers_map_parsed.end(); ++it)
    //     std::cout << it->first << " => " << it->second << '\n';

    is_complete = true;
    //printf("-------end body\n");
    return 0;
}


int on_part_data_begin(multipart_parser *p)
{
    std::cout << "-------begin data\n";

    return 0;
}

int on_headers_complete(multipart_parser *p)
{
    std::cout << "-------header complete\n";
    params_map_parsed.clear();
    attrs_map_str_parse(params_map_parsed,
                        headers_map_parsed.find("Content-Disposition")->second.c_str() + strlen("form-data;"));
    name_file = params_map_parsed.find("filename")->second;
    std::cout << name_file;
    fr = fopen(name_file.c_str(), "wb");
    
    //parser_body = body_multipart_parser::body_multipart_parser_init(boundary.c_str());
    //std::cout << parser_body;
    return 0;
}

multipart_parser *body_multipart_parser_init(const char *boundary)
{
    memset(&parser_body_settings, 0, sizeof(multipart_parser_settings));

    parser_body_settings.on_header_field = read_header_name;
    parser_body_settings.on_header_value = read_header_value;
    parser_body_settings.on_part_data = on_part_data;
    parser_body_settings.on_part_data_end = on_part_data_end;
    parser_body_settings.on_part_data_begin = on_part_data_begin;
    parser_body_settings.on_body_end = on_body_end;
    parser_body_settings.on_headers_complete = on_headers_complete;

    last_state = NAME;
    is_complete = false;

    headers_map_parsed.clear();
    params_map_parsed.clear();

    return multipart_parser_init(boundary, &parser_body_settings);
}
void body_multipart_parser_excute(multipart_parser *parser, const char *buf, char n)
{
    int nparsed = multipart_parser_execute(parser, buf, n);
    if (nparsed != n)
    {
        error("parser body error");
    }
}
}