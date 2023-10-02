#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "request_engine.h"
#include "json_parser.h"
#include "concat_strings.h"
#include "util.h"
#include "query.h"

//TODO - Add delete and update methods


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
static char *checkForPageSize() {
	char *pagesize = NULL;
	if (PAGESIZE != 0) {
		char buff[sizeof(int)];
		sprintf(buff, "%d", PAGESIZE);
		char *pagesizeTmp = getConcatString("\"PAGESIZE\":", buff); // Page size key for the query body
		pagesize = getConcatString(pagesizeTmp, ", ");
		free(pagesizeTmp);

		return pagesize;
	} else {
		return NULL;
	}
}

/**
  * Checks whether the page number is explicitly set by the user. If not then it defaults to 0
*/
static char *checkForPageNum() {
	char *pagenum = NULL;
	if (PAGENUM != 0) {
		char buff[sizeof(int)];
		sprintf(buff, "%d", PAGENUM);
		char *pagenumTmp = getConcatString("\"PAGENUM\":", buff); // Page number key for the query body
		pagenum = getConcatString(pagenumTmp, "}");
		free(pagenumTmp);
	} else {
		pagenum = "\"PAGENUM\":0}";
	}

	return pagenum;
}

/**
  * This checks whether the user has set any filters
*/
static char *checkForFilters() {
	if (filter == NULL)
		return NULL;
	else {
		return filter;
	}
}

/**
  * Private function to build the rest endpoint based on whether
	* collection ID or collection name is being used
*/
static char *getRestEndpoint() {
	char *restEndpoint = getPlatformURL();

	if (queryObj.collectionID != NULL) {
		restEndpoint = getConcatString(restEndpoint, "/api/v/1/data/");
		restEndpoint = getConcatString(restEndpoint, queryObj.collectionID);;
	} else if (queryObj.collectionName != NULL) {
		restEndpoint = getConcatString(restEndpoint, "/api/v/1/collection/");
		restEndpoint = getConcatString(restEndpoint, getSystemKey());
		restEndpoint = getConcatString(restEndpoint, "/");
		restEndpoint = getConcatString(restEndpoint, queryObj.collectionName);
	}

	return restEndpoint;
}

/**
  * Private function to initialize the header struct
*/
static struct Header createHeaders() {
	struct Header headers;
	memset(&headers, 0, sizeof(headers)); // Make all elements of the Header struct to NULL

	headers.systemKey = getSystemKey();

	if (getUserToken() != NULL) {
		headers.userToken = getUserToken();
	} else {
		headers.deviceToken = getDeviceToken();
	}

	//TODO - I don't think this needs to be done. Need to verify.
	if (queryObj.collectionID != NULL) {
		headers.collectionID = queryObj.collectionID;
	}

	return headers;
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
		char *paramTmp = getConcatString(query, pagesize);
		param = getConcatString(paramTmp, pagenum);
		free(paramTmp);
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
	if (getUserToken() == NULL && getDeviceToken() == NULL) {
		queryResponse(true, "Cannot execute query. Auth token is NULL. Please initialize with the ClearBlade Platform first\n");
	} else if (queryObj.collectionID == NULL) {
		queryResponse(true, "Cannot execute query. Collection ID is NULL. Please initialize the query object first\n");
	} else {
		char *param = getFetchURLParameter();
		char *restEndpointMiddle = "/api/v/1/data/";
		char *restEndpointTmp1 = getConcatString(getPlatformURL(), restEndpointMiddle);
		char *restEndpointTmp2 = getConcatString(restEndpointTmp1, queryObj.collectionID);
		free(restEndpointTmp1);
		char *restEndpoint = getConcatString(restEndpointTmp2, param);
		free(restEndpointTmp2);

		struct Header headers = createHeaders();
		headers.url = restEndpoint;
		headers.requestType = "GET";

		char *response = executeRequest(&headers);

		char *result = (char *) getPropertyValueFromJson(response, "DATA");
    	if (result == NULL)
    		queryResponse(true, response); // Query execution unsuccessful
    	else {
    		queryResponse(false, result); // Query execution successful
    	}

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
	} else if (queryObj.collectionID == NULL && queryObj.collectionName == NULL) {
		queryResponse(true, "Cannot execute query. Collection ID and Collection Name are NULL. Please initialize the query object first\n");
	} else {
		char *restEndpointMiddle = "/api/v/1/data/";
		char *restEndpointTmp = getConcatString(getPlatformURL(), restEndpointMiddle);
		char *restEndpoint = getConcatString(restEndpointTmp, queryObj.collectionID);
		free(restEndpointTmp);

		struct Header headers = createHeaders();
		headers.url = restEndpoint;
		headers.body = jsonBody;
		headers.requestType = "POST";

		char *response = executeRequest(&headers);

		bool isJsonArray = checkIfJsonArray(response);

		if (isJsonArray)
			queryResponse(false, response);
		else
			queryResponse(true, response);

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

struct Query initializeQueryObjectWithCollectionName(char *collectionName) {
	struct Query query;
	query.collectionName = collectionName;
	query.addFilters = addFilters;
	query.fetch = fetch;
	query.fetchAll = fetchAll;
	query.createItem = createItem;

	queryObj = query;

	return query;
}
