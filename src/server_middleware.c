/*
 * middleware.c
 *
 *  Created on: Nov 21, 2017
 *      Author: root
 */

#include "server_middleware.h"
#include "data_store.h"

////////////////////////////////////////////////
//         FUNCIONES AUXILIARES
////////////////////////////////////////////////

//This function initializes all the data structures and variables to an initial state.
//It could be necessary to store these data into a file or DB <-- NOT IMPLEMENTED
void middleware_initialize()
{
	idTagGroups=NULL;
	numCurrentlyConnected=0;

	//reservations
	numReservationId=20000;
	numReservations=0;

	//transactions
	transactionId=61000;

	currentnumvendors=0;
	for (int i=0; i<MAXVENDORS; i++)
	{
		vendorIds[i]=NULL;
	}

	for (int i=0; i<MAX_NUM_CHARGEPOINTS; i++)
	{
		currentTransactions[i]=NULL;
	}
}


//
//4.8
//
struct IdTagInfo *verifyIdentifierValidity(struct lws *wsi_in, const char* idTag)
{
		//Pag 40:The Central System MUST verify validity of the identifier in the StartTransaction.req
		// PDU, because the identifier  might  have  been  authorized  locally  by  the  Charge  Point  using  outdated  information.
		//The identifier, for instance, may have been blocked since it was added to the Charge Point’s Authorization Cache

		//It is likely that The Central System applies sanity checks to the data contained in a StartTransaction.req
		//it  received.  The  outcome  of  such  sanity  checks  SHOULD  NOT  ever  cause  the  Central  System  to  not
		//respond  with  a 	StartTransaction.conf.  Failing  to  respond  with  a	StartTransaction.conf  will  only  cause
		//the  Charge  Point  to  try  the  same  message  again  as  specified  in	Error  responses  to  transaction-related messages	.

	struct IdTagInfo *info=(struct IdTagInfo *)calloc(1,sizeof(struct IdTagInfo));

	time_t now;
	time(&now);
	struct tm* now_tm;
	now_tm = localtime(&now);

	//Da un tiempo de una hora de reserva
	now_tm->tm_hour+=1;

