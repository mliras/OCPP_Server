/*
 * data_store.c
 *
 *  Created on: Dec 15, 2017
 *      Author: root
 */
#include "data_store.h"

int connectFiles()
{
	return 0;
}

int readConfigurationFromFile()
{
	return 0;

}

int readVendorIDsFromFile()
{
	return 0;
}

int readChargePointModelsFromFile()
{
	//Aqui cargamos los Charge Points permitidos por este servidor. Inicialmente desde un fichero.
		// EN UN FUTURO TAMBIEN DESDE MYSQL <-- NO IMPLEMENTADO
			char *filename="/etc/ocpp/chargepointmodels.dat";   //<-- Cambiar a configuration key , NOT IMPLEMENTED
			struct chargePointModel *object2=calloc(1, sizeof(struct chargePointModel));
			FILE * file= fopen(filename, "rb");
			if (file != NULL)
			{
				int i=0;
				while (fread(object2, sizeof(struct chargePointModel), 1, file) == 1)
				{
				    strncpy(allowedChargePointModels[i].chargePointVendor.x,object2->chargePointVendor.x, sizeof(allowedChargePointModels[i].chargePointVendor.x));
				    strncpy(allowedChargePointModels[i].chargePointModel.x,object2->chargePointModel.x, sizeof(allowedChargePointModels[i].chargePointModel.x));
				    allowedChargePointModels[i].chargePointModel.x[sizeof(allowedChargePointModels[i].chargePointModel.x)-1]='\0';
				    allowedChargePointModels[i].chargePointVendor.x[sizeof(allowedChargePointModels[i].chargePointVendor.x)-1]='\0';

		 		    allowedChargePointModels[i].numConnectors=object2->numConnectors;
		 		    i++;
				}

				if (feof(file))
				{
					printf("Fin de fichero encontrado");
				}

				fclose(file);
				return 0;
			}

			return -1;
}

//Connects to the database indicated by params server, user and password which are read from the configuration file in example_server
void connectDB(char *server, char *user, char *pass)
{
	char *database="OCPP";
	conn=mysql_init(NULL);

	if (!mysql_real_connect(conn, server, user, pass, database, 0, NULL, 0))
	{
	          fprintf(stderr, "%s\n", mysql_error(conn));
	          if (debug) printf("Database connection error");
	          conn=NULL;
	          return;
	}

	data_store_type=2;
}


//////////////////////////////////////////////////////////
//          MODELS AND VENDORS
//////////////////////////////////////////////////////////

void readChargePointModelsFromDB()
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *vendor, *model, *numConnectors;

	if (!conn) return;

	char *query="SELECT * from ChargePointModels";
	if (mysql_query(conn, query)) {
		 fprintf(stderr, "%s\n", mysql_error(conn));
		 printf("La siguiente query ha fallado: %s", query);
		 exit(4);
		 	          //return NULL;
	  }

	res = mysql_use_result(conn);

	int i=0;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
    	//
    	vendor=row[1];
    	model=row[2];
    	numConnectors=row[3];
    	//printf("\n%s %s %s", row[1], row[2], row[3]);
    	strncpy(allowedChargePointModels[i].chargePointVendor.x,vendor, sizeof(allowedChargePointModels[i].chargePointVendor.x));
    	strncpy(allowedChargePointModels[i].chargePointModel.x,model, sizeof(allowedChargePointModels[i].chargePointModel.x));
    	allowedChargePointModels[i].chargePointModel.x[sizeof(allowedChargePointModels[i].chargePointModel.x)-1]='\0';
    	allowedChargePointModels[i].chargePointVendor.x[sizeof(allowedChargePointModels[i].chargePointVendor.x)-1]='\0';

    	allowedChargePointModels[i].numConnectors=atoi(numConnectors);
    	i++;
    }

    mysql_free_result(res);

}

void readVendorIDsFromDB()
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	if (mysql_query(conn, "SELECT distinct A.Vendor, B.VendorId from ChargePointModels A, VendorIDs B where A.VendorId=B.Id"))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		printf("La siguiente query ha fallado9: ");
	    exit(1);
	}

	res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
    {
		//Meterlo en memoria <--NOT IMPLEMENTED
    	setVendorId(row[0], row[1]);
    }

    mysql_free_result(res);
}

