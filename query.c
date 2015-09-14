#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "request_engine.h"
#include "json_parser.h"
#include "concat_strings.h"
#include "util.h"
#include "query.h"

struct Query queryObj;
char *filter = NULL;
//char *EQ = "{\"EQ\":[{\"";
int PAGENUM = 0;
int PAGESIZE = 0;

void setPageNumber(int pagenumber) {
	PAGENUM = pagenumber;
}

void setPageSize(int pagesize) {
	PAGESIZE = pagesize;
}

char *checkForPageSize() {
	char *pagesize = NULL;
	if (PAGESIZE != 0) {
		char buff[sizeof(int)];
		sprintf(buff, "%d", PAGESIZE);
		pagesize = getConcatString("\"PAGESIZE\":", buff);
		pagesize = getConcatString(pagesize, ", ");

		return pagesize;
	} else {
		return NULL;
	}

}

char *checkForPageNum() {
	char *pagenum = NULL;
	if (PAGENUM != 0) {
		char buff[sizeof(int)];
		sprintf(buff, "%d", PAGENUM);
		pagenum = getConcatString("\"PAGENUM\":", buff);
		pagenum = getConcatString(pagenum, "}");
	} else {
		pagenum = "\"PAGENUM\":0}";
	}

	return pagenum;
}

char *checkForFilters() {
	if (filter == NULL)
		return NULL;
	else {
		return filter;
	}
}

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

void filters(char *filters) {
	filter = filters;
	/*EQ = getConcatString(EQ, field);
	EQ = getConcatString(EQ, "\": \"");
	EQ = getConcatString(EQ, value);
	EQ = getConcatString(EQ, "\"}]}");*/
}

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

		char *result = getPropertyValueFromJson(response, "DATA");
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

void fetchAll(void (*queryResponse)(bool error, char *result)) {
	fetch(queryResponse);
}

struct Query initializeQueryObject(char *collectionID) {
	struct Query query;
	query.collectionID = collectionID;
	query.fetch = fetch;
	query.fetchAll = fetchAll;

	queryObj = query;

	return query;
}
