#ifndef _util_hpp_
#define _util_hpp_
#include <string>
#include <map>

using std::string;

void error(const char* msg);
string str_nconcat_char(const string &des, const char *src, int n);
void attrs_map_str_parse(std::map<string, string> &map,const char *str);
char *str_trim(char *str);
int str_starts_with(const char *str, const char *substr);
char *str_strip_quotes(char *str);

#endif