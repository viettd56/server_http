all: server
server: http_parser.c server_http.c
	gcc http_parser.c server_http.c -o server