void writeChargePointModelinDB(MYSQL *conn, char *Vendor, char *Model, int numConnectors, char *meterType)
{
	if ((!conn) || (!Vendor) || (!Model)) return;

	char *query=(char *)calloc(200,1);

	if (meterType)
	{
		sprintf(query, "INSERT INTO ChargePointModels VALUES(0, \"%s\", \"%s\", %d, \"%s\"", Vendor, Model, numConnectors, meterType);
	}
	else
	{
		sprintf(query, "INSERT INTO ChargePointModels VALUES(0, \"%s\", \"%s\", %d, \"UNKNOWN\"", Vendor, Model, numConnectors);
	}

	if (mysql_query(conn, query))
	{
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    printf("La siguiente query ha fallado8: %s", query);
	    exit(1);
	}

	//res = mysql_use_result(connection);

    //while ((row = mysql_fetch_row(res)) != NULL)
    //{
		//Meterlo en memoria <--NOT IMPLEMENTED

    //}

    if (query) free(query);
    //mysql_free_result(res);
}


int getModelIdFromDatabase(char *Model, char *Vendor)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	if ((!conn)) return -2;
	char *query=(char *)calloc(200,1);

	sprintf(query, "select id from ChargePointModels where Model='%s' and Vendor='%s'", Model, Vendor);

	if (mysql_query(conn, query))
	{
			    fprintf(stderr, "%s\n", mysql_error(conn));
			    if (debug) printf("LA CONEXION DEVUELVE ERROR 2");
			    free(query);
			    mysql_store_result(conn);
			    return -1;
	}

	if (query) free(query);
	res = mysql_use_result(conn);

	if (res)
	{
		//printf("RES NO ES NULL");
		row = mysql_fetch_row(res);

		if (row)
		{
			mysql_free_result(res);
			return atoi(row[0]);
		}
	}

	mysql_free_result(res);
	return -1;
}

//////////////////////////////////////////////////////////////////////
//              AUTH LISTS
//////////////////////////////////////////////////////////////////////

//returns 1 if valid
//returns 0 if not valid
int isValidAuthlistVersion(int version)
{
	MYSQL_RES *res;

	if ((!conn)) return 0;

	char *query=(char *)calloc(200,1);

	//Obtenemos el id del chargepoint en la BD a partir de la informacion indicada

	sprintf(query, "SELECT Version from AuthLists where Version=%d", version);

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));

		if (debug) printf("\nLA QUERY HA FALLADO::: %s", mysql_error(conn));
		return 0;
	}

	res = mysql_use_result(conn);
	int num_rows = mysql_num_rows(res);
	mysql_free_result(res);

	if (num_rows>0)
	{
		return 1;
	}
	else return 0;
}

void updateListVersionInDb(struct chargePoint *cp, int version)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);

	if (!isValidAuthlistVersion(version))
	{
		Log("Authentication List not found in DB", cp, 99998, AL_ERROR);
		return;
	}

	//Obtenemos el id del chargepoint en la BD a partir de la informacion indicada
	int chargepoint_id=getChargepointIdFromDatabase2(cp->IP, cp->SN, cp->Model, cp->Vendor);

	if (chargepoint_id<0)
	{
		//No encontró el chargepoint en DB. No podemos continuar.
		Log("No se encontro el chargepoint en la base de datos", cp, 99999, AL_ERROR);
		return;
	}
	else
	{
			sprintf(query, "UPDATE ChargePoints set authlist=%d where id=%d", version, chargepoint_id);
			strcat(query, ", '%Y-%m-%dT%h:%i:%s.'), NULL)");

			if (mysql_query(conn, query))
			{
				fprintf(stderr, "%s\n", mysql_error(conn));

				if (debug) printf("\nLA QUERY HA FALLADO::: %s", mysql_error(conn));
				return;
			}
		}

		if (query) free(query);
}

void insertEntryInListInDb(struct authorization_list_entry *entry, int list_id)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);

	char *expirytime=(char *)calloc(1, sizeof(char)*80);
	strftime (expirytime, 80, "%Y-%m-%dT%H:%M:%S.", entry->entry->expiryDate);
	expirytime[strlen(expirytime)]='\0';

	sprintf(query, "INSERT INTO AuthListEntry VALUES (0, STR_TO_DATE('%s','%%Y-%%m-%%dT%%h:%%i:%%s.'), '%s', '%s',%d,%d)",
			expirytime,entry->entry->parentIdTag, entry->idTag, entry->entry->status, list_id);

	free(expirytime);

	////////
	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));

		if (debug) printf("\nLA QUERY %s HA FALLADO: %s", query, mysql_error(conn));
		return;
	}
}

