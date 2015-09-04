#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "request_engine.h"
#include "util.h"
#include "concat_strings.h"

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

char *executePOST(struct Header *header) {
	char *systemKeyHeader = getConcatString("ClearBlade-SystemKey: ", header->systemKey);
	char *systemSecretHeader = NULL;
	char *userTokenHeader = NULL;
	char *serviceNameHeader = NULL;

	if (header->systemSecret != NULL) {
		systemSecretHeader = getConcatString("ClearBlade-SystemSecret: ", header->systemSecret);
	}

	if (header->userToken != NULL) {
		userTokenHeader = getConcatString("ClearBlade-UserToken: ", header->userToken);
	}

	if (header->serviceName != NULL) {
		serviceNameHeader = getConcatString("serviceName: ", header->serviceName);
	}

	struct string s;
    init_string(&s);

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept: application/json, text/plain, */*");
		headers = curl_slist_append(headers, "Expect:");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, systemKeyHeader);
		if (systemSecretHeader != NULL) {
			headers = curl_slist_append(headers, systemSecretHeader);
		} 
		if (userTokenHeader != NULL) {
			headers = curl_slist_append(headers, userTokenHeader);
		}
		if (serviceNameHeader != NULL) {
			headers = curl_slist_append(headers, serviceNameHeader);
		}
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, header->url);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		if (header->body != NULL) {
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, header->body);
		} else {
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
		}
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		
		res = curl_easy_perform(curl);
	
		if(res != CURLE_OK)
      		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    	free(systemKeyHeader);
    	if (systemSecretHeader != NULL) {
    		free(systemSecretHeader);
    	}
    	if (userTokenHeader != NULL) {
    		free(userTokenHeader);
    	}
    	if (serviceNameHeader != NULL) {
    		free(serviceNameHeader);
    	}
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
	}
	curl_global_cleanup();
	return s.ptr;
}
