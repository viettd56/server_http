all: server
server: http_parser.c http_request.cpp net.cpp request_parser.cpp server_http.cpp util.cpp body_multipart_parser.cpp multipart_parser.c
		g++ http_parser.c http_request.cpp net.cpp request_parser.cpp server_http.cpp util.cpp body_multipart_parser.cpp multipart_parser.c -o server