void insertAuthorizationListInDb(struct authorization_list *list)
{
	if ((!conn)) return;

	MYSQL_RES *res;
	MYSQL_ROW row;

	char *query=(char *)calloc(200,1);

	//El nombre de la lista tiene como maximo 32 caracteres
	sprintf(query, "INSERT INTO AuthLists values(1, '%s',0)", list->name);

	////////
	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));

		if (debug) printf("\nLA QUERY %s HA FALLADO CON ERROR %s", query, mysql_error(conn));
		return;
	}

	if (query) memset(query,0,200);
	/////////
	sprintf(query, "SELECT ID FROM AuthLists WHERE name='%s'", list->name);

	if (mysql_query(conn, query))
	{
			fprintf(stderr, "%s\n", mysql_error(conn));

			//if (debug) printf("\nLA SEGUNDA QUERY HA FALLADO::: %s", mysql_error(conn));
			return;
	}

	res = mysql_use_result(conn);
	int id=-1;

	if (res)
	{
		row = mysql_fetch_row(res);

		if (row)
		{
			id=atoi(row[0]);
		}

		mysql_free_result(res);
	}

	struct authorization_list_entry *l=list->list;

	if (id>=0)
	{
		while (l)
		{
			insertEntryInListInDb(l, id);
			l=l->next;
		}
	}
}

//Esta funcion lee los Charging Profiles de la BD
void readChargingProfilesFromDB()
{
	if ((!conn)) return;

	MYSQL_RES *res;
	MYSQL_ROW row;

	char *query=(char *)calloc(200,1);
	sprintf(query, "SELECT A.*, B.* FROM `ChargingProfile` A, `ChargingSchedulePeriods` B where B.chargingProfileId=A.id");

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		printf("\nLA QUERY: %s HA FALLADO::: %s", query, mysql_error(conn));
		return;
	}

	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);

	int ii=0;
	while (ii <= res->row_count)
	{
		if (row)
		{
			struct ChargingProfile *l=(struct ChargingProfile *)calloc(1,sizeof(struct ChargingProfile));
			struct ChargingSchedule *cs=(struct ChargingSchedule *)calloc(1,sizeof(struct ChargingSchedule));
			struct ChargingSchedulePeriod *csp=(struct ChargingSchedulePeriod *)calloc(1,sizeof(struct ChargingSchedulePeriod));

			if (row[0]) l->chargingProfileId=atoi(row[0]);
			if (row[1]) {
				int i=atoi(row[1]);
				l->transactionId=&i;
			}

			if (row[2]) l->stackLevel=atoi(row[2]);
			if (row[3]) l->chargingProfilePurpose=atoi(row[3]);
			if (row[4]) l->chargingProfileKind=atoi(row[4]);
			if (row[5])
			{
				int i=atoi(row[5]);
				l->recurrencyKind=&i;
			}

			//if (row[6] l->validFrom
			//if (row[7] l->validTo
			if (row[8])
			{
				int i=atoi(row[8]);
				cs->duration=&i;
			}

			//if (row[9] l->chargingSchedule->startSchedule=
			if (row[10]) cs->chargingRateUnit=atoi(row[10]);
			if (row[11])
			{
				double d=atof(row[11]);
				cs->minChargingRate=&d;
			}

			//l->chargingSchedule->chargingSchedulePeriods
			if (row[13]) csp->startPeriod=atoi(row[13]);
			if (row[14]) csp->limit=atof(row[14]);
			if (row[15]) csp->numPhases=atoi(row[15]);

			cs->chargingSchedulePeriods=*csp;
			l->chargingSchedule=*cs;
			chargingProfiles[ii]=l;
		}

		row = mysql_fetch_row(res);
		ii++;
	}

	//show_chargingProfiles();

	if (res) mysql_free_result(res);
}

