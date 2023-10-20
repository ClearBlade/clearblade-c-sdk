#ifndef _request_engine_h
#define _request_engine_h

/**
  * Struct to store all the custom headers
*/
struct Header {
	char *systemKey;
	char *systemSecret;
	char *userToken;
	char *deviceToken;
	char *url;
	char *serviceName;
	char *collectionID;
	char *body;
	char *requestType;
};


/**
  * Struct to store the response string after the HTTP request
*/
struct string {
  char *ptr;
  size_t len;
};


/** 
  * This executes the HTTP request and returns the response.
  * It checks for the custom headers and sets them accordingly. 
  * It uses the libCURL library to make the HTTP requests.
*/
char *executeRequest(struct Header *header);

/** 
  * This executes the HTTP request and returns the response.
  * It checks for the custom headers and sets them accordingly. 
  * It requires the path of a x509 certificate and the path to the private key file
  * It uses the libCURL library to make the HTTP requests using mTLS.
*/
char *executex509MtlsRequest(struct Header *header, char *certFile, char *keyFile);

char *makeRequest(struct Header *header, char *certFile, char *keyFile);

#endif
