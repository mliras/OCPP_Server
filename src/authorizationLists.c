/*
 * authorizationLists.c
 *
 *  Created on: Jan 3, 2018
 *      Author: root
 */
#include "authorizationLists.h"
#include <time.h>

//Esta funcion asigna un auth list a un chargepoint. Para ello, recorre el array de asignaciones de auth lists a chargepoints y
// si encuentra el chargepoint, cambia el valor, si no, crea uno nuevo
void setAuthListToChargePoint(struct chargePoint *cp, struct authorization_list *list)
{
	for (int i=0; i<MAX_NUM_CHARGEPOINTS; i++)
	{
		if (authorizationListsAssignedToChargePoints[i])
		{
			if (authorizationListsAssignedToChargePoints[i]->cp==cp)
			{
				authorizationListsAssignedToChargePoints[i]->authlist=list;
				return;
			}
		}
	}

	for (int i=0; i<MAX_NUM_CHARGEPOINTS; i++)
	{
			if (!authorizationListsAssignedToChargePoints[i])
			{
				struct authlistchargepoint *alcp=(struct authlistchargepoint *)calloc(1, sizeof(struct authlistchargepoint ));
				alcp->authlist=list;
				alcp->cp=cp;
				authorizationListsAssignedToChargePoints[i]=alcp;
				return;
			}
	}

//Si tenemos la lista de asignaciones llena, llegaria hasta aqui.
	addLog("Lista de asignaciones llena", 0);
	return;
}

//Busca un idTag en una lista de authorizacion. Si lo encuentra devuelve 1, en caso contrario devuelve 0
int isInAuthorizationList(struct authorization_list *list, char *idTag)
{
	struct authorization_list_entry *l=list->list;

	while (l && strcmp(idTag, l->idTag)!=0)
	{
		l=l->next;
	}

	if (l) return 1;
	else return 0;
}

//Devuelve el status de un idTag en un authorizacionList. Si no lo encuentra devuelve -1
int getIdTagStatus(struct authorization_list *list, char *idTag)
{
	if (debug) printf("\nINTENTA BUSCAR el idtag %s en la authorization_list %x, %s\n", idTag, list, list->name);
	struct authorization_list_entry *l=list->list;

	while (l && strcmp(idTag, l->idTag)!=0)
	{
	//	if (debug) printf("\nSE COMPARA el idtag %s con el recibido %s\n", l->idTag, idTag);
		l=l->next;
	}

	if (l && l->entry) return l->entry->status;
	else return -1;
}

struct authorization_list *getAuthListFromName(char *name)
{
	for (int i=0; i<MAX_NUM_AUTH_LISTS; i++)
	{
		if (authorization_lists[i])
		{
			if (strcmp(authorization_lists[i]->name, name)==0)
			{
				return authorization_lists[i];
			}
		}
	}

	return NULL;

}

//Esta funcion recorre el array de asignaciones de auth lists a chargepoints y devuelve la authlist para el chargepoint dado.
//Si no lo encuentra, devuelve NULL
struct authorization_list *getAuthListOfChargePoint(struct chargePoint *cp)
{
	for (int i=0; i<MAX_NUM_CHARGEPOINTS; i++)
	{
		if (authorizationListsAssignedToChargePoints[i])
		{
			if (authorizationListsAssignedToChargePoints[i]->cp==cp)
			{
				return authorizationListsAssignedToChargePoints[i]->authlist;
			}
		}
	}

	return NULL;
}

//Esta funcion muestra por pantalla las listas de autorizacion.
//Es una mera funcion de debug
void showAuthorizationLists()
{
	printf("\n======================");
	printf("\nAUTHORIZATION LISTS");

	for (int i=0; i<MAX_NUM_AUTH_LISTS; i++)
	{
		//if (debug) printf("\nUNA VEZ DENTRO  authorization_lists de %d se almacena en %x", i, authorization_lists[i]);
		if (authorization_lists[i])
		{
			printf("\nLista: %s", authorization_lists[i]->name);
			struct authorization_list_entry *ale=authorization_lists[i]->list;
			while (ale)
			{
				printf("\n          ");
				if (ale->idTag)				printf("IdTag: %s", ale->idTag);
				if (ale->entry)
				{
					if (ale->entry->expiryDate)
					{
						char *o=(char *)calloc(1, sizeof(char)*80);
						strftime (o, 80, "%Y-%m-%dT%H:%M:%S.", ale->entry->expiryDate);
						printf(",expiryDate: %s", o);
						free(o);
					}
					if (ale->entry->parentIdTag)				printf(",parentIdTag: %s", ale->entry->parentIdTag);
					if (ale->entry->status)		printf(",status: %d", ale->entry->status);
				}

				ale=ale->next;
			}
		}
	}
}

//
//crea una nueva entrada en el array authorization_lists
//Si lo consigue crear, devuelve la posición
//En caso contrario, devuelve -1
//
int createAuthorizationList(const char *name)
{
	int first =-1;
	for (int i=0; i<MAX_NUM_AUTH_LISTS; i++)
	{
		if (authorization_lists[i])
		{
			if (strcmp(authorization_lists[i]->name,name)==0)
			{
				show_info("Ya existe otra lista con ese nombre");
				return -1;
			}
		}
		else
		{
			if (first<0) first=i;
		}
	}

	if (first>=0)
	{
		struct authorization_list *temp=(struct authorization_list *)calloc(1, sizeof(struct authorization_list));
		temp->name=strdup(name);
		temp->listVersion=listVersion++; //Si tenemos una unica lista y la version es esta, pero en base de datos,
		///lo estoy haciendo de forma que pueda haber pequeñas listas específicas para cada CP
		temp->list=NULL;

		authorization_lists[first]=temp;
		return first;
	}
}

void addAuthorizationToList(struct authorization_list *list, struct authorization_list_entry *entry)
{
	struct authorization_list_entry *temp, *aux;

	entry->next=NULL;
	aux=NULL;
	temp=list->list;

	while (temp)
	{
		aux=temp;
		temp=temp->next;
	}

	//Hemos llegado al final de la lista
	//temp=(struct authorization_list_entry *)calloc(1, sizeof(struct authorization_list_entry));
	if (aux)
	{
		aux->next=entry;
	}
	else
	{
		list->list=entry;
	}
}

//
//Esta funcion añade una entrada a un authorization list.
//Para ello crea las estructuras de datos necesarias.
int addAuthorizationDataToList(struct authorization_list *list, const char *idTag, const char *parentIdTag, const char *expiryDate, int status)
{
	struct authorization_list_entry *temp, *aux;
	struct authorization_record* entry;

	temp=list->list;
	aux=NULL;

	while (temp)
	{
		aux=temp;
		temp=temp->next;
	}

	//Hemos llegado al final de la lista
	temp=(struct authorization_list_entry *)calloc(1, sizeof(struct authorization_list_entry));

	entry=(struct authorization_record *)calloc(1, sizeof(struct authorization_record));

	strncpy(entry->parentIdTag, parentIdTag, sizeof(entry->parentIdTag));
	entry->status=status;
	entry->expiryDate=(struct tm*)calloc(1,sizeof(struct tm));
	strptime(expiryDate, "%Y-%m-%dT%H:%M:%S.", entry->expiryDate);
	temp->idTag=strdup(idTag);
	temp->entry=entry;
	if (aux)
	{
		aux->next=temp;
	}
	else
	{
		list->list=temp;
	}
	temp->next=NULL;

	return 0;
}