//Esta funcion lee las listas de autorizacion de la BD MySQL
void readAuthorizationListsFromDB()
{
	if ((!conn)) return;

	MYSQL_RES *res;
	MYSQL_ROW row;

	struct AuthList *al;
	struct authorization_list *l;

	char *query=(char *)calloc(200,1);
	sprintf(query, "SELECT name, version, id FROM AuthLists");

	if (mysql_query(conn, query))
	{
				fprintf(stderr, "%s\n", mysql_error(conn));

				if (debug) printf("\nLA PRIMERA QUERY: %s HA FALLADO CON ERROR: %s", query, mysql_error(conn));
				return;
	}

	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);

	//Recorremos la lista y creamos las listas vacias.
	int ii=0;

	while (ii < res->row_count)
	{
		if (row)
		{
			struct authorization_list *l=(struct authorization_list *)calloc(1,sizeof(struct authorization_list *));

			l->name=strdup(row[0]);
			l->listVersion=atoi(row[1]);
			l->list=NULL;
			authorization_lists[ii]=l;

			//if (debug) printf("\nauthorization_lists de %d se almacena en %x", ii, authorization_lists[ii]);

		}
		row = mysql_fetch_row(res);
		ii++;
	}

	mysql_free_result(res);


	sprintf(query, "SELECT * FROM AuthLists A, AuthListEntry B WHERE A.Id=B.AuthListVersion");

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));

		if (debug) printf("\nLA SIGUIENTE QUERY %s HA FALLADO CON ERROR: %s", query,mysql_error(conn));
		return;
	}

	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);

	//Recorremos la lista y creamos las listas vacias.
	int i=0;
	while (i < res->row_count)
	{
		if (row)
		{
			struct authorization_list_entry *entry=(struct authorization_list_entry *)calloc(1,sizeof(struct authorization_list_entry));
			if (row[6]) entry->idTag=strdup(row[6]);
			entry->next=NULL;
			struct authorization_record *data=(struct authorization_record *)calloc(1,sizeof(struct authorization_record));

			if (row[4])
			{
				//printf("\nAHORA ALMACENAMOS LA FECHA DE EXPIRACION");
				struct tm* expiryTime=(struct tm *)calloc(1, sizeof(struct tm));
				strptime(row[4], "%Y-%m-%dT%H:%M:%S.", expiryTime);
				data->expiryDate=expiryTime;
			}

			if (row[5]) strcpy(data->parentIdTag,strdup(row[5]));

			if (row[7]) data->status=atoi(row[7]);

			entry->entry=data;
			addAuthorizationToList(authorization_lists[atoi(row[8])-1], entry);
		}
		row = mysql_fetch_row(res);
		i++;
	}
	mysql_free_result(res);

//	if (debug) showAuthorizationLists();
}

//////////////////////////////////////////////////////////////////////
//              CONNECTIONS
//////////////////////////////////////////////////////////////////////


void insertConnectionIntoDB(const char *chargePointModel, const char *chargePointVendor, const char *chargePointSerialNumber, char *ip, char *chargeBoxSerialNumber, char *firmwareVersion, char *iccid, char *imsi, char *meterSerialNumber, char *meterType, char *currentTime)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);

	//Obtenemos el id del chargepoint en la BD a partir de la informacion indicada
	int chargepoint_id=getChargepointIdFromDatabase2(ip, chargePointSerialNumber, chargePointModel, chargePointVendor);

	if (chargepoint_id<0)
	{
		//No encontró el chargepoint en DB. Lo insertamos.
		insertChargePointIntoDatabase2(ip, chargePointSerialNumber, chargePointModel, chargePointVendor, chargeBoxSerialNumber, firmwareVersion, iccid, imsi, meterSerialNumber, NULL);
		chargepoint_id=getChargepointIdFromDatabase2(ip, chargePointSerialNumber, chargePointModel, chargePointVendor);
	}

	mysql_store_result(conn);

	if (chargepoint_id>=0)
	{
		sprintf(query, "insert into Connections values(0, %d, STR_TO_DATE('%s'", chargepoint_id, currentTime);
		strcat(query, ", '%Y-%m-%dT%h:%i:%s.'), NULL)");

		if (mysql_query(conn, query))
		{
			fprintf(stderr, "%s\n", mysql_error(conn));

			if (debug) printf("\nLA QUERY HA FALLADO::: %s", mysql_error(conn));
			return;
		}


	}

	if (query) free(query);
}

//////////////////////////////////////////////////
//         METER VALUES
//////////////////////////////////////////////////

void insertMeterValueIntoDB(int connectorId, int transactionId, const char *timeStamp, char *value, int context, int format,int measurand,int phase, int location, int unit)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);

	if (transactionId<0)
	{
		sprintf(query, "INSERT INTO meterValues VALUES(0,%d,NULL,STR_TO_DATE('%s', '%%Y-%%m-%%dT%%H:%%i:%%s.'),%s, %d, %d,%d,%d,%d,%d)",
				connectorId,timeStamp,value,context, format,measurand,phase, location, unit);
	}
	else
	{
		sprintf(query, "INSERT INTO meterValues VALUES(0,%d,%d,STR_TO_DATE('%s', '%%Y-%%m-%%dT%%H:%%i:%%s.'),%s, %d, %d,%d,%d,%d,%d)", connectorId,transactionId, timeStamp,value,  context, format,measurand,phase, location, unit);
	}

	//if (debug) printf("Se lanza la query: %s y contect es %d", query, context);

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));

		printf("\nLA QUERY %s HA FALLADO::: %s", query, mysql_error(conn));
		return;
	}

}
//////////////////////////////////////////////////
//         IDTAGS
//////////////////////////////////////////////////