	char *oneHourPlus=(char *)calloc(1,sizeof(char)*80);
	strftime (oneHourPlus, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
	oneHourPlus[strlen(oneHourPlus)]='\0';

	info->expiryDate=oneHourPlus;

	strncpy(info->parentIdtag.IdToken, idTag, 20);

	struct chargePoint *cp=getChargePointFromWSI(wsi_in);
	struct authorization_list *al=getAuthListOfChargePoint(cp);

	//Si el chargepoint tiene una authorizationList asignada, se busca en ella.
	if (al)
		info->status=getIdTagStatus(al, idTag);
	else
	{
		//En caso contrario, se busca en la lista de autorizacion GENERAL
		al=getAuthListFromName("GENERAL");
		info->status=getIdTagStatus(al, idTag);
	}

	if (info->status==-1) info->status=_CP_AUTHORIZATION_INVALID;

	return info;
}


void manageDiagnosticsFile(char *fn)
{

}

////////////////////////////////////////////////////
//     CHARGING PROFILES
////////////////////////////////////////////////////

int ChargePointNeedsChargingProfile()
{
	return 1;  //<--- NOT IMPLEMENTED
}

///////////////////////////////
//  VENDORS && MODELS
///////////////////////////////

char *getVendorId(char *vendor)
{
	char *vendor_low=strlwr_ex(vendor);
	char *vendortable_low;

	for (int i=0; i<currentnumvendors; i++)
	{
		if (vendorIds[i])
		{
			vendortable_low=strlwr_ex(vendorIds[i]->vendor);
			printf("\n%s vs %s", vendortable_low, vendor_low);
			if (strcmp(vendortable_low, vendor_low)==0)
			{

				free(vendortable_low);
				free(vendor_low);
				printf("\nDevolvemos %s", vendorIds[i]->vendorId);
				return vendorIds[i]->vendorId;
			}
			free(vendortable_low);
		}
	}

	free(vendor_low);
	return NULL;
}

void setVendorId(char *vendor, char *vendorId)
{
	int found=0;
	struct vendorIdData *p;
	for (int i=0; i<currentnumvendors; i++)
	{
		if (vendorIds[i])
		{
			if (strcmp(vendorIds[i]->vendor, vendor)==0)
			{
				free(vendorIds[i]->vendorId);
				vendorIds[i]->vendorId=strdup(vendorId);
				found=1;
			}
		}
	}

//	printf("Vendor es: %s", vendor);
//	printf("VendorID es: %s", vendorId);
	if (!found)
	{
		p=calloc(1, sizeof(struct vendorIdData));
		p->vendor=strdup(vendor);
		p->vendorId=strdup(vendorId);
		vendorIds[currentnumvendors]=p;
		currentnumvendors++;
	}
}

///////////////////////////////////////////
//      CONFIGURATION KEYS
///////////////////////////////////////////

//Returns 0 for INT
//Returns 1 for BOOL
//Returns 2 for STRING
//Returns 3 for CSL
//Returns -1 for Error (not found or bad configured)
int getConfigurationKeyType(char *key)
{
	int i =0;
	while (clientConfigurationKeys[i])
	{
		if (strcmp(clientConfigurationKeys[i], key)==0)
		{
			if (configurationKeyCharacteristics[i]%100<0) return -1;
			if (configurationKeyCharacteristics[i]%100<10) return 1; //INT
			if (configurationKeyCharacteristics[i]%100<20) return 2; //STRING
			if (configurationKeyCharacteristics[i]%100<30) return 3; //CSL
			if (configurationKeyCharacteristics[i]%100>30) return 0; //BOOL
		}
		i++;
	}

	return -1;
}

char* getConfigurationKeyStringValue(char *key)
{
	int i=0;
	while (configurationKeys[i])
	{
		if (strcmp(configurationKeys[i], key)==0)
		{
			return configurationValues[i].stringValue;
		}
		i++;
	}

	return NULL;
}

int getConfigurationKeyIntValue(char *key)
{
	int i=0;
		while (configurationKeys[i])
		{
			if (strcmp(configurationKeys[i], key)==0)
			{
				return atoi(configurationValues[i].stringValue);
			}
			i++;
		}

	return -9999;
}

int modifyConfigurationKey(char *key, char *value)
{
	int location=0;
	while (configurationKeys[location])
	{
		//printf("\nCompara %s con %s", key, configurationKeys[location]);
		if (strcmp(configurationKeys[location], key)==0)
		{
			    	configurationValues[location].stringValue=strdup(value);
			    	return 0;
  		}
		location++;
	}

	//printf("No se encontro la clave %s", key);
	//No se encontro la clave
	return -1;
}

int containsCSL(char *key, const char *value)
{
	if (!key||!value) return 0;
	char *valueList=getConfigurationKeyStringValue(key);

	if (!valueList) return 0;

	if(strstr(valueList, value) != NULL) {

		    return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
//
// WSI
//
//Aqui llega una cadena VENDOR_MODEL_IP_SN y obtiene su WSI
struct lws *getwsifromChargePointString(char *chargePoint)
{
	char** tokens = str_split(chargePoint, '_');
	char *vendor=NULL;
	char *model=NULL;
	char *IP=NULL;
	char *SN=NULL;

	if (tokens)
	{
		vendor=*(tokens);
		model=*(tokens+1);
		IP=*(tokens+2);
		SN=*(tokens+3);
	}

	int i=0;
	if (SN)
	{
		while (i<numCurrentlyConnected)
		{
			//printf("\n1.- los tokens son: %s, %s, %s", IP, SN, model);
			//printf("\n2.- los tokens son: %s, %s, %s", connected_ChargePoints[i].IP, connected_ChargePoints[i].SN, connected_ChargePoints[i].Model);
			if (strcmp(connected_ChargePoints[i].IP, IP)==0 && strcmp(connected_ChargePoints[i].SN, SN)==0 && strcmp(connected_ChargePoints[i].Model, model)==0)
			{
				return connected_ChargePoints[i].wsi;
			}
			i++;
		}
	}
	else
	{
		while (i<numCurrentlyConnected)
		{
			//printf("\n%s vs %s, %s vs %s",connected_ChargePoints[i].IP, IP, connected_ChargePoints[i].Model, model);
			if (strcmp(connected_ChargePoints[i].IP, IP)==0 && strcmp(connected_ChargePoints[i].Model, model)==0)
			{
				return connected_ChargePoints[i].wsi;
			}
			i++;
		}
	}

	return NULL;
}



char *getChargePointStringFromWSI(struct lws *wsi)
{
	int i=0;
	char *token=NULL;

	while (i<numCurrentlyConnected)
	{
		if (connected_ChargePoints[i].wsi==wsi)
		{
			token=(char *)calloc(1,512);
			strcpy(token,connected_ChargePoints[i].Vendor);
			strcat(token,"_");
			strcat(token,connected_ChargePoints[i].Model);
			strcat(token,"_");
			strcat(token,connected_ChargePoints[i].IP);

			if (connected_ChargePoints[i].SN)
			{
				strcat(token,"_");
				strcat(token,connected_ChargePoints[i].SN);
			}
		}
	}

	return token;
}

//Esta funcion obtiene una estructura chargePoint a partir de la cadena de chargepoitn
struct chargePoint *getChargePointFromString(char *chargePoint)
{
		char** tokens = str_split(chargePoint, '_');
		char *vendor=NULL;
		char *model=NULL;
		char *IP=NULL;
		char *SN=NULL;

		if (tokens)
		{
			vendor=*(tokens);
			model=*(tokens+1);
			IP=*(tokens+2);
			SN=*(tokens+3);
		}

		for (int i=0; i<numCurrentlyConnected; i++)
		{
			if (SN && strcmp(connected_ChargePoints[i].SN,SN)==0) return &connected_ChargePoints[i];
			else
			{
				if ((strcmp(connected_ChargePoints[i].IP,IP)==0)
						&& (strcmp(connected_ChargePoints[i].Model,model)==0)
						&& (strcmp(connected_ChargePoints[i].Vendor,vendor)==0)) return &connected_ChargePoints[i];
			}
		}

		return NULL;
}

struct chargePoint *getChargePointStructFromWSI(struct lws *wsi)
{
	int i=0;
	while (i<numCurrentlyConnected)
	{
		if (connected_ChargePoints[i].wsi==wsi)
		{
			return &connected_ChargePoints[i];
		}
	}

	return NULL;
}


//returns NULL if not found
struct chargePoint *getChargePointFromWSI(struct lws *wsi)
{
	struct chargePoint *temp=(struct chargePoint *)calloc(1, sizeof(struct chargePoint));

	for (int i=0; i<numCurrentlyConnected; i++)
	{
		if (connected_ChargePoints[i].wsi==wsi)
		{
			char *IP=(char *)calloc(15, sizeof(char));
			strcpy(IP, connected_ChargePoints[i].IP);

			char *Model=(char *)calloc(20, sizeof(char));
			strcpy(Model, connected_ChargePoints[i].Model);

			char *Vendor=(char *)calloc(20, sizeof(char));
			strcpy(Vendor, connected_ChargePoints[i].Vendor);

			char *SN=(char *)calloc(25, sizeof(char));
			strcpy(SN, connected_ChargePoints[i].SN);

			temp->IP=IP;
			temp->Model=Model;
			temp->SN=SN;
			temp->Vendor=Vendor;
			temp->wsi=wsi;

			return temp;
		}
	}

	return NULL;
}

char *getChargePointString(struct chargePoint *cp)
{
	int length=0;

	if (cp->IP) length+=strlen(cp->IP);
	if (cp->Model) length+=strlen(cp->Model);
	if (cp->Vendor) length+=strlen(cp->Vendor);
	if (cp->SN) length+=strlen(cp->SN);
	length+=4;

	char *cp_str=(char *)calloc(1, length);

	if (cp->Model) strcat(cp_str, cp->Model);
	strcat(cp_str,"_");
	if (cp->Vendor) strcat(cp_str, cp->Vendor);
	strcat(cp_str,"_");
	if (cp->IP) strcat(cp_str, cp->IP);
	strcat(cp_str,"_");
	if (cp->SN) strcat(cp_str, cp->SN);

	return cp_str;
}

//Esto se llama desde BootNotification para añadir un nuevo ChargePoint.
//Actualiza el array connected_ChargePoints
//FALTA POR AÑADIR QUE SE ELIMINEN CHARGE POINTS AL DESCONECTARSE <-- NOT IMPLEMENTED
int addConnectedChargePoint(json_object *Payload, struct lws *wsi)
{
	const char *chargePointModel=NULL;
	const char *chargePointVendor=NULL;
	const char *chargePointSerialNumber=NULL;

	json_object *obj_chargePointModel = json_object_object_get(Payload, "chargePointModel");
	if (!obj_chargePointModel) return 0;
	chargePointModel=json_object_get_string(obj_chargePointModel);

	json_object *obj_chargePointVendor=json_object_object_get(Payload, "chargePointVendor");
	if (!obj_chargePointVendor) return 0;
	chargePointVendor=json_object_get_string(obj_chargePointVendor);

	json_object *obj_chargePointSerialNumber=json_object_object_get(Payload, "chargePointSerialNumber");

	//Primero lo añadimos a las estructuras de control internas
	if (!obj_chargePointSerialNumber)
	{
		chargePointSerialNumber=NULL;
	}
	else
	{
		chargePointSerialNumber=json_object_get_string(obj_chargePointSerialNumber);
		connected_ChargePoints[numCurrentlyConnected].SN=(char *)calloc(25, sizeof(char));
	}

	connected_ChargePoints[numCurrentlyConnected].IP=(char *)calloc(15, sizeof(char));
	connected_ChargePoints[numCurrentlyConnected].Model=(char *)calloc(20, sizeof(char));
	connected_ChargePoints[numCurrentlyConnected].Vendor=(char *)calloc(20, sizeof(char));

	char name[64];
	char ip[16];

	int fd = lws_get_socket_fd(wsi);
	lws_get_peer_addresses(wsi, fd, name, 64, ip, 16);

	//Obtenemos la fecha actual
	time_t now;
	time(&now);
	struct tm* now_tm;
	now_tm = localtime(&now);
	char *currentTime=(char *)calloc(80, sizeof(char));
	strftime (currentTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
	//

	//Los tres primeros son obligatorios, el cuarto es un campo opcional
	strncpy(connected_ChargePoints[numCurrentlyConnected].IP, ip, 16);
	strncpy(connected_ChargePoints[numCurrentlyConnected].Model, chargePointModel, 20);
	strncpy(connected_ChargePoints[numCurrentlyConnected].Vendor, chargePointVendor, 20);
	if (obj_chargePointSerialNumber){
		strncpy(connected_ChargePoints[numCurrentlyConnected].SN, chargePointSerialNumber, 25);
	}
	else
	{
		connected_ChargePoints[numCurrentlyConnected].SN=NULL;
	}

	connected_ChargePoints[numCurrentlyConnected].wsi=wsi;

	numCurrentlyConnected++;

	////////////////////////////////////
	//Luego lo añadimos a la BD
	if (conn)
	{
			json_object *obj_chargeBoxSerialNumber=json_object_object_get(Payload, "chargeBoxSerialNumber");
			json_object *obj_firmwareVersion=json_object_object_get(Payload, "firmwareVersion");
			json_object *obj_iccid=json_object_object_get(Payload, "iccid");
			json_object *obj_imsi=json_object_object_get(Payload, "imsi");
			json_object *obj_meterSerialNumber=json_object_object_get(Payload, "meterSerialNumber");
			json_object *obj_meterType=json_object_object_get(Payload, "meterType");

			char *chargeBoxSerialNumber=NULL, *firmwareVersion=NULL, *iccid=NULL, *imsi=NULL, *meterSerialNumber=NULL, *meterType=NULL;

			if (obj_chargeBoxSerialNumber) chargeBoxSerialNumber=json_object_get_string(obj_chargeBoxSerialNumber);
			if (obj_firmwareVersion) firmwareVersion=json_object_get_string(obj_firmwareVersion);
			if (obj_iccid) iccid=json_object_get_string(obj_iccid);
			if (obj_imsi) imsi=json_object_get_string(obj_imsi);
			if (obj_meterSerialNumber) meterSerialNumber=json_object_get_string(obj_meterSerialNumber);
			if (obj_meterType) meterType=json_object_get_string(obj_meterType);

			insertConnectionIntoDB(chargePointModel, chargePointVendor, chargePointSerialNumber, ip, chargeBoxSerialNumber, firmwareVersion, iccid, imsi, meterSerialNumber, meterType, currentTime);
	}
	else printf("NO SE PUDO INTRODUCIR LA CONEXION EN LA BD");

	return 1;
}

//
//Esto se llama desde Boot Notification como paso previo al anterior para comprobar que el Charge Point que se ha conectado esta entre los permitidos
//
int checkValidChargePoint(json_object *Payload)
{
	const char *chargePointModel=NULL;
	const char *chargePointVendor=NULL;

	json_object *obj_chargePointModel = json_object_object_get(Payload, "chargePointModel");
	if (obj_chargePointModel) chargePointModel=json_object_get_string(obj_chargePointModel);

	json_object *obj_chargePointVendor=json_object_object_get(Payload, "chargePointVendor");
	if (obj_chargePointVendor) chargePointVendor=json_object_get_string(obj_chargePointVendor);

	if (!chargePointModel || !chargePointVendor) return 0;

	//if (debug) printf("\n[LOG] El model es %s y el vendor es %s",chargePointModel,chargePointVendor);

	//Comprueba el array allowedChargePointModels[i] para ver si el Modelo y el vendor estan permitidos
	for (int i=0; i<sizeof(allowedChargePointModels)/sizeof(allowedChargePointModels[0]); i++)
	{
		//if (debug) printf("\n[LOG] Y se compara con %s y con %s",allowedChargePointModels[i].chargePointModel.x,allowedChargePointModels[i].chargePointVendor.x);

		if ((strcmp(trimwhitespace(strlwr(allowedChargePointModels[i].chargePointModel.x)), trimwhitespace(strlwr(chargePointModel)))==0) &&
				(strcmp(trimwhitespace(strlwr(allowedChargePointModels[i].chargePointVendor.x)), trimwhitespace(strlwr(chargePointVendor)))==0))
		{
			return 1;
		}
	}
	//if (debug) printf("\n[LOG] Y NO LO ENCUENTRA");
	return 0;
}

int getConnectorNumber(char *chargePointVendor, char *chargePointModel)
{
	for (int i=0; i<sizeof(allowedChargePointModels)/sizeof(allowedChargePointModels[0]); i++)
		{
			if ((strcmp(trimwhitespace(strlwr(allowedChargePointModels[i].chargePointModel.x)), trimwhitespace(strlwr(chargePointModel)))==0) &&
					(strcmp(trimwhitespace(strlwr(allowedChargePointModels[i].chargePointVendor.x)), trimwhitespace(strlwr(chargePointVendor)))==0))
			{
				return allowedChargePointModels[i].numConnectors;
			}
		}
	return -1;
}

/////////////////////////////////////////////////////
//          IDTAGS
/////////////////////////////////////////////////////

int addIdtagtoGroup( char *parentIdtag, char *IdTag)
{
	struct parentIdTagRecord *n;
	n=idTagGroups;

	while (n)
	{
		if (strcmp(n->parentIdTag, parentIdtag)==0)
		{
			struct idTagRecord *nodoIdTag, *aux;
			nodoIdTag=n->idTags;
			aux=n->idTags;

			while (nodoIdTag)
			{

				//SI YA ESTA DEBE DEVOLVER ERROR
				if (strcmp(nodoIdTag->idTag, IdTag)==0)
				{
					return 0;
				}

				aux=nodoIdTag;
				nodoIdTag=nodoIdTag->next;
			}

			//Allocate space for the new idTag Token
			nodoIdTag=(struct idTagRecord *)calloc(1,sizeof (struct idTagRecord));
			if (!nodoIdTag) return 0;

			strncpy(nodoIdTag->idTag,IdTag,20);
			nodoIdTag->next=NULL;
			aux->next=nodoIdTag;

			return 1;
		}
	}

	return 0;
}

int removeIdtagFromGroup(char *parentIdtag, char *IdTag)
{
	struct parentIdTagRecord *n;
	n=idTagGroups;

	while (n)
	{
		if (strcmp(n->parentIdTag, parentIdtag)==0)
		{
			struct idTagRecord *nodoIdTag, *aux;
			nodoIdTag=n->idTags;
			aux=n->idTags;

			while (nodoIdTag)
			{
				if (strcmp(nodoIdTag->idTag, IdTag)==0)
				{
					aux->next=nodoIdTag->next;
					free(nodoIdTag->idTag);
					free(nodoIdTag);
					return 1;
				}

				aux=nodoIdTag;
				nodoIdTag=nodoIdTag->next;
			}
		}
		n=n->next;
	}

	//NO SE PUDO ELIMINAR
	return 0;


}

struct idTagRecord* getIdTagsFromGroup( char *parentIdtag)
{
	struct parentIdTagRecord *n, aux2;
	n=idTagGroups;
	int num=0;

	while (n)
	{
		if (strcmp(n->parentIdTag, parentIdtag)==0)
		{
			return n->idTags;
		}
	}

	return NULL;
}

//Si el segundo parametro no es NULL, lo añade al listado de IdTags del ParentIdTag
int addParentIdTag(char *parentIdTag, char *IdTag)
{
	struct idTagRecord *nodoIdTag=NULL;
	struct parentIdTagRecord *nodo=(struct parentIdTagRecord *)calloc(1,sizeof(struct parentIdTagRecord));

	if (!nodo) return 0;

	char *nombre=(char *)calloc(1,sizeof(char)*20);
	if (!nombre) return 0;

	strncpy(nombre, parentIdTag, 20);
	nodo->parentIdTag=nombre;

	if (IdTag)
	{
		nodoIdTag=(struct idTagRecord *)calloc(1,sizeof(struct idTagRecord));
		if (!nodoIdTag) return 0;

		strncpy(nodoIdTag->idTag, IdTag, 20);
		nodoIdTag->next=NULL;
	}


	nodo->idTags=nodoIdTag;
	nodo->next=NULL;

	struct parentIdTagRecord *n=idTagGroups;

	if (n)
	{
		while (n->next)
		{
			n=n->next;
		}
		n->next=nodo;
	}
	else idTagGroups=nodo;

	return 1;
}

int removeParentIdTag(char *parentIdtag)
{
	struct parentIdTagRecord *n, *aux2;
	n=idTagGroups;
	aux2=idTagGroups;
	int num=0;

	while (n)
	{
		if (strcmp(n->parentIdTag, parentIdtag)==0)
		{
			struct idTagRecord *nodoIdTag, *aux;
			nodoIdTag=n->idTags;

			while (nodoIdTag){
				aux=nodoIdTag;
				free (nodoIdTag->idTag);
				nodoIdTag=nodoIdTag->next;
				free(aux);
			}

			aux2->next=n->next;
			free(n);
			return 1;
		}

		aux2=n;
		n=n->next;
	}

	//NO SE PUDO ELIMINAR
	return 0;
}

int isIdTagsInGroup( char *parentIdtag, char *IdTag)
{
	struct parentIdTagRecord *n=idTagGroups;

	while (n)
	{
		if (strcmp(n->parentIdTag, parentIdtag)==0)
		{
			struct idTagRecord *nodoIdTag, *aux;
			nodoIdTag=n->idTags;

			while (nodoIdTag)
			{
				if (strcmp(nodoIdTag->idTag, IdTag)==0)
				{
					//ENCONTRADO
					return 1;
				}
				nodoIdTag=nodoIdTag->next;
			}
		}

		n=n->next;
	}

	//NO ENCONTRADO
	return 0;
}


/*
typedef struct idTagRecord{
	char *idTag;
	struct idTagRecord *next;
};

typedef struct parentIdTagRecord{
	char *parentIdTag;
	struct idTagRecord *idTags;
};

struct parentIdTagRecord *idTagGroups;
*/

/////////////////////////////////////////////////////
// RESERVATIONS
/////////////////////////////////////////////////////


int getReservationID(int i){
	if (i>=numReservations) return NULL;

	return reservations[i].reservation_id;
}

char *getReservation(int i)
{
	if (i>=numReservations) return NULL;

	char *reserva=(char *)calloc(60,sizeof(char));

	sprintf(reserva, "%s_%s_%s_%d", reservations[i].Vendor, reservations[i].Model, reservations[i].IP, reservations[i].Connector);
	return reserva;
}

int addPendingReservation(char *vendor, char *model, char *IP, int Connector, int id, char *expiryDate, char *idTag)
{
	struct reservationsList *reserva=(struct reservationsList *)calloc(1, sizeof(struct reservationsList));

	if (!reserva) return 0;

	reserva->Connector=Connector;
	reserva->IP=strdup(IP);

	//La añadimos a las estructuras de control
	reserva->Model=strdup(model);
	reserva->Vendor=strdup(vendor);
	reserva->expiryDate=strdup(expiryDate);
	reserva->status=-1;
	reserva->reservation_id=id;
	reserva->next=NULL;

	if (pending_reservations)
	{
		//SE INSERTA AL PRINCIPIO DE LA LISTA
		reserva->next=pending_reservations->next;
	}
	pending_reservations=reserva;

	//La añadimos a la BD
	insertReservationIntoDatabase(vendor, model, IP, Connector, id, expiryDate, idTag);


	return 1;
}

//Devuelve el numero de conector de la reserva
//O -1 si no pudo realizarla
int removePendingReservation(int reservationId)
{
	//Si nos llega una reserva negativa nos devuelve -1
 if (reservationId>=0)
 {
	struct reservationsList *reserva, *aux;
	reserva=pending_reservations;
	aux=pending_reservations;

	int conector=-1;

	while (reserva)
	{
		if (reserva->reservation_id==reservationId)
		{
			//Si solo habia un registro
			if (reserva==pending_reservations)
			{
				pending_reservations=NULL;
			}
			else
			{
				aux->next=reserva->next;
			}

			//	struct reservationRecord *reservation;
			//ELIMINAMOS LA RESERVA
			if (reserva->IP) free(reserva->IP);
			if (reserva->Vendor) free(reserva->Vendor);
			if (reserva->Model) free(reserva->Model);
			if (reserva->expiryDate) free(reserva->expiryDate);
			conector=reserva->Connector;
			free(reserva);

			return conector;
		}

		aux=reserva;
		reserva=reserva->next;
	}

	//NO SE ENCONTRO LA RESERVA
	return conector;
 }
 else
 {
	 return -1;
 }
}

//Devuelve 0 si no se encontró la reserve y 1 si la eliminó
int removeLastPendingReservation(char *IP)
{
	struct reservationsList *reserva, *aux;
	reserva=pending_reservations;
	aux=pending_reservations;

	printf("SE ESTA ELIMINANDO LAST PENDING RESERVATION");

	printf("LA IP ES: %s", IP);
	while (reserva)
	{
		if (pending_reservations) printf("PENDING RESERVATIONS NO ES NULL");

		if (strcmp(reserva->IP, IP)==0)
		{

			expireReservationIdFromDb(reserva->reservation_id);

			//Si solo habia un registro
			if (reserva==pending_reservations){
				pending_reservations=reserva->next;
			}
			else
			{
				aux->next=reserva->next;
			}

			printf("SE ESTA ELIMINANDO LAST PENDING RESERVATION");

			free(reserva->IP);
			free(reserva->idTag);
			free(reserva->Vendor);
			free(reserva->expiryDate);
			free(reserva->Model);
			free(reserva);
			return 1;
		}

		aux=reserva;
		reserva=reserva->next;
	}

	//NO SE ENCONTRO LA RESERVA
	return 0;
}

void checkReservationExpirations()
{
	struct reservationsList *aux, *aux2;
	aux=pending_reservations;
	aux2=pending_reservations;

	__time_t now;
	time (&now);
	struct tm * timeinfo= localtime (&now);

	while (aux)
	{
		struct tm * _fecha;
		strptime(aux->expiryDate, "%Y-%m-%dT%H:%M:%S.", &_fecha);
		__time_t fecha = mktime(_fecha);
		double diffSecs = difftime(fecha, now);

		//Lo eliminamos
		if (diffSecs<0.0)
		{
			aux->next=aux2->next;
			free(aux->IP);
			free(aux->Model);
			free(aux->Vendor);
			free(aux->expiryDate);
			free(aux);
		}

		aux2=aux;
		aux=aux->next;
	}

	aux=reservations;
	aux2=reservations;
	while (aux)
	{
		struct tm * _fecha;
		strptime(aux->expiryDate, "%Y-%m-%dT%H:%M:%S.", &_fecha);
		__time_t fecha = mktime(_fecha);
		double diffSecs = difftime(fecha, now);

		//Lo eliminamos
		if (diffSecs<0.0)
		{
			aux->next=aux2->next;
			free(aux->IP);
			free(aux->Model);
			free(aux->Vendor);
			free(aux->expiryDate);
			free(aux);
		}
	}
}

//Si reservationId es -1 es que no tenemos el reservationId
void removeReservation(int reservationId)
{
	if (reservationId==-1) return;

	struct reservationsList *p, *ant;
	struct ReservationRecord *rd;
	p=reservations;
	ant=reservations;

	//printf("IS es:%d", reservationId);
	while (p)
	{
		if (p->reservation_id==reservationId)
		{
			if (ant==reservations)
			{
				reservations=p->next;
			}
			else
			{
				ant->next=p->next;
			}

			free(p->IP);
			free(p->Model);
			free(p->Vendor);
			free(p->expiryDate);
			free(p->idTag);
			free(p);
			numReservations--;
		//	printf("NONO");
			return;
		}

		ant=p;
		p=p->next;
	}

//	printf("FIN");
}


//Elimina una entrada de la pending list
void acceptLastPendingReservation(char *IP, int status)
{
	struct reservationsList *reserva, *aux;
	//struct reservationsList *pending_reservations;
	reserva=pending_reservations;
	aux=pending_reservations;

//	printf("\nreserva apunta a %p", reserva);
//	printf("\npending_reservations apunta a: %p", pending_reservations);

//	printf("\la IP es: %s", pending_reservations->IP);
//	printf("\la IP de reserva es: %s", reserva->IP);

	if (status==0)
	{
		//ACEPTADO


		while (reserva)
		{
		//	printf("\nAhora, la IP es: %s", IP);
		//	printf("\la IP de reserva es: %s", reserva->IP);

			//printf("\nAl añadirla, el reservation id es: %d", reserva->reservation_id);

			if (strcmp(reserva->IP, IP)==0)
			{
				reserva->status=status;

				//Lo borramos de pending
				if (aux==pending_reservations)
				{
					pending_reservations=NULL;
				}
				else
				{
					aux->next=reserva->next;
				}

				//Lo añadimos al listado de reservas finales
				if (!reservations)
				{
					reservations=reserva;
				}
				else
				{
					//SE INTRODUCE AL PRINCIPIO
					reserva->next=reservations->next;
					reservations=reserva;
				}

				updateReservationInDB(reserva->reservation_id, status);
				numReservations++;
			}
			aux=reserva;
			reserva=reserva->next;
		//	reserva=NULL;
		}
	}

}

/*
void reserve(char *vendor, char *model, char *IP, int Connector){



	if (!reservations)
	{

	}

	reservations[numReservations].Connector=Connector;

	reservations[numReservations].IP=(char *)malloc(sizeof(char)*16);
	reservations[numReservations].Model=(char *)malloc(sizeof(char)*20);
	reservations[numReservations].Vendor=(char *)malloc(sizeof(char)*20);

	strncpy(reservations[numReservations].IP, IP, 16);
	strncpy(reservations[numReservations].Vendor, vendor, 20);
	strncpy(reservations[numReservations].Model, model, 20);

	numReservations++;
}

*/


int getNextReservationId()
{
	return 	numReservationId++;

}
/////////////////////////////////////////////////////
// TRANSACTIONS
/////////////////////////////////////////////////////

/////////////////////////////////////////////
// Returns 0 si todo fue bien
// Returns 1 si no encontró el chargepoint
// Returns 2 si lo encontró pero ya habia un transaccion en curso.
int addTransaction(struct chargePoint *cp, int transactionId, int connector, int meterStart)
{
	int i;
	struct transaction *trans;

	printf("EN ADD TRANSACTION");
	//Buscamos en que posición se encuentra el chargepoint dentro de connected_chargepoints, porque en esa misma posicion vamos a colocar la transaccion en currentTransactions
	for (i=0; i<MAX_NUM_CHARGEPOINTS; i++)
	{
		if (connected_ChargePoints[i].IP && connected_ChargePoints[i].SN)
		{
			if ((strcmp(connected_ChargePoints[i].IP,cp->IP)==0) && strcmp(connected_ChargePoints[i].SN,cp->SN)==0)
			{
				break;
			}
		}
	}

	//Si no encuentra el Chargepoint devuelve 1
	if (i==MAX_NUM_CHARGEPOINTS) return 1;

	printf("\nEN ADD TRANSACTION2");
	struct transaction *t=NULL;
	t=currentTransactions[i];

	//Si ya hay una transaction devuelve 2
	if (t)
	{
		//Ya hay una transaccion en ese chargepoint, a ver si es en el mismo conector...
		while (t && t->connectorId!=connector)
		{
			t=t->next;
		}

		if (t) return 2; //Ya hay una transaccion en ese conector.
	}
	else
	{
		trans=(struct transaction *)calloc(1, sizeof(struct transaction));
		trans->transactionId=transactionId;
		trans->CP=cp;
		trans->connectorId=connector;
		trans->transactionStartValue=meterStart;
		trans->next=NULL;
		currentTransactions[i]=trans;

		//Obtenemos la fecha actual
			time_t now;
			time(&now);
			struct tm* now_tm;
			now_tm = localtime(&now);
			char *currentTime=(char *)calloc(80, sizeof(char));
			strftime (currentTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
			//

			printf("\nLLAMAMOS A insertTransactionIntoDb");
		insertTransactionIntoDb(cp,transactionId, connector, meterStart, currentTime);
	}

	return 0;
}

// Libera una transaccion dada con un transactionId dado
// devuelve 1 si la pudo liberar exitosamente
// devuelve 0 si hubo algun problema (no encontro ese transactionId)
//
int removeTransaction(int transactionId)
{
	int i;
	struct transaction *t, *ant;
	for (i=0; i<MAX_NUM_CHARGEPOINTS; i++)
		{
			t=currentTransactions[i];
			ant=t;
			while (t)
			{
				if (t->transactionId==transactionId)
				{
					if (ant==t) currentTransactions[i]=t->next;
					else ant->next=t->next;

					//Yo no quiero borrarla, sino actualizar la fecha de parada
					//removeTransactionFromDb(transactionId);
					updateTransactionStopTimeOnDb(transactionId);
					free(t);
					return 1;
				}
				ant=t;
				t=t->next;
			}
		}
	return 0;
}

//
// Auxiliary functions
//
queue_node * create_empty_queue_node(int messageType, const char *chargePoint)
{
	queue_node *nuevo_mensaje;
	int n=getNextUniqueID();
	nuevo_mensaje = (queue_node *)calloc(1,sizeof(queue_node));
	nuevo_mensaje->Messagetype=messageType;
	nuevo_mensaje->UniqueId=n;
	nuevo_mensaje->sent=0; //NOT SENT YET
	nuevo_mensaje->wsi=getwsifromChargePointString(chargePoint);

	nuevo_mensaje->next=NULL;
	return nuevo_mensaje;
}


////////////////////////////////////////////////
//         FUNCIONES OCPP
////////////////////////////////////////////////

//
// 4.2
//
char *respondBootNotification(json_object * obj_UniqueId, json_object * Payload, char ip[16], struct lws *wsi)
{
	//Primero comprobamos el correcto formato del mensaje que llegó. En caso de error devolvemos mensaje de error
	//
	  if (!obj_UniqueId) return prepareErrorResponse(obj_UniqueId, _PROTOCOL_ERROR);

 	  json_object *obj_chargePointModel = json_object_object_get(Payload, "chargePointModel");
   	  if (!obj_chargePointModel) return prepareErrorResponse(obj_UniqueId, _PROTOCOL_ERROR);
   	  const char *chargePointModel=json_object_get_string(obj_chargePointModel);

   	  json_object *obj_chargePointVendor=json_object_object_get(Payload, "chargePointVendor");
   	  if (!obj_chargePointVendor) return prepareErrorResponse(obj_UniqueId, _PROTOCOL_ERROR);
   	  const char *chargePointVendor=json_object_get_string(obj_chargePointVendor);

   	  const char *chargePointSerialNumber=NULL;
   	  json_object *obj_chargePointSerialNumber=json_object_object_get(Payload, "chargePointSerialNumber");

   	  //En caso de que en el mensaje no llegara el Serial Number, el valor que almacenamos para el listado de CPs conectados es el UniqueId.   	  //¿¿CAMBIAR??
   	  if (obj_chargePointSerialNumber)
   	  {
   		  chargePointSerialNumber=json_object_get_string(obj_chargePointSerialNumber);
   	  }
   	  else
   	  {
   		//Esto no va a dar nunca error porque se ha comprobado antes
  		chargePointSerialNumber=json_object_get_string(obj_UniqueId);
  	  }

   	//Comprobamos si es un Charge point permitido.
	char *status=NULL;
    if (checkValidChargePoint(Payload))
    {
    	status="Accepted";
    	addConnectedChargePoint(Payload, wsi);
    	addGUIChargePoint(chargePointVendor, chargePointModel, ip, chargePointSerialNumber);//<-- Debe ser implementado por el GUI
   // 	AddLog("\n[LOG] BootNotification requested replied with status Accepted", 0);
	}
    else
    {
    	status="Rejected";
  //  	AddLog("\n[LOG] BootNotification requested replied with status REJECTED", 0);
    }

    return prepareBootNotificationResponse(obj_UniqueId, Payload, status);
}


char *manageMeterValues(const char *UniqueId, int connectorId, int transactionId, json_object *obj_meterValue)
{
	json_object *obj_sampledValue, *obj_value, *obj_format,*obj_measurand,*obj_phase,*obj_location,*obj_unit,*obj_context;
	const char *timestamp;
	char *value, *context_str, *format_str, *measurand_str, *phase_str, *location_str, *unit_str;
	int context, format, measurand, phase, location, unit;

	int numMeterValues=-1;
	if (obj_meterValue) numMeterValues=json_object_array_length(obj_meterValue);
	//else printf("NO LLEGA METERVALUE!!!");


	json_object *obj_MV=NULL;
	json_object *obj_timestamp=NULL;
	json_object *obj_sampledValues=NULL;

   for (int hh=0; hh<numMeterValues; hh++)
   {
	 obj_MV=json_object_array_get_idx (obj_meterValue, hh);
	 if (obj_MV) obj_timestamp=json_object_object_get(obj_MV, "timestamp");
	 if (obj_MV) obj_sampledValues=json_object_object_get(obj_MV, "sampledValue");

	 int num=-1;
	 if (obj_timestamp) timestamp=json_object_get_string(obj_timestamp);

    if (obj_sampledValues) num=json_object_array_length(obj_sampledValues);
    else printf("NO LLEGA sampledValues!!!");

  //  printf("\nA");
    for (int ii=0; ii<num; ii++)
    {
    	obj_sampledValue=json_object_array_get_idx (obj_sampledValues, ii);
    	//printf("\nB");

    	//Value
    	if (obj_sampledValue) obj_value=json_object_object_get(obj_sampledValue, "value");
    	if (obj_value) value=json_object_get_string(obj_value);

    	//Context
    	context=-1;
    	if (obj_sampledValue) obj_context=json_object_object_get(obj_sampledValue, "context");
    	if (obj_context)
    	{
    		if (enums_as_integers)
    		{
    			context=json_object_get_int(obj_context);
    		}
    		else
    		{
    			context_str=json_object_get_string(obj_context);

    			if (context_str)
    			{
    				for (int i=0; i<8; i++)
    				{
    					if (strcmp(context_str, Context_texts[i])==0)
    					{
    						context=i;
    						break;
    					}
    				}
    			}
    		}
    	}

    	//Format
    	format=-1;
    	if (obj_sampledValue) obj_format=json_object_object_get(obj_sampledValue, "format");
    	if (obj_format)
    	{
    		if (enums_as_integers)
    		{
    			format=json_object_get_int(obj_format);
    		}
    		else
    		{
    			format_str=json_object_get_string(obj_format);
    			if (format_str)
    			{
    				for (int i=0; i<2; i++)
    				{
    					if (strcmp(format_str, Format_Texts[i])==0)
    					{
    						format=i;
    						break;
    					}
    				}
    			}
    		}
    	}

    	//Measurand
    	measurand=-1;
    	    	if (obj_sampledValue) obj_measurand=json_object_object_get(obj_sampledValue, "measurand");
    	    	if (obj_measurand)
    	    	{
    	    		if (enums_as_integers)
    	    		{
    	    			measurand=json_object_get_int(obj_measurand);
    	    		}
    	    		else
    	    		{
    	    			measurand_str=json_object_get_string(obj_measurand);

    	    			for (int i=0; i<22; i++)
    	    			{
    	    				if (measurand_str && strcmp(measurand_str, Measurand_texts[i])==0)
    	    				{
    	    					measurand=i;
    	    					break;
    	    				}
    	    			}
    	    		}
    	    	}


    	    	//Phase
    	    	phase=-1;
    	    	    	if (obj_sampledValue) obj_phase=json_object_object_get(obj_sampledValue, "phase");
    	    	    	if (obj_phase)
    	    	    	{
    	    	    		if (enums_as_integers)
    	    	    		{
    	    	    			phase=json_object_get_int(obj_phase);
    	    	    		}
    	    	    		else
    	    	    		{
    	    	    			phase_str=json_object_get_string(obj_phase);

    	    	    			for (int i=0; i<10; i++)
    	    	    			{
    	    	    				if (phase_str && strcmp(phase_str, Phase_texts[i])==0)
    	    	    				{
    	    	    					phase=i;
    	    	    					break;
    	    	    				}
    	    	    			}
    	    	    		}
    	    	    	}

    	    	    	//Location
    	    	    	location=-1;
    	    	    	    	if (obj_sampledValue) obj_location=json_object_object_get(obj_sampledValue, "location");
    	    	    	    	if (obj_location)
    	    	    	    	{
    	    	    	    		if (enums_as_integers)
    	    	    	    		{
    	    	    	    			location=json_object_get_int(obj_location);
    	    	    	    		}
    	    	    	    		else
    	    	    	    		{
    	    	    	    			location_str=json_object_get_string(obj_location);

    	    	    	    			for (int i=0; i<10; i++)
    	    	    	    			{
    	    	    	    				if (location_str && strcmp(location_str, Location_texts[i])==0)
    	    	    	    				{
    	    	    	    					location=i;
    	    	    	    					break;
    	    	    	    				}
    	    	    	    			}
    	    	    	    		}
    	    	    	    	}

    	    	    	    	//Unit
    	    	    	    	unit=-1;
    	    	    	    	if (obj_sampledValue) obj_unit=json_object_object_get(obj_sampledValue, "unit");
    	    	    	    	if (obj_unit)
    	    	    	    	{
    	    	    	    		if (enums_as_integers)
    	    	    	    	    {
    	    	    	    	    	unit=json_object_get_int(obj_unit);
    	    	    	    	    }
    	    	    	    	    else
    	    	    	    	    {
    	    	    	    	    	unit_str=json_object_get_string(obj_unit);

    	    	    	    	    	for (int i=0; i<10; i++)
    	    	    	    	    	{
    	    	    	    	    		if (unit_str && strcmp(unit_str, UnitOfMeasureTexts[i])==0)
    	    	    	    	    	    {
    	    	    	    	    	    	unit=i;
    	    	    	    	    	    	break;
    	    	    	    	    	    }
    	    	    	    	    	}
    	    	    	    	   }
    	    	    	    	}

    	// if (debug) printf("EL METER VALUES RECIBE: VALUE: %s, TIMESTAMP: %s, CONTEXT: %d, FORMAT: %d, MEASURAND: %d, PHASE: %d, LOCATION: %d, UNIT: %d",value, timestamp, context, format,measurand,phase, location, unit );
///Ahora que tenemos todos los datos, metemos los meter values en la BD

         insertMeterValueIntoDB(connectorId, transactionId, timestamp, value, context, format,measurand,phase, location, unit);
    } ///Fin del for
   }

   return prepareMeterValuesResponse(UniqueId);
}


////////////////////////////////////
//
// 4.8.- Start Transaction Request
//
char *respondStartTransactionRequest(const char *UniqueId, int connector, const char *idTag, int meterStart, int reservationId, const char *timeStamp, struct lws *wsi)
{
	//Pag 40: The Central System MUST verify validity of the identifier in the StartTransaction.req PDU, because the
	//identifier  might  have  been  authorized  locally  by  the  Charge  Point  using  outdated  information.  The
	//identifier, for instance, may have been blocked since it was added to the Charge Point’s Authorization Cache.

	//Cuando recibimos un starttransacrtion, aumentamos el numero de transaccion, eliminamos la reserva y... ¿¿??
	//int transaction=transactionId++;
	int transaction=getLastTransactionId()+1;

//	printf("\nLA PROXIMA TRANSACCION SERA: %d", transaction);

	//Pag 39: If this transaction ends a reservation (See Reserve Now operation) the the startTransaction.req MUST contain the reservationId

	if (reservationId>=0) removePendingReservation(reservationId);

	///GET AUTHORIZATION STATUS
	//Pag 39 y 40: Upon   receipt   of   a	StartTransaction.req   PDU,   the   Central   System   SHOULD   respond   with   a StartTransaction.conf
	//PDU.  This  response  PDU  MUST  include  a  transaction  id  and  an  authorization status value.The Central System MUST verify validity of the identifier in the
	//StartTransaction.req PDU, because the identifier  might  have  been  authorized  locally  by  the  Charge  Point  using  outdated  information.  The
	//identifier, for instance, may have been blocked since it was added to the Charge Point’s	Authorization Cache.
	struct IdTagInfo *info=verifyIdentifierValidity(wsi, idTag);

	printf("\nEL STATUS ES: %d", info->status);


	if (info->status==_CP_AUTHORIZATION_ACCEPTED)
	{
		struct chargePoint *cp=getChargePointFromWSI(wsi);

		if (cp)
		{
			addTransaction(cp, transaction, connector, meterStart);

			 //Pag 57: If  the  Central  System  receives  a StartTransaction.req the  Central  System  SHALL  respond  with  a
			//StartTransaction.conf.  If there is a need for a charging profile, The Central System MAY choose to send a
			//SetChargingProfile.req  to the Charge Point.

			 int chargingProfileId=ChargePointNeedsChargingProfile(); //<--NOT IMPLEMENTED. RETURNS ALWAYS 0
			 if (chargingProfileId>0)
			 {
				 send_setchargingprofile_request(getChargePointString(cp), connector, convert(transaction), convert(chargingProfileId));
			 }
		}
	}
	else
	{
		transaction=-1;
	}
//	printf("\nLA PROXIMA TRANSACCION SERA: %d", transaction);
	return prepareStartTransactionResponse(UniqueId, info, transaction);
}

//
// 4.10
//
char *respondStopTransactionRequest(const char *UniqueId, int meterStop, char *timestamp, int transactionId, char *reason, char *idTag, char *transactionData)
{
	//Primero eliminamos la transaccion en la tabla de transacciones
	removeTransaction(transactionId);

	//Pag 46: The Central System cannot prevent a transaction from stopping. It MAY only inform the	Charge Point it has received the StopTransaction.req
	// and MAY send information about the idTag used to stop the transaction. This information SHOULD be used to update the Authorization Cache
	//, if implemented.

	//update_authorization_Cahce();  <--NOT IMPLEMENTED

	//Pag 47: It is likely that The Central System applies sanity checks to the data contained in a StopTransaction.req	it  received.  The  outcome  of
	//such  sanity  checks  SHOULD  NOT  ever  cause  the  Central  System  to  not	respond  with  a StopTransaction.conf.  Failing  to  respond
	//with  a	StopTransaction.conf  will  only  cause	the  Charge  Point  to  try  the  same  message  again  as  specified  in	Error  responses
	//to  transaction-related messages.  ...NO SE QUE SANITY CHECKS...

	//apply_sanity_checks(); <--NOT IMPLEMENTED

	struct IdTagInfo *info=NULL;

	if (idTag)
	{
		info=(struct IdTagInfo *)calloc(1,sizeof(struct IdTagInfo));

		time_t now;
		time(&now);
		struct tm* now_tm;
		now_tm = localtime(&now);

		//Da un tiempo de una hora de reserva
		now_tm->tm_hour+=1;

		char *oneHourPlus=(char *)calloc(1,sizeof(char)*80);
		strftime (oneHourPlus, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
		oneHourPlus[strlen(oneHourPlus)]='\0';

		info->expiryDate=oneHourPlus;

		strncpy(info->parentIdtag.IdToken, idTag, 20);
		//info->parentIdtag=token;
		info->status=_CP_AUTHORIZATION_ACCEPTED;
	}

	return prepareStopTransactionResponse(UniqueId, info);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                       REQUESTS
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.1
//
void send_cancel_reservation_request(const char *chargepoint, const char *reservationID)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(CANCEL_RESERVATION, chargepoint);

	nuevo_mensaje->payload=prepareCancelReservationRequest(nuevo_mensaje->UniqueId, reservationID);

	Enqueue(nuevo_mensaje);
}

//
//5.2 CHANGE AVAILABILITY
//
void send_changeAvailability_request(const char *chargepoint, int connector, int new_availability)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(CHANGE_AVAILABILITY, chargepoint);

	nuevo_mensaje->payload=prepareChangeAvailabilityRequest(nuevo_mensaje->UniqueId, connector, new_availability);

	Enqueue(nuevo_mensaje);
}

//
// 5.3 CHANGE CONFIGURATION
//
void send_change_configuration_request(char *chargePoint, char *key, char *value)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(CHANGE_CONFIGURATION, chargePoint);

	nuevo_mensaje->payload=prepareChangeConfigurationRequest(nuevo_mensaje->UniqueId, key, value);

	Enqueue(nuevo_mensaje);
}

//
// 5.4 CLEAR CACHE
//

void send_clearcache_request(const char *chargePoint){
	queue_node *nuevo_mensaje=create_empty_queue_node(CLEAR_CACHE, chargePoint);

	nuevo_mensaje->payload=prepareClearCacheRequest(nuevo_mensaje->UniqueId);

	Enqueue(nuevo_mensaje);
}

//
// 5.5
//
void send_clearchargingprofile_request(const char *chargePoint, const char *chargingprofileid, int connectorid, int purpose, int stacklevel)
{
	char *connectorid_str=NULL;
	char *purpose_str=NULL;
	char *stacklevel_str=NULL;

	queue_node *nuevo_mensaje=create_empty_queue_node(CLEAR_CHARGING_PROFILE, chargePoint);

	//charging profile id
	if (strlen(chargingprofileid)<1) chargingprofileid=NULL;

	//conector id
	if (connectorid>=0) connectorid_str=convert(connectorid);

	//purpose
	if (purpose>=0){
		if (enums_as_integers)
		{
			purpose_str=convert(purpose);
		}
		else
		{
			purpose_str=ChargingProfilePurposeTypeTexts[purpose];
		}
	}

	//stack level
	if (stacklevel>=0) stacklevel_str=convert(stacklevel);

	nuevo_mensaje->payload=prepareClearChargingProfileRequest(nuevo_mensaje->UniqueId, chargingprofileid, connectorid_str, purpose_str, stacklevel_str);

	Enqueue(nuevo_mensaje);
}

//
// 5.6 DATA TRANSFER
//
void send_data_transfer_request(const char *chargePoint, char *messageId, char *data)
{
	queue_node *nuevo_mensaje;
	char** tokens = str_split(strdup(chargePoint), '_');
	char *vendor=*(tokens);
	char *vendorId;

	if (vendor)
	{
		//IMPLEMENTACION DEL VENDOR ID.
		vendorId=getVendorId(vendor);

		if (vendorId)
		{
			nuevo_mensaje=create_empty_queue_node(CS_DATA_TRANSFER, chargePoint);
		//	char *texto=prepareDataTransferRequest(nuevo_mensaje->UniqueId, vendorId, messageId, data);
		//	texto[strlen(texto)]='\0';
		//	nuevo_mensaje->payload=strdup(texto);
		//	free(texto);
			nuevo_mensaje->payload=prepareDataTransferRequest(nuevo_mensaje->UniqueId, vendorId, messageId, data);
			Enqueue(nuevo_mensaje);
		}
	}
}

//
// 5.8 GET CONFIGURATION
//
void send_getconfiguration_request(const char *chargePoint, char *key)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(GET_CONFIGURATION, chargePoint);

	nuevo_mensaje->payload=prepareGetConfigurationRequest(nuevo_mensaje->UniqueId, key);

	Enqueue(nuevo_mensaje);
}

//
// 5.9 GET DIAGNOSTICS
//
void send_getdiagnostics_request(const char *chargePoint, const char *location, const char *retries_str, const char *retryInterval_str, const char *startTime, const char *stopTime)
{
	int retries=-1;
	int retryInterval=-1;

	if (retries_str) retries=atoi(retries_str);
	if (retryInterval_str) retryInterval=atoi(retryInterval_str);

	queue_node *nuevo_mensaje=create_empty_queue_node(GET_DIAGNOSTICS, chargePoint);

	nuevo_mensaje->payload=prepareGetDiagnosticsRequest(nuevo_mensaje->UniqueId, location, retries, retryInterval, startTime, stopTime);

	Enqueue(nuevo_mensaje);

}

//
// 5.10 GET LOCAL LIST
//
void send_getlocallist_request(char *chargePoint)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(GET_LOCAL_LIST_VERSION, chargePoint);

	nuevo_mensaje->payload=prepareGetLocalListVersionRequest(nuevo_mensaje->UniqueId);

	Enqueue(nuevo_mensaje);
}

//
// 5.11 REMOTE START TRANSACTION
//
void send_remotestarttransaction_request(const char *chargePoint, const char *idTag, const char *connectorId_str, const char *stacklevel_str, const int purpose, const int kind, const int recurrency, const char *validfrom, const char *validto, int chargingProfile, const char *duration_str, const char *startSchedule, int chargingRateUnit, struct ChargingSchedulePeriod periodos[], int num_periods, const char *minChargingRate)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(REMOTE_START_TRANSACTION, chargePoint);
	int transaction=-1;

// Ver pag 90. TransactionId is only valid if ChargingProfilePurpose is set to TxProfile
	if ((chargingProfile) && (purpose==_CPPT_TXPROFILE))
	{
		//transactionId. Se trata de un numero que aumenta cada vez que se le envía una solciitud  al CP
		//storetransationInfo(transactionId) <-- NOT IMPLEMENTED. O bien en DB o bien en memoria

		transaction=transactionId++;

		//Pag 90: validFrom-->not to be used when ChargingProfilePurpose is TxProfile.
		//Pag 91: validTo-->not to be used when ChargingProfilePurpose is TxProfile.

		if (validfrom)
		{
			memset(validfrom, 0, strlen(validfrom));
			free(validfrom);
		}

		if (validto)
		{
			memset(validto, 0, strlen(validto));
			free(validto);
		}
	}

///CONNECTOR ID
	int connectorId;
	if (connectorId_str)
	{
		connectorId=atoi(connectorId_str);
	}
	else
	{
		connectorId=0;
	}

///STACK LEVEL
	int stackLevel=-1;
	if (stacklevel_str) stackLevel=atoi(stacklevel_str);

//DURATION
	int duration=-1;
	if (duration_str) duration=atoi(duration_str);

//minimum rate
	float minimumChargingRate=-1.0;
	if (minChargingRate)
	{
		if (sscanf(minChargingRate, "%f", &minimumChargingRate) != 1) minimumChargingRate=-1.0;
	}

	nuevo_mensaje->payload=prepareRemoteStartTransactionRequest(nuevo_mensaje->UniqueId, idTag, connectorId, transaction, stackLevel, purpose, kind, recurrency, validfrom, validto, chargingProfile, duration, startSchedule, chargingRateUnit, periodos, num_periods, minimumChargingRate);

	Enqueue(nuevo_mensaje);
}

//
// 5.12 REMOTE STOP TRANSACTION
//
void send_remotestoptransaction_request(const char *chargePoint, const char* transactionId)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(REMOTE_STOP_TRANSACTION, chargePoint);

