#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "request_engine.h"
#include "concat_strings.h"
#include "util.h"
#include "query.h"

struct Query queryObj;
char *queryParam = NULL;
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
	} else {
		char *param = getFetchURLParameter();
		queryParam = param;
		char *endpointTemp = "/api/v/1/data/";
		char *endpointTemp2 = getConcatString(getPlatformURL(), endpointTemp);
		char *endpointTemp3 = getConcatString(endpointTemp2, queryObj.collectionID);
		char *restEndpoint = getConcatString(endpointTemp3, queryParam);

		struct Header headers;
		memset(&headers, 0, sizeof(headers)); // Make all elements of the Header struct to NULL

		headers.url = restEndpoint;						
		headers.systemKey = getSystemKey();			 // Set Headers	
		headers.userToken = getUserToken();
		headers.collectionID = queryObj.collectionID;
		headers.requestType = "GET"; 

		char *response = executeRequest(&headers);

		free(queryParam);
		free(endpointTemp2);
		free(endpointTemp3);
		free(restEndpoint);
		queryResponse(false, response);
	}
}

struct Query initializeQueryObject(char *collectionID) {
	struct Query query;
	query.collectionID = collectionID;
	query.fetch = fetch;

	queryObj = query;

	return query;
}