char *getParentIdTagFromDatabase(char *idtag)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	if ((!conn)) return NULL;

	char *query=(char *)calloc(200,1);

	sprintf(query, "select parentIdTag from Users where Idtag='%s'", idtag);

	if (mysql_query(conn, query))
	{

	    fprintf(stderr, "%s\n", mysql_error(conn));
	    free(query);
	    return NULL;
	}

	free(query);


	char *str=NULL;

	res = mysql_use_result(conn);
	if (res)
	{
		row = mysql_fetch_row(res);
		str=strdup(row[0]);
	}

	mysql_free_result(res);

	//printf("row[0] es %s", str);

	return str;
}

char *getIdTagsFromDatabase()
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	if ((!conn)) return NULL;

	char *query=(char *)calloc(200,1);

	sprintf(query, "select IdTag from Users");

	if (mysql_query(conn, query))
	{
		printf("En la query: %s", query);
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    free(query);
	    return NULL;
	}

	free(query);

	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);
	char *resultado=(char *)calloc(2048,1);
	strncpy(resultado, row[0], 20);

	while ((row = mysql_fetch_row(res)))
	{
		strcat(resultado, ",");
		strncat(resultado, row[0], 20);
	}

	mysql_free_result(res);
	return resultado;
}


//////////////////////////////////////////////////////////////////////
//              CHARGEPOINTS
//////////////////////////////////////////////////////////////////////


////
//Busca el ID del chargepoint en la BD a partir de los datos almacenados en la struct chargepoint
//Devuelve -2 si no hay una conexion a BD activa
//Devuelve -1 si no encontró ningun chargepoint que cuadre en la BD
//Devuelve el ID del chargepoint si lo encuentra.
int getChargepointIdFromDatabase(struct chargePoint *cp)
{
		MYSQL_RES *res;
		MYSQL_ROW row;

		if ((!conn)) return -2;

		char *query=(char *)calloc(200,1);

		if (cp->SN)
				sprintf(query, "select id from ChargePoints where SerialNumber='%s'", cp->SN);
		else
		{
			int model_id=getModelIdFromDatabase(cp->Model, cp->Vendor);

			sprintf(query, "select id from ChargePoints where IP='%s' and ModelId=%d", cp->IP, model_id);
		}

	//	printf("LA QUERY ENIADA ES: %s", query);
		if (mysql_query(conn, query))
		{
		    fprintf(stderr, "%s\n", mysql_error(conn));
		    printf("\nLa siguiente query ha fallado7: %s", query);
		    mysql_store_result(conn);
		    free(query);
		    return -1;
		}

		res = mysql_use_result(conn);
		row = mysql_fetch_row(res);

		if (query) free(query);

		if (row)
		{
			//printf("EL ID LEIDO ES... %s", row[0]);
			//printf("EL ID LEIDO ES... %s", row[0]);
			int mirow=atoi(row[0]);
			mysql_free_result(res);
			return mirow;
		}
		else return -1;
}

int getChargepointIdFromDatabase2(char *IP, char *SN, char *Model, char *Vendor)
{
		MYSQL_RES *res;
		MYSQL_ROW row;

		if ((!conn)) return -2;

		char *query=(char *)calloc(200,1);

		if (SN)
			sprintf(query, "select id from ChargePoints where SerialNumber='%s'", SN);
		else
		{
			int model_id=getModelIdFromDatabase(Model, Vendor);

			sprintf(query, "select id from ChargePoints where IP='%s' and ModelId=%d", IP, model_id);
		}


//		This can happen, for example, if you are using mysql_use_result() and try to execute a new query before you have called mysql_free_result().
//		It can also happen if you try to execute two queries that return data without calling mysql_use_result() or mysql_store_result() in between.

		if (mysql_query(conn, query))
		{
		    fprintf(stderr, "%s\n", mysql_error(conn));
		    printf("La siguiente query ha fallado6: %s", query);
		    free(query);
		    return -1;
		}

		free(query);

		res = mysql_use_result(conn);
		if (res)
		{
			row = mysql_fetch_row(res);

			mysql_free_result(res);
			if (row)
			{
				//printf("EL ID LEIDO ES... %s", row[0]);
				return atoi(row[0]);
			}
			else return -1;
		}
		else
		{
			mysql_free_result(res);
			return -1;
		}
}



