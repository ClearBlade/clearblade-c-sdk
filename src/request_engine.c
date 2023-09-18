#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "request_engine.h"
#include "util.h"
#include "concat_strings.h"
#include "die.h"


/**
  * Function to initialize the string struct. Allocates memory to char *ptr which is used to store the HTTP response afterwards
*/
void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}


/**
  * This is the HTTP response callback. This constructs the string containing the response and returns it back
*/
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


/**
  * This executes the HTTP request and returns the response.
  * It checks for the custom headers and sets them accordingly.
  * It uses the libCURL library to make the HTTP requests.
*/
char *executeRequest(struct Header *header) {
	return makeRequest(header, "", "");
}

char *executex509MtlsRequest(struct Header *header, char *certFile, char *keyFile) {
	return makeRequest(header, certFile, keyFile);
}

char*makeRequest(struct Header *header, char *certFile, char *keyFile) {
	char *systemKeyHeader = getConcatString("ClearBlade-SystemKey: ", header->systemKey); // This is a required header for all calls
	char *systemSecretHeader = NULL;
	char *userTokenHeader = NULL;
	char *serviceNameHeader = NULL;
	char *collectionIDHeader = NULL;
	char *deviceTokenHeader = NULL;

	if (header->systemSecret != NULL || header->systemSecret != 0) {
		systemSecretHeader = getConcatString("ClearBlade-SystemSecret: ", header->systemSecret);
	}

	if (header->userToken != NULL || header->userToken != 0) {
		userTokenHeader = getConcatString("ClearBlade-UserToken: ", header->userToken);
	} else if (header->deviceToken != NULL || header->deviceToken != 0) {
		deviceTokenHeader = getConcatString("ClearBlade-DeviceToken: ", header->deviceToken);
	}

	if (header->serviceName != NULL || header->serviceName != 0) {
		serviceNameHeader = getConcatString("serviceName: ", header->serviceName);
	}

	if (header->collectionID != NULL || header->collectionID != 0) {
		collectionIDHeader = getConcatString("collectionID: ", header->collectionID);
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
		headers = curl_slist_append(headers, "Expect:"); // We dont need the Expect header
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, systemKeyHeader);
		if (systemSecretHeader != NULL) {
			headers = curl_slist_append(headers, systemSecretHeader);
		}
		if (userTokenHeader != NULL) {
			headers = curl_slist_append(headers, userTokenHeader);
		} else if (deviceTokenHeader != NULL) {
			headers = curl_slist_append(headers, deviceTokenHeader);
		}
		if (serviceNameHeader != NULL) {
			headers = curl_slist_append(headers, serviceNameHeader);
		}
		if (collectionIDHeader != NULL) {
			headers = curl_slist_append(headers, collectionIDHeader);
		}
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, header->url);
		if (strcmp("POST", header->requestType) == 0) {
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			if (header->body != NULL) {
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, header->body); // This is the POST body
			} else {
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L); // Send POST without body
			}
		}

		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
   	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
   	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
   	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		if (certFile && certFile[0] != '\0') {
    	char *substrPtr = strstr(certFile, "BEGIN CERTIFICATE");
    	if (substrPtr) {
				struct curl_blob blob;

				blob.data = certFile;
  			blob.len = strlen(certFile);
  			blob.flags = CURL_BLOB_COPY;

				curl_easy_setopt(curl, CURLOPT_SSLCERT_BLOB, &blob);
			} else {
				//See if we have a file path
				FILE* filePtr = fopen(certFile, "r");
    		if (filePtr) {
					curl_easy_setopt(curl, CURLOPT_SSLCERT, certFile);
    			fclose(filePtr);
    		} else {
    			die("certFile parameter must contain either the contents of an SSL certificate file or the path to the certificate file");
    		}
			}
			curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		}

		if (keyFile && keyFile[0] != '\0') {
			//See if keyFile contains the private key or the path to the private key
    	char *substrPtr = strstr(keyFile, "PRIVATE KEY");
    	if (substrPtr) {
				struct curl_blob blob;

				blob.data = keyFile;
  			blob.len = strlen(keyFile);
  			blob.flags = CURL_BLOB_COPY;

				curl_easy_setopt(curl, CURLOPT_SSLKEY_BLOB, &blob);
			} else {
				//See if we have a file path
				FILE* filePtr = fopen(keyFile, "r");
    		if (filePtr) {
					curl_easy_setopt(curl, CURLOPT_SSLKEY, keyFile);
    			fclose(filePtr);
    		} else {
    			die("keyFile parameter must contain either the contents of a private key file or the path to the private key file");
    		}
			}
			curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
		}

		res = curl_easy_perform(curl); // Execute the request

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
  	if (collectionIDHeader != NULL) {
  		free(collectionIDHeader);
  	}
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
	}
	curl_global_cleanup();

	return s.ptr;
}