	int trans=atoi(transactionId);

	nuevo_mensaje->payload=prepareRemoteStartTransactionStopRequest(nuevo_mensaje->UniqueId, trans);

	Enqueue(nuevo_mensaje);
}


//
// 5.13 SEND RESERVE NOW
//

void send_reservenow_request(char *chargePoint, int connector, char *idTag, char *reservationText, char *parentIdTag)
{
	char *cpForQueue=strdup(chargePoint);
	queue_node *nuevo_mensaje=create_empty_queue_node(RESERVE_NOW, cpForQueue);

	char** tokens = str_split(strdup(chargePoint), '_');
	char *vendor=NULL;
	char *model=NULL;
	char *IP=NULL;

	if (tokens)
	{
			vendor=*(tokens);
			model=*(tokens+1);
			IP=*(tokens+2);
	}

	int reservationId;

	if (reservationText)
	{
	   char *ptr;
	   reservationId= strtol(reservationText, &ptr, 10);
	}
	else
	{
		reservationId=getNextReservationId();
	}

	char *expiryDate=getExpiryTime(1,'h');

	addPendingReservation(vendor, model, IP, connector, reservationId, expiryDate, idTag);

	nuevo_mensaje->payload=prepareReserveNowRequest(nuevo_mensaje->UniqueId, connector, expiryDate, idTag, reservationId, parentIdTag);

	Enqueue(nuevo_mensaje);

	free(chargePoint);
}