void insertChargePointIntoDatabase(struct chargePoint *cp, char *CB_SN, char *FW_Version, char *ICCID, char *IMSI, char *Meter_SN, char *Description)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);
	int model_id=getModelIdFromDatabase(cp->Model, cp->Vendor);
	sprintf(query, "insert into ChargePoints values (0, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',-1)", model_id, CB_SN, cp->SN, FW_Version, ICCID, IMSI, Meter_SN,Description, cp->IP);

	if (mysql_query(conn, query))
	{
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    printf("La siguiente query ha fallado5: %s", query);
	    //exit(1);
	}

	mysql_store_result(conn);
    free(query);
    //mysql_free_result(res);

}

void insertChargePointIntoDatabase2(char *IP, char *SN, char *Model, char *Vendor, char *CB_SN, char *FW_Version, char *ICCID, char *IMSI, char *Meter_SN, char *Description)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);
	int model_id=getModelIdFromDatabase(Model, Vendor);
	sprintf(query, "insert into ChargePoints values (0, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',-1)", model_id, CB_SN, SN, FW_Version, ICCID, IMSI, Meter_SN,Description, IP);

	if (mysql_query(conn, query))
	{
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    printf("La siguiente query ha fallado4: %s", query);
	}

	mysql_store_result(conn);
    free(query);
}

//////////////////////////////////////////////////////////////////////
//              TRANSACTIONS
//////////////////////////////////////////////////////////////////////


void insertTransactionIntoDb(struct chargePoint *cp,int transactionId, int connector, double meterStart, char *currentTime)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);
	int cp_id=getChargepointIdFromDatabase2(cp->IP, cp->Model, cp->SN, cp->Vendor);
	sprintf(query, "insert into Transactions values (%d, NULL, %d, %f, 0, STR_TO_DATE('%s','%%Y-%%m-%%dT%%H:%%i:%%s.'), NULL)", transactionId, cp_id, meterStart, currentTime);

	printf("\nSE LANZA LA QUERY %s", query);
	if (mysql_query(conn, query))
	{
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    printf("La siguiente query ha fallado3: %s", query);
	}

	mysql_store_result(conn);
    if (query) free(query);
}

//Esta funcion elimina una transaccion de la BD, pero realmente nosotros no queremos esto, sino
//actualizar la fecha de parada.
void removeTransactionFromDb(int transactionId)
{
		if ((!conn)) return;

		char *query=(char *)calloc(200,1);

		sprintf(query, "delete from Transactions where transactionId='%d'", transactionId);

		if (mysql_query(conn, query))
		{
		    fprintf(stderr, "%s\n", mysql_error(conn));
		    printf("\nLa siguiente query ha fallado: %s", query);
		}

		mysql_store_result(conn);
		if (query) free(query);
}

void updateTransactionStopTimeOnDb(int transactionId)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);
	char *currentTime=getCurrentTime();

	sprintf(query, "update Transactions set Stop_Time=STR_TO_DATE('%s', '%%Y-%%m-%%dT%%h:%%i:%%s.') where transactionId='%d'", currentTime, transactionId);

	if (mysql_query(conn, query))
	{
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    printf("\nLa siguiente query ha fallado: %s", query);
	}

	mysql_store_result(conn);
	if (query) free(query);
}


int getLastTransactionId()
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	if ((!conn)) return -2;

	char *query=(char *)calloc(200,1);
	sprintf(query, "SELECT MAX(Transactionid) from Transactions");

	//printf("Se lanza la query %s", query);

	if (mysql_query(conn, query))
	{
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    printf("La siguiente query ha fallado: %s", query);
	    free(query);
	    return -1;
	}

	free(query);
	res= mysql_store_result(conn);
	//res = mysql_use_result(conn);
	if (res)
	{
	//	printf("TIENE %d CAMPOS:", mysql_num_fields(res));
		row = mysql_fetch_row(res);

		mysql_free_result(res);
		if (row)
		{
		//	printf("EL ID LEIDO ES... %s", row[0]);
			//printf("EL ID LEIDO ES... %s", row[1]);
			return atoi(row[0]);
		}
		else return -1;
	}
	else
	{
		mysql_free_result(res);
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
//              RESERVATIONS
//////////////////////////////////////////////////////////////////////


void insertReservationIntoDatabase(char *vendor, char *model, char *IP, int Connector, int id, char *expiryDate, char *idTag)
{
	if ((!conn)) return;

	int RESERVATION_STATUS_PENDING=-1;
	int cp_id=-1;
	cp_id=getChargepointIdFromDatabase2(IP, model, NULL, vendor);

	char *query=(char *)calloc(200,1);
	if (cp_id==-1)
	{
		sprintf(query, "INSERT INTO Reservations VALUES (%d, STR_TO_DATE('%s', '%%Y-%%m-%%dT%%H:%%i:%%s.'), %d, True, '%s', NULL, %d)", id, expiryDate, Connector, idTag,  RESERVATION_STATUS_PENDING);
	}
	else
	{
		sprintf(query, "INSERT INTO Reservations VALUES (%d, STR_TO_DATE('%s', '%%Y-%%m-%%dT%%H:%%i:%%s.'), %d, True, '%s', %d, %d)", id, expiryDate, Connector, idTag, cp_id, RESERVATION_STATUS_PENDING);
	}

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		printf("La siguiente query ha fallado: %s", query);
	}

	mysql_store_result(conn);
	if (query) free(query);
}

