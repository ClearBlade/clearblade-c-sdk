#ifndef _query_h
#define _query_h

struct Query {
	char *collectionID;
	void (*fetch)(void (*queryResponse)(bool error, char *result));
};

void setPageNumber(int pagenumber);
void setPageSize(int pagesize);

void filters(char *filter);

struct Query initializeQueryObject(char *collectionID);

#endif