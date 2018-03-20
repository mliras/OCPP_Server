/*
 * authorizationLists.h
 *
 *  Created on: Nov 27, 2017
 *      Author: root
 */

#ifndef AUTHORIZATIONLISTS_H_
#define AUTHORIZATIONLISTS_H_

#include "aux.h"
#include <string.h>

//Hasta 64 listas con sus nombres, diferentes
#define MAX_NUM_AUTH_LISTS 64

//Ver Pag 15
enum localAuthorizationType{
	NOT_SUPPORTED,
	AUTHORIZATION_CACHE,
	LOCAL_AUTHORIZATION_LIST,
	BOTH,
};

struct authorization_record{
	int status;
	struct tm *expiryDate;
	char parentIdTag[20];
};

struct authorization_list_entry{
	struct authorization_record *entry;
	char *idTag;
	struct authorization_list_entry *next;
};

//typedef struct authorization_list_entry list_entry;

struct authorization_list{
	char *name;
	int listVersion;
	struct authorization_list_entry *list;
};


//Se debe asociar de alguna forma que authorization list esta usando cada chagepoint.
//Se crea esta struct y este array para ello.
struct authlistchargepoint{
	struct authorization_list* authlist;
	struct chargePoint* cp;
};
struct authlistchargepoint* authorizationListsAssignedToChargePoints[MAX_NUM_CHARGEPOINTS];
///

static int listVersion=1;

//Este array mantiene un listado de authorization lists con idTags que pueden ser mandados a los diferentes chargepoints.
struct authorization_list * authorization_lists[MAX_NUM_AUTH_LISTS];

int createAuthorizationList(const char *name);
void addAuthorizationToList(struct authorization_list *list, struct authorization_list_entry *entry);
int addAuthorizationDataToList(struct authorization_list *list, const char *idTag, const char *parentIdTag, const char *expiryDate, int status);
void showAuthorizationLists();
struct authorization_list *getAuthListOfChargePoint(struct chargePoint *cp);
struct authorization_list *getAuthListFromName(char *name);
void setAuthListToChargePoint(struct chargePoint *cp, struct authorization_list *list);
int isInAuthorizationList(struct authorization_list *list, char *idTag);
int getIdTagStatus(struct authorization_list *list, char *idTag);
/*
int removeAuthorizationFromList(struct authorization_list *list, struct authorization_list_entry *entry);
int remoevAuthorizationList(char *name);
*/



#endif /* AUTHORIZATIONLISTS_H_ */