void updateReservationInDB(int reservation_id, int status)
{
	if ((!conn)) return;

	char *query=(char *)calloc(200,1);
	sprintf(query, "UPDATE Reservations SET status=%d WHERE Id=%d", status, reservation_id);

	if (mysql_query(conn, query))
	{
			fprintf(stderr, "%s\n", mysql_error(conn));
			printf("La siguiente query ha fallado: %s", query);
	}

	mysql_store_result(conn);
	if (query) free(query);
}




void expireReservationIdFromDb(int reservation_id)
{
	if ((!conn)) return;

	char *query=(char *)calloc(255,1);

	sprintf(query, "UPDATE Reservations SET Active=0 where id=%d", reservation_id);

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		printf("\nLa siguiente query ha fallado: %s", query);
	}

	mysql_store_result(conn);
	if (query) free(query);
}


int getLastReservationIdFromDb()
{
	if ((!conn)) return -1;
	MYSQL_RES *res;
	MYSQL_ROW row;

	if (mysql_query(conn, "SELECT id From Reservations order by 1 desc limit 1 "))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		printf("\nLa siguiente query ha fallado: %s", "SELECT id From Reservations order by 1 desc limit 1 ");
	}

	res= mysql_store_result(conn);
	if (res)
	{
		row = mysql_fetch_row(res);

		mysql_free_result(res);
		if (row)
		{
			return atoi(row[0]);
		}
		else return -1;
	}
	else
	{
		mysql_free_result(res);
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
//              CONFIGURATION KEYS
//////////////////////////////////////////////////////////////////////

void readConfigurationKeysfromDB(MYSQL *conn)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *key, *value;

	if ((!conn)) return;

	char *query=(char *)calloc(200,1);

	sprintf(query, "select * from ConfigurationKeys;");


	if (mysql_query(conn, query))
	{
	    fprintf(stderr, "%s\n", mysql_error(conn));
	    printf("Error al leer las configuration keys");
	    //exit(1);
	}

	res = mysql_use_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL)
    {
    	key=row[0];
    	value=row[1];

    	modifyConfigurationKey(key,strndup(value, strlen(value)-1));
    }

    if (query) free(query);
    mysql_free_result(res);
}