//
// 5.14 Reset request
//
void send_reset_request(const char *chargePoint, int resetType)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(RESET_MSG, chargePoint);

	nuevo_mensaje->payload=prepareResetRequest(nuevo_mensaje->UniqueId, resetType);

	Enqueue(nuevo_mensaje);

	free(chargePoint);
}

//
// 5.15
void send_locallist_request(const char *chargePoint, const char *listName, int type)
{
	struct authorization_list_entry *list=NULL;

	//El numero de version por defecto es la ultima version actual.
	int version=listVersion;

	int i;
	if(listName)
	{
		 for (i=0; i<MAX_NUM_AUTH_LISTS; i++)
		 {
				if (authorization_lists[i])
				{
					if (strcmp(listName, authorization_lists[i]->name)==0)
					{
						version=authorization_lists[i]->listVersion;
						list=authorization_lists[i]->list;
						break;
					}
				}
		 }
	}

	struct chargePoint *cp=getChargePointFromString(chargePoint);
	setAuthListToChargePoint(cp, authorization_lists[i]);

	queue_node *nuevo_mensaje=create_empty_queue_node(SEND_LOCAL_LIST, chargePoint);

	nuevo_mensaje->payload=prepareSendLocalListRequest(nuevo_mensaje->UniqueId, version, list ,type);

	Enqueue(nuevo_mensaje);

	free(chargePoint);
}
//
//
// 5.16 Set charging profile
//
void send_setchargingprofile_request(const char *chargePoint, int connector, const char *transactionId, const char *chargingProfile)
{
	struct ChargingProfile *profile=chargingProfiles[atoi(chargingProfile)];
	//if (debug) show_chargingProfiles();

	//Si no encuentra un profile que mandarle, no manda nada.
	if (profile)
	{
		queue_node *nuevo_mensaje=create_empty_queue_node(SET_CHARGING_PROFILE, chargePoint);

		nuevo_mensaje->payload=prepareSetChargingProfileRequest(nuevo_mensaje->UniqueId, connector, chargingProfile, transactionId);

		Enqueue(nuevo_mensaje);

		free(chargePoint);
	}
}

