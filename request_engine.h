#ifndef _request_engine_h
#define _request_engine_h

struct Header {
	char *systemKey;
	char *systemSecret;
	char *userToken;
	char *url;
	char *serviceName;
	char *body;
};

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s);

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);

char *executePOST(struct Header *header);

#endif