void insertValidKeysInDb(struct chargePoint *cp, KeyVal validkeys[CONFIGURATION_KEY_LIST_SIZE])
{
	//printf("ENTRA1");
	if ((!conn)) return;

	//printf("ENTRA2");
	char *query=(char *)calloc(2000,1);

	int cp_id=-1;
	cp_id=getChargepointIdFromDatabase(cp);

	//Obtenemos la fecha actual
	//////////
	time_t now;
	time(&now);
	struct tm* now_tm;
	now_tm = localtime(&now);
	char *currentTime=(char *)calloc(80, sizeof(char));
	strftime (currentTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
	//////////

	for (int i=0; i<CONFIGURATION_KEY_LIST_SIZE; i++)
	{
		if (validkeys[i].readonly>=0)
		{
			//Entrada inicializada

			int type=getConfigurationKeyType(validkeys[i].key.x);


			sprintf(query, "INSERT INTO ConfigurationKeys VALUES(0, '%s', %d, NULL, '%s', %d, %d, STR_TO_DATE('%s', '%%Y-%%m-%%dT%%h:%%i:%%s.'), %d)", validkeys[i].key.x, type, validkeys[i].value.x, cp_id, validkeys[i].readonly,currentTime,1);

			//strcpy(format, "INSERT INTO ConfigurationKeys VALUES(0, '%s', %d, NULL, '%s', %d, %d, STR_TO_DATE('%s', ");
			//strcat(format, "'%Y-%m-%dT%h:%i:%s.'), %d)");
			//sprintf(query, format, validkeys[i].key.x, type, validkeys[i].value.x, cp_id, validkeys[i].readonly,currentTime,1);


			if (mysql_query(conn, query))
			{
				fprintf(stderr, "%s\n", mysql_error(conn));
				printf("\nLa siguiente query ha fallado: %s", query);
			}
		}

		memset(query, '\0', 2000);
	}

	mysql_store_result(conn);
	if (query) free(query);
}

//////////////////////////////////////////////////
//	LOGGING
//////////////////////////////////////////////////

void LogInDb(struct chargePoint *cp, int alarm_code, int alarm_level, char *texto)
{
	if ((!conn)) return;

	char *query=(char *)calloc(2000,1);

	int alck=-1;
	alck=getConfigurationKeyIntValue("LogAlarmLevel");

	//if (debug) printf("ALCK ES %d", alck);

	//Solo logamos aquello que este permitido segun una configuration key.
	if (alck<=alarm_level)
	{
		//Obtenemos el chargepoint, si no lo encuentra, cp_id se queda con -1
		int cp_id=-1;
		if (cp)
		{
			cp_id=getChargepointIdFromDatabase2(cp->IP, cp->Model, cp->SN, cp->Vendor);
		}

		//Obtenemos la fecha actual
		//////////
		time_t now;
		time(&now);
		struct tm* now_tm;
		now_tm = localtime(&now);
		char *currentTime=(char *)calloc(80, sizeof(char));
		strftime (currentTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
		//////////

		if (cp_id==-1)
		{
			sprintf(query, "INSERT INTO Alarms VALUES(0, NULL, %d, %d, '%s', STR_TO_DATE('%s', '%%Y-%%m-%%dT%%h:%%i:%%s.'))", alarm_code, alarm_level, texto, currentTime);
		}
		else
		{
			sprintf(query, "INSERT INTO Alarms VALUES(0, %d, %d, %d, '%s', STR_TO_DATE('%s', '%%Y-%%m-%%dT%%h:%%i:%%s.'))", cp_id, alarm_code, alarm_level, texto, currentTime);
		}
	//	if (debug) printf("La query es %s", query);

		if (mysql_query(conn, query))
		{
			fprintf(stderr, "%s\n", mysql_error(conn));
			printf("\nLa siguiente query ha fallado: %s", query);
		}
	}

	mysql_store_result(conn);
	if (query) free(query);
}

void LogInDb2(char *model, char *vendor, char *IP, char *SN, int alarm_code, int alarm_level, char *texto)
{
	if ((!conn)) return;

	char *query=(char *)calloc(2000,1);

	int alck=-1;
	alck=getConfigurationKeyIntValue("LogAlarmLevel");

	//Solo logamos aquello que este permitido segun una configuration key.
	if (alck<=alarm_level)
	{
		int cp_id=-1;
		cp_id=getChargepointIdFromDatabase2(IP, model, SN, vendor);

		//Obtenemos la fecha actual
		//////////
		time_t now;
		time(&now);
		struct tm* now_tm;
		now_tm = localtime(&now);
		char *currentTime=(char *)calloc(80, sizeof(char));
		strftime (currentTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
		//////////

		if (cp_id==-1)
		{
			sprintf(query, "INSERT INTO Alarms (`id`, `ChargePointId`, `Alarm_Code`, `Alarm_Level`, `Description`, `TimeStamp`) VALUES(0, NULL, %d, %d, '%s', STR_TO_DATE('%s', '%%Y-%%m-%%dT%%h:%%i:%%s.'))", alarm_code, alarm_level, texto, currentTime);
		}
		else
		{
			sprintf(query, "INSERT INTO Alarms (`id`, `ChargePointId`, `Alarm_Code`, `Alarm_Level`, `Description`, `TimeStamp`) VALUES(0, %d, %d, %d, '%s', STR_TO_DATE('%s', '%%Y-%%m-%%dT%%h:%%i:%%s.'))", cp_id, alarm_code, alarm_level, texto, currentTime);
		}

		if (mysql_query(conn, query))
		{
			fprintf(stderr, "%s\n", mysql_error(conn));
			printf("\nLa siguiente query ha fallado: %s", query);
		}
	}

	mysql_store_result(conn);
	if (query) free(query);
}


