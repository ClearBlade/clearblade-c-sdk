#ifndef _query_h
#define _query_h

/**
  * The Query struct which essentially is a query object that holds the collection ID and pointers to query manupulation functions
  * This object is used by the user to call the query functions
*/
struct Query {
	char *collectionID;
  char *collectionName;
	void (*addFilters)(char *filters);
	void (*fetch)(void (*queryResponse)(bool error, char *result));
	void (*fetchAll)(void (*queryResponse)(bool error, char *result));
	void (*createItem)(char *jsonBody, void (*queryResponse)(bool error, char *result));
};


// Sets the page number of the page which is to be fetched/queried from the collection
void setPageNumber(int pagenumber);


// Sets the page size of the page which is to be fetched/queried from the collection
void setPageSize(int pagesize);

void filters(char *filter);


/**
  * This function must be called first before any query operations. This sets the collection ID and sets all the query functions
  * Returns the query object which is then used to call all the other functions 
  * Without this, the user will not be able to call any of the query functions
*/
struct Query initializeQueryObject(char *collectionID);

struct Query initializeQueryObjectWithCollectionName(char *collectionName);

#endif