#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "request_engine.h"
#include "json_parser.h"
#include "concat_strings.h"
#include "util.h"
#include "query.h"

struct Query queryObj; // This stores the collection ID and all sets the functions for querying
char *filter = NULL;
//char *EQ = "{\"EQ\":[{\"";
int PAGENUM = 0;
int PAGESIZE = 0;

// Sets the page number of the page which is to be fetched/queried from the collection
void setPageNumber(int pagenumber) {
	PAGENUM = pagenumber;
}

// Sets the page size of the page which is to be fetched/queried from the collection
void setPageSize(int pagesize) {
	PAGESIZE = pagesize;
}

/**
  * Checks whether the page size is explicitly set by the user. If not then it defaults to NULL
*/
char *checkForPageSize() {
	char *pagesize = NULL;
	if (PAGESIZE != 0) {
		char buff[sizeof(int)];
		sprintf(buff, "%d", PAGESIZE);
		pagesize = getConcatString("\"PAGESIZE\":", buff); // Page size key for the query body
		pagesize = getConcatString(pagesize, ", ");

		return pagesize;
	} else {
		return NULL;
	}
}

/**
  * Checks whether the page number is explicitly set by the user. If not then it defaults to 0
*/
char *checkForPageNum() {
	char *pagenum = NULL;
	if (PAGENUM != 0) {
		char buff[sizeof(int)];
		sprintf(buff, "%d", PAGENUM);
		pagenum = getConcatString("\"PAGENUM\":", buff); // Page number key for the query body
		pagenum = getConcatString(pagenum, "}");
	} else {
		pagenum = "\"PAGENUM\":0}";
	}

	return pagenum;
}

/**
  * This checks whether the user has set any filters
*/
char *checkForFilters() {
	if (filter == NULL)
		return NULL;
	else {
		return filter;
	}
}

/**
  * This constructs the query body with filters, page number and page size 
*/
char *getFetchURLParameter() {
	char *query = "?query={";
	char *filters = checkForFilters();
	char *pagesize = checkForPageSize();
	char *pagenum = checkForPageNum();
	char *param = NULL;

	if (filters != NULL)
		param = getConcatString(query, filters);

	if (pagesize == NULL) 
		param = getConcatString(query, pagenum);
	else {
		param = getConcatString(query, pagesize);
		param = getConcatString(param, pagenum);
	}

	if (PAGENUM != 0)
		free(pagenum);

	if (PAGESIZE != 0)
		free(pagesize);

	return param;
}

/**
  * This is temporary
  * Basically this tells users to construct the entire FILTERS array themselves and pass it as a string to this function
  * Then when the user does a fetch(), this filter array will be added to the query body as it is
*/
void addFilters(char *filters) {
	filter = filters;
}

/**
  * This makes a GET request to the fetch REST API and gives the response back to the user via the queryResponse callback
*/
void fetch(void (*queryResponse)(bool error, char *result)) {
	if (getUserToken() == NULL) {
		queryResponse(true, "Cannot execute query. Auth token is NULL. Please initialize with the ClearBlade Platform first\n");
	} else if (queryObj.collectionID == NULL) {
		queryResponse(true, "Cannot execute query. Collection ID is NULL. Please initialize the query object first\n");
	} else {
		char *param = getFetchURLParameter();
		char *restEndpoint = "/api/v/1/data/";
		restEndpoint = getConcatString(getPlatformURL(), restEndpoint);
		restEndpoint = getConcatString(restEndpoint, queryObj.collectionID);
		restEndpoint = getConcatString(restEndpoint, param);

		struct Header headers;
		memset(&headers, 0, sizeof(headers)); // Make all elements of the Header struct to NULL

		headers.url = restEndpoint;						
		headers.systemKey = getSystemKey();			 // Set Headers	
		headers.userToken = getUserToken();
		headers.collectionID = queryObj.collectionID;
		headers.requestType = "GET"; 

		char *response = executeRequest(&headers);

		char *result = (char *) getPropertyValueFromJson(response, "DATA");
    	if (result == NULL)
    		queryResponse(true, response); // Query execution unsuccessful
    	else {
    		queryResponse(false, result); // Query execution successful
    	}

		/* Clean shit up */
		free(param);
		free(restEndpoint);
	}
}

/* Fetch the entire collection */
void fetchAll(void (*queryResponse)(bool error, char *result)) {
	fetch(queryResponse);
}

/**
  * Creates a new item in the column specified by the user in an existing collection
  * The user needs to construct the JSON body for the HTTP request
  * JSON body format: {"nameOfColumn":"itemValue"}
*/
void createItem(char *jsonBody, void (*queryResponse)(bool error, char *result)) {
	if (getUserToken() == NULL) {
		queryResponse(true, "Cannot execute query. Auth token is NULL. Please initialize with the ClearBlade Platform first\n");
	} else if (queryObj.collectionID == NULL) {
		queryResponse(true, "Cannot execute query. Collection ID is NULL. Please initialize the query object first\n");
	} else {
		char *restEndpoint = "/api/v/1/data/";
		restEndpoint = getConcatString(getPlatformURL(), restEndpoint);
		restEndpoint = getConcatString(restEndpoint, queryObj.collectionID);

		struct Header headers;
		memset(&headers, 0, sizeof(headers)); // Make all elements of the Header struct to NULL

		headers.url = restEndpoint;						
		headers.systemKey = getSystemKey();			 // Set Headers	
		headers.userToken = getUserToken();
		headers.collectionID = queryObj.collectionID;
		headers.body = jsonBody;
		headers.requestType = "POST"; 

		char *response = executeRequest(&headers);

		bool isJsonArray = checkIfJsonArray(response);

		if (isJsonArray)
			queryResponse(false, response);
		else
			queryResponse(true, response);

		/* Clean shit up */
		free(restEndpoint);
	}
}

/**
  * This function must be called first before any query operations. This sets the collection ID and sets all the query functions
  * Returns the query object which is then used to call all the other functions 
  * Without this, the user will not be able to call any of the query functions
*/
struct Query initializeQueryObject(char *collectionID) {
	struct Query query;
	query.collectionID = collectionID;
	query.addFilters = addFilters;
	query.fetch = fetch;
	query.fetchAll = fetchAll;
	query.createItem = createItem;

	queryObj = query;

	return query;
}