//
// 5.17 Message Trigger
//
void send_messagetrigger_request(const char *chargePoint, int connector, int messageID)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(TRIGGER_MESSAGE, chargePoint);

	nuevo_mensaje->payload=prepareTriggerMessageRequest(nuevo_mensaje->UniqueId, connector, messageID);

	Enqueue(nuevo_mensaje);

	free(chargePoint);
}

//
//5.18 Unlock Connector
//

void send_unlockconnector_request(const char *chargePoint, int connector)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(UNLOCK_CONNECTOR, chargePoint);

	nuevo_mensaje->payload=prepareUnlockConnectorRequest(nuevo_mensaje->UniqueId, connector);

	Enqueue(nuevo_mensaje);

	free(chargePoint);

}

//
// 5.19 FIRMWARE UPDATE
//
void send_firmwareupdate_request(const char *chargePoint, const char *location, const char *retrieveDate, const char *retries_str, const char *retryInterval_str)
{
	queue_node *nuevo_mensaje=create_empty_queue_node(UPDATE_FIRMWARE, chargePoint);
	int retries=-1;
	int retryInterval=-1;
	if (retries_str) retries=atoi(retries_str);
	if (retryInterval_str) retries=atoi(retryInterval_str);

	nuevo_mensaje->payload=prepareFirmwareUpdateRequest(nuevo_mensaje->UniqueId, location, retrieveDate, retries, retryInterval);

	Enqueue(nuevo_mensaje);

	free(chargePoint);
}
