
#include "ocpp_server.h"
//#include "server.h"


//
//  AUXILIARY FUNCTIONS
//

//
// Esta funcion implementa el parrafo de la pagina 36 que indica que un messageId puede ser usado para indicar un tipo especial de implementacion
// Como no explica nada mas, dejo la linea comentada donde se manejaría este messageId, sea lo que sea.
// Los message Id estan en el fichero .INI y se leen en la variable de configuracion messageUdList
// La funcion busca la cadena recibida en el mensaje, dentro de ese listado.
//
int checkMessageId(const char *messageId)
{
	return containsCSL ("messageIdList", messageId);
}

//
// Esta funcion implementa el parrafo de la pagina 36 que indica que un vendorId debe ser conocido por
//el CS. Los vendor Id estan en la BD de datos.
// La funcion carga los vendor desde la BD, busca la cadena recibida en el mensaje, dentro de ese listado.
//
int checkWellKnownVendorId(const char *vendorId)
{
	//Lo leemos de la BD
	readVendorIDsFromDB();

	//Si no lo encuentra devuelve NULL (es decir 0)
	return getVendorId(vendorId);
	//return containsCSL ("vendorIdList", vendorId);
}

int checkDataTransferShouldbeRejected(json_object * Payload){
		return 0;
}

char *prepareResponseWrapper(int MessageType, char *UniqueId, char *action)
{
  char *string=(char *) calloc(1,sizeof(char) * 190);

  if (send_tags)
  {
	  sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":", MessageType, UniqueId); //Optional
  }
  else
  {
	  sprintf(string, "[%d,\"%s\",",MessageType, UniqueId); //Optional);
  }
  //string[strlen(string)]='\0';

  return string;
}

char *prepareResponseWrapper_i(int MessageType, int UniqueId, char *action)
{
  char *string=(char *) calloc(1,sizeof(char) * 190);

  if (send_tags)
  {
	  sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%d\",\"Payload\":", MessageType, UniqueId); //Optional
  }
  else
  {
	  sprintf(string, "[%d,\"%d\",",MessageType, UniqueId); //Optional);
  }
  //string[strlen(string)]='\0';

  //printf("String es: %s", string);
  return string;
}

char *prepareWrapper(int MessageType, const char *UniqueId, char *action)
{
  char *string=(char *) calloc(1,sizeof(char) * 512);

  if (send_tags)
  {
	  if (MessageType==_CALLMESSAGE)
	  {
		  sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Action\":\"%s\",\"Payload\":", MessageType, UniqueId, action); //Optional
	  }
	  else if (MessageType==_CALLRESULT)
	  {
		  sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":", MessageType, UniqueId, action); //Optional
	  }
  }
  else
  {
	  if (MessageType==_CALLMESSAGE)
	  {
		  sprintf(string, "[%d,\"%s\",\"%s\",",MessageType, UniqueId, action); //Optional);
	  }
	  else if (MessageType==_CALLRESULT)
	  {
		  sprintf(string, "[%d,\"%s\",",MessageType, UniqueId); //Optional);
	  }
  }
 // string[strlen(string)]='\0';

  //printf("String es: %s", string);
  return string;
}

char *prepareWrapper_i(int MessageType, int UniqueId, char *action)
{
  char *string=(char *) calloc(1,sizeof(char) * 512);

  if (send_tags)
  {
	  if (MessageType==_CALLMESSAGE)
	  {
		  sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%d\",\"Action\":\"%s\",\"Payload\":", MessageType, UniqueId, action); //Optional
	  }
	  else if (MessageType==_CALLRESULT)
	  {
		  sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%d\",\"Payload\":", MessageType, UniqueId); //Optional
	  }
  }
  else
  {
	  if (MessageType==_CALLMESSAGE)
	  {
		  sprintf(string, "[%d,\"%d\",\"%s\",",MessageType, UniqueId, action); //Optional);
	  }
	  else if (MessageType==_CALLRESULT)
	  {
		  sprintf(string, "[%d,\"%d\",",MessageType, UniqueId); //Optional);
	  }

  }
  //string[strlen(string)]='\0';

  //printf("String es: %s", string);
  return string;
}



//
//ERROR RESPONSE. Los error codes estan en
//
char *prepareErrorResponse(json_object * obj_UniqueId, int error_code){

	if (error_code >19) return NULL;  //Invalid error code. Se supone que son valores de 0 a 9. Indico 19 por si en el futuro se añaden nuevos.

	LogInDb(NULL, 99999, -1, "PROTOCOL_ERROR");

	char *message=(char *) calloc(1,sizeof(char) * 3000);
	char *UniqueId;
	if(obj_UniqueId)
	{
		UniqueId=json_object_get_string(obj_UniqueId);
	}
	else UniqueId="0";
	if (!UniqueId) UniqueId="0";

	char *errorCode=(char *) calloc(1,sizeof(char) * 3);  //Se entiende que el error code nunca va a tener mas de 2 digitos.
	char *errorDescription;

	snprintf (errorCode, sizeof(errorCode), "%d",error_code);

	switch (error_code){
		case _NOT_IMPLEMENTED:
			errorDescription="Requested Action is not known by receiver";
			break;
		case _NOT_SUPPORTED:
			errorDescription="Requested Action is recognized but not supported by the receiver";
			break;
		case _INTERNAL_ERROR:
			errorDescription="An internal error occurred and the receiver was not able to process the requested Action successfully";
			break;
		case _PROTOCOL_ERROR:
			errorDescription="Payload for Action is incomplete";
			break;
		case _SECURITY_ERROR:
			errorDescription="During the processing of Action a security issue occurred preventing receiver from completing the Action successfully";
			break;
		case _FORMATION_VIOLATION:
			errorDescription="Payload for Action is syntactically incorrect or not conform the PDU structure for Action";
			break;
		case _PROPERTY_CONSTRAINT_VIOLATION:
			errorDescription="Payload is syntactically correct but at least one field contains an invalid value";
			break;
		case _OCCURENCE_CONSTRAINT_VIOLATION:
			errorDescription="Payload for Action is syntactically correct but at least one of the fields violates ocurence constraints";
			break;
		case _TYPE_CONSTRAINT_VIOLATION:
			errorDescription="Payload for Action is syntactically correct but at least one of the fields violates data type constraints";
			break;
		case _GENERIC_ERROR:
			errorDescription="Any other error not covered by the previous ones";
			break;
	}

	 if (send_tags)
	 {
		sprintf(message, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"errorCode\":\"%s\",\"errorDescription\":\"%s\",\"errorDetails\":%s}",_CALLERROR, UniqueId, errorCode, errorDescription, "{}"); //Mandatory
	 }
	 else
	 {
		sprintf(message, "[%d,\"%s\",\"%s\",\"%s\",{\"%s\"}]",_CALLERROR, UniqueId, errorCode, errorDescription, ""); //Mandatory
	 }

	 return message;
}


//
// 4.1. AUTHORIZE
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Authorize".
//
char * prepareAuthorizeResponse(json_object * obj_UniqueId, struct IdTagInfo *idTagInfo){
	const char *UniqueId=json_object_get_string(obj_UniqueId);
	return prepareStopTransactionResponse(UniqueId, idTagInfo);
}

//
// 4.2. BOOT NOTIFICATION
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Boot Notification".
//
char * prepareBootNotificationResponse(json_object * obj_UniqueId, json_object * Payload, char *status)
{
	  char *message=(char *) calloc(1,sizeof(char) * 3000);
   	  const char *UniqueId=json_object_get_string(obj_UniqueId);
   	  char *wrapper=prepareWrapper(_CALLRESULT, UniqueId, "BootNotification");

   	  const char *chargePointModel, *chargePointVendor, *meterType, *chargeBoxSerialNumber, *chargePointSerialNumber, *firmwareVersion, *ICCID, *IMSI, *meterSerialNumber;

   	  //Ver Pag 34
   	  json_object *obj_chargePointModel = json_object_object_get(Payload, "chargePointModel");
   	  if (obj_chargePointModel) chargePointModel=json_object_get_string(obj_chargePointModel);

   	  json_object *obj_chargePointVendor=json_object_object_get(Payload, "chargePointVendor");
   	  if (obj_chargePointVendor) chargePointVendor=json_object_get_string(obj_chargePointVendor);

   	  json_object *obj_chargeBoxSerialNumber=json_object_object_get(Payload, "chargeBoxSerialNumber");
   	  if (obj_chargeBoxSerialNumber) chargeBoxSerialNumber=json_object_get_string(obj_chargeBoxSerialNumber);

   	  json_object *obj_chargePointSerialNumber=json_object_object_get(Payload, "chargePointSerialNumber");
   	  if (obj_chargePointSerialNumber) chargePointSerialNumber=json_object_get_string(obj_chargePointSerialNumber);

   	  json_object *obj_firmwareVersion=json_object_object_get(Payload, "firmwareVersion");
   	  if (obj_firmwareVersion) firmwareVersion=json_object_get_string(obj_firmwareVersion);

   	  json_object *obj_iccid=json_object_object_get(Payload, "iccid");
   	  if (obj_iccid) ICCID=json_object_get_string(obj_iccid);

   	  json_object *obj_imsi=json_object_object_get(Payload, "imsi");
   	  if (obj_imsi) IMSI=json_object_get_string(obj_imsi);

   	  json_object *obj_meterSerialNumber=json_object_object_get(Payload, "meterSerialNumber");
   	  if (obj_meterSerialNumber) meterSerialNumber=json_object_get_string(obj_meterSerialNumber);

   	  json_object *obj_meterType=json_object_object_get(Payload, "meterType");
   	  if (obj_meterType) meterType=json_object_get_string(obj_meterType);

   	  int heartbeat=getConfigurationKeyIntValue("HeartbeatInterval");
   	  char *currentTime=getCurrentTime();

   	  if (enums_as_integers)
   	  {
   		  //if (debug) printf("\n[LOG] prepareBootNotificationResponse. Replies with status %s", status);
   		  int status_int=-1;
   		  if (strcmp(status, "Accepted")==0) status_int=_RS_ACCEPTED;
   		  if (strcmp(status, "Rejected")==0) status_int=_RS_NOTCONNECTED;

   		  sprintf(message, "%s{\"status\":%d,\"currentTime\":\"%s\",\"heartbeatInterval\":%d}",wrapper, status_int, currentTime, heartbeat); //Mandatory
   	  }
   	  else
   	  {
   		  sprintf(message, "%s{\"status\":\"%s\",\"currentTime\":\"%s\",\"heartbeatInterval\":%d}",wrapper, status, currentTime, heartbeat); //Mandatory
   	  }

	  if (send_tags) strcat(message, "}");
	  else strcat(message, "]");

 	  return message;
}

//
// 4.3. DATA TRANSFER
//
// prepare_dataTransfer_request: Este método prepara la cadena JSON de una petición de dataTransfer que se mandará por JSON
//
// NOTAS: Lo dejo preparado para que tire de lo que le pasamos como parámetro, pero en la primera prueba, obtiene los datos de _DATA
//


//
// Esta función prepara el texto del mensaje de respuesta a una petición Data Transfer
// Ver Seccion 4.3
//
// The vendor Id should be Known to the Central System and uniquely identify the vendor specific implementation.
// The vendor Id should be a value from the reversed DNS namespace, where the top tiers of the name, when reversed,
// should correspond with the publicly registered primary DNS name of the vendor organisation.

char * prepareDataTransferResponse(json_object * obj_UniqueId, json_object * Payload){
	  char *message=(char *) calloc(1,sizeof(char) * 5000);
   	  const char *UniqueId=json_object_get_string(obj_UniqueId);
   	  const char *vendorId, *messageId;
	  char *data;
   	  char *status=(char *) calloc(1,sizeof(char) * 16);
   	  strcpy(status,"Accepted");
   	  status[strlen(status)]='\0';

	  //LogDataTransfer(); <-- Tarea no implementada

   	// Pag 36:
   	// The vendor Id should be Known to the Central System and uniquely identify the vendor specific implementation.
   	// The vendor Id should be a value from the reversed DNS namespace, where the top tiers of the name, when reversed,
   	// should correspond with the publicly registered primary DNS name of the vendor organisation.

   	  json_object *obj_vendorId = json_object_object_get(Payload, "vendorId");
   	  if (obj_vendorId) vendorId=json_object_get_string(obj_vendorId);

   	  //Pag 36: If the recipient of the request has no implementation for the specific vendorId it SHALL
   	  //return a status ‘UnknownVendor’  and  the  data  element  SHALL  not  be  present.  In  case  of
   	  //a  messageId  mismatch  (if used)  the  recipient  SHALL  return  status  ‘UnknownMessageId’.  In
   	  //all  other  cases  the  usage  of  status ‘Accepted’  or  ‘Rejected’  and  the  data  element  is
   	  //part  of  the  vendor-specific  agreement  between  the parties involved.
   	  if (!checkWellKnownVendorId(vendorId)) //<--No implementada. //Ver Pag 36
   	  {
   		strcpy(status,"UnknownVendor");
   		status[strlen(status)]='\0';
   	  }

      json_object *obj_messageId = json_object_object_get(Payload, "messageId");
   	  if (obj_messageId) vendorId=json_object_get_string(obj_messageId);


   	  if (!checkMessageId(messageId)){ //<--No implementada. //Ver Pag 36
 		strcpy(status,"UnknownMessageId");
 		status[strlen(status)]='\0';
   	  }


   	  //if the recipient of the request has no implementation for the specific vendorId it SHALL return a status
   	  //‘UnknownVendor’  and  the  data  element  SHALL  not  be  present.  In  case  of  a  messageId  mismatch  (if
   	  //used)  the  recipient  SHALL  return  status  ‘UnknownMessageId’.


   	  if (strcmp(status, "Accepted")==0){

   		  char *d;
   	      json_object *obj_data = json_object_object_get(Payload, "data");
   	   	  if (obj_data) d=json_object_get_string(obj_data);

   	      data=(char *) calloc(1,sizeof(char) * strlen(d)+10);
   	   	  sprintf(data, ",\"data\":\"%s\"",d);
   	  }
   	  else data="";

   	  //In  all  other  cases  the  usage  of  status
   	  //‘Accepted’  or  ‘Rejected’  and  the  data  element  is  part  of  the  vendor-specific  agreement  between  the
   	  //parties involved.
   	  if (checkDataTransferShouldbeRejected(Payload)==1)
   	  {
   	 		strcpy(status,"Rejected");
   	 		status[strlen(status)]='\0';
   	  }

 	  if (send_tags)
 	  {
 		 	  sprintf(message, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"status\":\"%s\"%s}}",_CALLRESULT, UniqueId, status, data); //Mandatory
 	  }
 	  else
 	  {
 			  sprintf(message, "[%d,\"%s\",\"%s\",\"%s\"]",_CALLRESULT, UniqueId, status, data); //Mandatory
 	  }


 	 return message;
}

//
// 4.4. DIAGNOSTICS STATUS NOTIFICATION
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Diagnostics Status Notification".
//Coge los datos de constantes en código y del fichero .INI
//
char *prepareDiagnosticsStatusNotificationResponse(const char* UniqueId){
	char *message=(char *) calloc(1,sizeof(char) * 300);

	 if (send_tags)
	 {
	 	  sprintf(message, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{}}",_CALLRESULT, UniqueId);//UniqueId); //Mandatory
	 }
	 else
	 {
	 	  sprintf(message, "[%d,\"%s\",{}]",_CALLRESULT, UniqueId);//UniqueId); //Mandatory
	 }

	return message;
}


//
// 4.5. FIRMWARE STATUS NOTIFICATION
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Firmware Status Notification".
//
char *prepareFirmwareStatusNotificationResponse(const char *UniqueId){
	char *message=(char *) calloc(1,sizeof(char) * 70);

	 if (send_tags)
	 {
	 	  sprintf(message, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{}}",_CALLRESULT, UniqueId); //Mandatory
	 }
	 else
	 {
	 	  sprintf(message, "[%d,\"%s\",{}]",_CALLRESULT, UniqueId); //Mandatory
	 }

	return message;
}


//
// 4.6. HEARTBEAT
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Heartbeat".
//PAG 37:
//Upon  receipt  of  a Heartbeat.req  PDU,  the  Central  System  SHALL  respond  with  a
//Heartbeat.conf.  The response  PDU  SHALL  contain  the  current  time  of  the  Central  System,  which  is  RECOMMENDED  to  be
//used by the Charge Point to synchronize its internal clock.
//
	char *prepareHeartbeatResponse(const char *UniqueId){
	char *string=(char *) calloc(1,sizeof(char) * 190);

  //Recuperamos fecha y hora
	  time_t now;
	  time(&now);
      struct tm* now_tm;
	  now_tm = localtime(&now);

	  char currentTime[80];
	  strftime (currentTime, 80, "%Y-%m-%dT%H:%M:%S.", now_tm);
	  //LogChargePointRestart(out); <-- Tarea no implementada
	  char *curr=currentTime;


	 if (send_tags)
	 {
	 	 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"currentTime\":\"%s\"}}",_CALLRESULT, UniqueId, curr); //Mandatory
	 }
	 else
	 {
	 	  sprintf(string, "[%d,\"%s\",{\"currentTime\":\"%s\"}]",_CALLRESULT, UniqueId, curr); //Mandatory
	 }


	return string;

}

//
// 4.7. METERVALUES
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Meter values".
//PAG 39:
//Upon receipt of a MeterValues.req  PDU, the Central System SHALL respond with a MeterValues.conf
//
//
char *prepareMeterValuesResponse(const char *UniqueId){
	 char *string=(char *) calloc(1,sizeof(char) * 190);

	 if (send_tags)
	 {
	 	 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{}}",_CALLRESULT, UniqueId); //Mandatory
	 }
	 else
	 {
	 	  sprintf(string, "[%d,\"%s\",{}]",_CALLRESULT, UniqueId); //Mandatory
	 }

	return string;
}


// 4.8. START TRANSACTION
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Start Transaction".
//
//
char *prepareStartTransactionResponse(const char *UniqueId, struct IdTagInfo *info, int transactionId)
{
	 char *string=(char *) calloc(1,sizeof(char) * 190);
	 printf("\nLA PROXIMA TRANSACCION SERA: %d", transactionId);
	 if (send_tags)
	 {
		 if (info->status==_CP_AUTHORIZATION_ACCEPTED||info->status==_CP_AUTHORIZATION_CONCURRENT_TX)
		 {
			 if (enums_as_integers)
			 {
				 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"idTagInfo\":{\"expiryDate\":\"%s\", \"parentIdTag\":\"%s\",\"status\":%d}, \"transactionId\":%d}}",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, info->status, transactionId); //Mandatory
			 }
			 else
			 {
				 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"idTagInfo\":{\"expiryDate\":\"%s\", \"parentIdTag\":\"%s\",\"status\":\"%s\"}, \"transactionId\":%d}}",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, AuthorizationStatusTexts[info->status], transactionId); //Mandatory
			 }
		 }
		 else
		 {
			 if (enums_as_integers)
			 {
				 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"idTagInfo\":{\"status\":%d}, \"transactionId\":%d}}",_CALLRESULT, UniqueId, info->status, transactionId); //Mandatory
			 }
			 else
			 {
				 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"idTagInfo\":{\"status\":\"%s\"}, \"transactionId\":%d}}",_CALLRESULT, UniqueId, AuthorizationStatusTexts[info->status], transactionId); //Mandatory
			 }
		 }
	 }
	 else
	 {
		 if (info->status==_CP_AUTHORIZATION_ACCEPTED||info->status==_CP_AUTHORIZATION_CONCURRENT_TX)
		 {
			 if (enums_as_integers)
			 {
				 sprintf(string, "[%d,\"%s\",{\"idTagInfo\":{\"expiryDate\":\"%s\", \"parentIdTag\":\"%s\",\"status\":%d}, \"transactionId\":%d}]",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, info->status, transactionId); //Mandatory
			 }
			 else
			 {
				 sprintf(string, "[%d,\"%s\",{\"idTagInfo\":{\"expiryDate\":\"%s\", \"parentIdTag\":\"%s\",\"status\":\"%s\"}, \"transactionId\":%d}]",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, AuthorizationStatusTexts[info->status], transactionId); //Mandatory
			 }
		 }
		 else
		 {
			 if (enums_as_integers)
			 {
				 sprintf(string, "[%d,\"%s\",{\"idTagInfo\":{\"status\":%d}, \"transactionId\":%d}]",_CALLRESULT, UniqueId, info->status, transactionId); //Mandatory
			 }
			 else
			 {
				 sprintf(string, "[%d,\"%s\",{\"idTagInfo\":{\"status\":%s}, \"transactionId\":%d}]",_CALLRESULT, UniqueId, AuthorizationStatusTexts[info->status], transactionId); //Mandatory
			 }
		 }
	 }

	return string;
}


// 4.9. STATUS NOTIFICATION
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Status Notification".
//
//
char *prepareStatusNotificationResponse(const char *UniqueId){
	 char *string=(char *) calloc(1,sizeof(char) * 190);

	 if (send_tags)
	 {
	 	 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{}}",_CALLRESULT, UniqueId); //Mandatory
	 }
	 else
	 {
	 	  sprintf(string, "[%d,\"%s\",{}]",_CALLRESULT, UniqueId); //Mandatory
	 }

	return string;
}

// 4.10. STOP TRANSACTION
//
//Este método prepara el JSON que se enviará desde el CP al CS para la solicitd "Stop Transaction".
//
//
char *prepareStopTransactionResponse(const char *UniqueId, struct IdTagInfo *info){
	 char *string=(char *) calloc(1,sizeof(char) * 190);

	 if (send_tags)
	 {
		 if (info)
		 {
			 if (enums_as_integers)
			 {
				 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"idTagInfo\":{\"expiryDate\":\"%s\",\"parentIdTag\":\"%s\",\"status\":%d}}}",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, info->status); //Mandatory
			 }
			 else
			 {
				 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{\"idTagInfo\":{\"expiryDate\":\"%s\",\"parentIdTag\":\"%s\",\"status\":\"%s\"}}}",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, AuthorizationStatusTexts[info->status]); //Mandatory
			 }
		 }
		 else
		 {
			 sprintf(string, "{\"MessageTypeId\":%d,\"UniqueId\":\"%s\",\"Payload\":{}}",_CALLRESULT, UniqueId); //Mandatory
		 }
	 }
	 else
	 {
		if (info)
		{
			 if (enums_as_integers)
			 {
				 sprintf(string, "[%d,\"%s\",{\"idTagInfo\":{\"expiryDate\":\"%s\",\"parentIdTag\":\"%s\",\"status\":%d}}]",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, info->status); //Mandatory
			 }
			 else
			 {
				 sprintf(string, "[%d,\"%s\",{\"idTagInfo\":{\"expiryDate\":\"%s\",\"parentIdTag\":\"%s\",\"status\":\"%s\"}}]",_CALLRESULT, UniqueId, info->expiryDate, info->parentIdtag.IdToken, AuthorizationStatusTexts[info->status]); //Mandatory
			 }
		}
		else
		{
			sprintf(string, "[%d,\"%s\",{}]",_CALLRESULT, UniqueId);
		}
	 }

	return string;
}

//
// 5.1 CANCEL RESERVATION
//
// Pag 48: To  cancel  a  reservation  the  Central  System  SHALL  send  an CancelReservation.req  PDU  to  the  ChargePoint.
char *prepareCancelReservationRequest(int UniqueId, char *reservationID)
{
	char *string=(char *) calloc(120, sizeof(char));
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "CancelReservation");
	//printf("En prepareCancelReservationRequest, wrapper es: %s", wrapper);
	sprintf(string, "%s{\"reservationId\":%d}",wrapper, atoi(reservationID)); //Mandatory
	//string[sizeof(string)]='\0';
	if (send_tags) strcat(string,"}");
	else strcat(string,"]");
	//printf("En prepareCancelReservationRequest, string es: %s", string);

	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pag 48: Central  System  can  request  a  Charge  Point  to  change  its  availability.  A  Charge  Point  is  considered
//available  (“operative”)  when  it  is  charging  or  ready  for  charging.  A  Charge  Point  is  considered
//unavailable    when    it    does    not    allow    any    charging.    The    Central    System    SHALL    send    a ChangeAvailability.req
// PDU for requesting a Charge Point to change its availability. The Central System
//can change the availability to available or unavailable (0 or 1).

//
// 5.2 CHANGE AVAILABILITY
//
char *prepareChangeAvailabilityRequest(int UniqueId, int connector, int new_availability)
{
	char *string=(char *) calloc(1,sizeof(char) * 120);
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "ChangeAvailability");

	if (enums_as_integers)
	{
		sprintf(string, "%s{\"connectorId\":%d, \"type\":%d}",wrapper, connector, new_availability); //Mandatory
	}
	else
	{
		sprintf(string, "%s{\"connectorId\":%d, \"type\":%d}",wrapper, connector, availabilityTypeTexts[new_availability]); //Mandatory
	}
	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	string[strlen(string)]='\0';

	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.3 CHANGE CONFIGURATION
//
//Pag 49: Central System can request a Charge Point to change configuration parameters. To achieve this, Central
//System SHALL send a ChangeConfiguration.req. This request contains a key-value pair, where "key" is
//the  name  of  the  configuration  setting  to  change  and  "value"  contains  the  new  setting  for  the
//configuration setting.
//
char *prepareChangeConfigurationRequest(int UniqueId, char *key, char *value)
{
	char *string=(char *) calloc(1,sizeof(char) * (90+strlen(key)+strlen(value)));
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "ChangeConfiguration");

	sprintf(string, "%s{\"key\":%d, \"value\":\"%s\"}",wrapper, key,  value); //Mandatory

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	string[strlen(string)]='\0';

	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.4 CLEAR CACHE
//
//Pag 49: Central System can request a Charge Point to clear its Authorization Cache. The Central System SHALL
//send a ClearCache.req PDU for clearing the Charge Point’s Authorization Cache.
//
char *prepareClearCacheRequest(int UniqueId)
{
	char *string=(char *) calloc(90, sizeof(char));
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "ClearCache");

	sprintf(string, "%s{}",wrapper); //Mandatory

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	//string[strlen(string)]='\0';
	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.5 CLEAR CHARGING PROFILE
//
char *prepareClearChargingProfileRequest(int UniqueId, const char *chargingprofileid, const char *connectorid_str, const char *purpose_str, const char *stacklevel_str)
{
	char *string=(char *) calloc(190, sizeof(char));
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "ClearChargingProfile");

	//PREPARACION DEL PAQUETE
	sprintf(string, "%s{",wrapper); //Mandatory

	//Optional (Integer)
	if (chargingprofileid)
	{
			strcat(string, ",\"id\":");
			strcat(string, chargingprofileid);
	}

	//Optional (Integer)
	if (connectorid_str)
	{
		strcat(string, ",\"connectorId\":");
		strcat(string, connectorid_str);
	}

	//Optional
	if (purpose_str)
	{
		if (enums_as_integers)
		{
			strcat(string, ",\"chargingProfilePurpose\":");
			strcat(string, purpose_str);
		}
		else
		{
			strcat(string, ",\"chargingProfilePurpose\":\"");
			strcat(string, purpose_str);
			strcat(string, "\"");
		}
	}

	//Optional (Integer)
	if (stacklevel_str)
	{
		strcat(string, ",\"stackLevel\":\""); //Mandatory
		strcat(string, stacklevel_str); //Mandatory
	}

	if (send_tags) strcat(string,"}}");
	else strcat(string,"}]");

	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.6 CS DATA TRANSFER
//
//If  the  Central  System  needs  to  send  information  to  a  Charge  Point  for  a  function  not  supported  by
//OCPP, it SHALL use the DataTransfer.req PDU.
//
char *prepareDataTransferRequest(int UniqueId, char *vendorId, char *messageId, char *data)
{
	char *string=(char *) calloc(128+strlen(vendorId)+strlen(messageId)+strlen(data), sizeof(char));
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "CSDataTransfer");
	int i=0;

	//PREPARACION DEL PAQUETE
	sprintf(string, "%s{\"vendorId\":\"%s\"",wrapper, vendorId); //Mandatory

	//Optional
	if (messageId)
	{
		strcat(string, ",\"messageId\":\""); //Mandatory
		strncat(string, messageId, strlen(messageId)); //Mandatory
		strcat(string, "\""); //Mandatory
	}


	//Optional
	if (data)
	{
		strcat(string, ",\"data\":\""); //Mandatory
		strncat(string, data, strlen(data)); //Mandatory
		strcat(string, "\""); //Mandatory
	}

	if (send_tags) strcat(string,"}}");
	else strcat(string,"}]");

	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.8 GET CONFIGURATION
//
char *prepareGetConfigurationRequest(int UniqueId, char *key)
{
	char *string=(char *) calloc(4096,sizeof(char));
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "GetConfiguration");
	char *payload=(char *)calloc(4096, sizeof(char));

	if (strlen(key)==0)
	{
		strcpy(payload, "\"key\":[]");
	}
	else
	{
		char *t;
		char** tokens = str_split(strdup(key), ',');

		int i=0;
		t=*(tokens);
	/*
		while (t)
		{
			if (i==0)
			{
				strcpy(payload, "\"key\":\"");
			}
			else
			{
				strcat(payload, ",\"key\":\"");
			}

			strncat(payload, trimwhitespace(t), 50);
			strcat(payload, "\"");
			i++;
			t=*(tokens+i);
		}
	*/

		strcpy(payload, "\"key\":[");
		while (t)
		{
			if (i!=0)
			{
				strcat(payload, ",");
			}
			strcat(payload, "\"");
			strncat(payload, trimwhitespace(t), 50);
			strcat(payload, "\"");
			i++;
			t=*(tokens+i);
		}
		strcat(payload, "]");
	}

		sprintf(string, "%s{%s}",wrapper,payload); //Mandatory

		if (send_tags) strcat(string,"}");
		else strcat(string,"]");

		string[strlen(string)]='\0';
		return string;

}

//
// 5.9 GET DIAGNOSTICS
//
char *prepareGetDiagnosticsRequest(int UniqueId, const char *location, int retries, int retryInterval, const  char *startTime, const char *stopTime)
{
	char *string=(char *) calloc(256, sizeof(char));

	char *payload=(char *) calloc(216, sizeof(char));

	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "GetDiagnostics");

	//Compulsory
	strcpy(payload, "\"location\":\"");
	strncat(payload, location, 64); //Max length of URI=64. No lo pone pero lo digo yo. Ver 6.25
	strcat(payload, "\"");

	if (retries>0)
	{
		strcat(payload, ",\"retries\":");
		strcat(payload, convert(retries));
	}

	if (retryInterval>0)
	{
		strcat(payload, ",\"retryInterval\":");
		strcat(payload, convert(retryInterval));
	}

	if (startTime)
	{
			strcat(payload, ",\"startTime\":\"");
			strncat(payload, startTime, 32);
			strcat(payload, "\"");
	}

	if (stopTime)
	{
			strcat(payload, ",\"stopTime\":\"");
			strncat(payload, stopTime, 32);
			strcat(payload, "\"");
	}

	sprintf(string, "%s{%s}",wrapper, payload);

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	free(payload);
	return string;
}

//
// 5.10 GET LOCAL LIST VERSION
//
char *prepareGetLocalListVersionRequest(int UniqueId)
{
	char *string=(char *) calloc(90, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "GetLocalListVersion");

	sprintf(string, "%s{}",wrapper); //Mandatory

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
// 5.11 REMOTE START TRANSACTION
//
char *prepareRemoteStartTransactionRequest(int UniqueId, char *idTag, int connectorId, int transaction, int stackLevel, int purpose, int kind, int recurrency, char *validfrom, char *validto, int chargingProfile, int duration, char *startSchedule, int chargingRateUnit, struct ChargingSchedulePeriod periodos[], int num_periods, float minChargingRate)
{
	char *string=(char *) calloc(1024, sizeof(char) );
	char *payload=(char *) calloc(980, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "RemoteStartTransaction");

	strcpy(payload, "\"idTag\":\"");
	strncat(payload, idTag, 20);
	strncat(payload, "\"", 20);

	if (connectorId)
	{
		strcat(payload, ",\"connectorId\":");
		strncat(payload, convert(connectorId), 1);
	}

	if (chargingProfile)
	{
		//chargingProfileId. Se trata de un numero que aumenta cada vez que se le envía un charging profile al cliente
		//storeCharginProfileInfo(chargingProfileId) <-- NOT IMPLEMENTED. O bien en DB o bien en memoria
		strcat(payload, ",\"chargingProfile\":{\"chargingProfileId\":");
		strcat(payload,convert(chargingProfileId++));

		//stackLevel
		strcat(payload, ",\"stackLevel\":");
		strcat(payload,convert(stackLevel));

		//transactionId
		if (transaction>0)
		{
			strcat(payload,",\"transactionId\":");
			strcat(payload,convert(transaction));
		}

		if (enums_as_integers)
		{
			strcat(payload, ",\"chargingProfilePurpose\":");
			strcat(payload,convert(purpose));

			strcat(payload, ",\"chargingProfileKind\":");
			strcat(payload,convert(kind));

			if (recurrency>=0)
			{
				strcat(payload, ",\"recurrencyKind\":");
				strcat(payload,convert(recurrency));
			}
		}
		else
		{
			strcat(payload, ",\"chargingProfilePurpose\":\"");
			strcat(payload,ChargingProfilePurposeTypeTexts[purpose]);
			strcat(payload, ",\"");

			strcat(payload, ",\"chargingProfileKind\":\"");
			strcat(payload,ChargingProfileKindTypeTexts[kind]);
			strcat(payload, ",\"");

			if (recurrency>=0)
			{
				strcat(payload, ",\"recurrencyKind\":\"");
				strcat(payload,RecurrencyKindTypeTexts[recurrency]);
				strcat(payload, ",\"");
			}
		}

		//validfrom
		if (validfrom)
		{
			strcat(payload, ",\"validFrom\":\"");
			strcat(payload, validfrom);
			strcat(payload, "\"");
		}

		if (validto)
		{
					strcat(payload, ",\"validTo\":\"");
					strcat(payload, validto);
					strcat(payload, "\"");
		}

		strcat(payload, "\"chargingSchedule\":{");

		if (duration>=0)
		{
			strcat(payload, ",\"duration\":");
			strcat(payload, convert(duration));
		}

		if (startSchedule)
		{
			strcat(payload, ",\"startSchedule\":");
			strcat(payload, startSchedule);
		}

		if (chargingRateUnit>=0)
		{
			if (enums_as_integers)
			{
				strcat(payload, ",\"chargingRateUnit\":");
				strcat(payload, convert(chargingRateUnit));
			}
			else
			{
				strcat(payload, ",\"chargingRateUnit\":\"");
				strcat(payload, ChargingRateUnitTypeTexts[chargingRateUnit]);
				strcat(payload, "\"");
			}
		}

		///chargingSchedulePeriod
		strcat(payload, ",\"chargingSchedulePeriod\":[");
		for (int i=0; i<num_periods; i++)
		{
			if (i>0) strcat(payload, ",");
			strcat(payload, "\"chargingSchedulePeriod\":{");
			strcat(payload, "\"startPeriod\":");
			strcat(payload, convert(periodos[i].startPeriod));
			strcat(payload, "\"limit\":");
			strcat(payload, convert(periodos[i].limit));
			if (periodos[i].numPhases)
			{
				strcat(payload, "\"numberPhases\":");
				strcat(payload, convert(*periodos[i].numPhases));
			}
			strcat(payload, "}");
		}

		//Cerramos el listado de charging Schedule Period
		strcat(payload, "]");

		///MINCHARGINRATE
		if (minChargingRate>=0.0)
		{
			strcat(payload, ",\"minChargingRate\":");
			strcat(payload, convertF(minChargingRate));
		}

		//Cerramos el charging Schedule
		strcat(payload, "}");

		//Cerramos el charging Profile
		strcat(payload, "}");
	}

	sprintf(string, "%s{%s}",wrapper, payload); //Mandatory

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");


	return string;
}

//
// 5.12
//
char *prepareRemoteStartTransactionStopRequest(int UniqueId, int transactionId)
{
	char *string=(char *) calloc(190, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "RemoteStopTransaction");

	sprintf(string, "%s{\"transactionid\":%d}", wrapper, transactionId); //Mandatory

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
// 5.13
//
char *prepareReserveNowRequest(int UniqueId, int connector, char *expiryDate, char *idTag, int reservationId, char *parentIdTag)
{
	char *string=(char *) calloc(190, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "ReserveNow");
	if (parentIdTag)
	{
		sprintf(string, "%s{\"connectorId\":%d, \"expiryDate\":\"%s\", \"idTag\":\"%s\", \"reservationId\":%d,\"parentIdTag\":\"%s\"}",wrapper, connector,  expiryDate, idTag, reservationId, parentIdTag); //Mandatory
	}
	else
	{
		sprintf(string, "%s{\"connectorId\":%d, \"expiryDate\":\"%s\", \"idTag\":\"%s\", \"reservationId\":%d}",wrapper, connector,  expiryDate, idTag, reservationId); //Mandatory
	}

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
// 5.14 RESET
//
char * prepareResetRequest(int UniqueId, int resetType)
{
	char *string=(char *) calloc(190, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "Reset");

	if (enums_as_integers)
	{
		sprintf(string, "%s{\"type\":%d}",wrapper, resetType); //Mandatory
	}
	else
	{
		sprintf(string, "%s{\"type\":\"%s\"}",wrapper, ResetType_texts[resetType]); //Mandatory
	}

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
// 5.15
//
char *prepareSendLocalListRequest(int UniqueId, int version, struct authorization_list_entry *list , int type)
{
	char *string=(char *) calloc(190, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "SendLocalList");

	struct authorization_record *data;
	struct authorization_list_entry *inicio_lista=list;

	//Ver pag 77:
	sprintf(string, "%s{\"listVersion\":%d",wrapper, version); //Mandatory

	//UpdateType
	if (enums_as_integers)
	{
			strcat(string, ",\"updateType\":");
			strcat(string, convert(type));
	}
	else
	{
		strcat(string, ",\"updateType\":\"");
		strcat(string, UpdateType_texts[type]);
		strcat(string, "\"");
	}


	if (list) strcat(string, ",\"localAuthorizationList\":[");
	//Ptro al ppio de la lista:

	int firstAuthorizationTag=1;
	int max=getConfigurationKeyIntValue("SendLocalListMaxLength");

	//Pag 77: Maximum number of	AuthorizationData	elements is available in	the configuration key:	SendLocalListMaxLength
	if (max<0)
	{
		//No se ha definido esta configuration key
		while (list)
		{
			if (!firstAuthorizationTag) strcat(string, ",");
			strcat(string, "{\"idTag\":\"");
			strcat(string, list->idTag);
			strcat(string, "\"");

			struct authorization_record *data=list->entry;
			firstAuthorizationTag=0;
			int firstIdtagInfoField=1;
			if (data) //optional
			{
				strcat(string, ",\"idTagInfo\":{");

				if (data->expiryDate){

					char *expiryDate=(char *)calloc(1, sizeof(char)*80);
					strftime (expiryDate, 80, "%Y-%m-%dT%H:%M:%S.", data->expiryDate);

					strcat(string, "\"expiryDate\":\""); //optional
					strcat(string, expiryDate); //optional
					strcat(string, "\"");

					firstIdtagInfoField=0;
				}

				if (data->parentIdTag && strlen(data->parentIdTag)>1)
				{
					if (!firstIdtagInfoField) strcat(string, ",");

					strcat(string, "\"parentIdTag\":\"");
					strcat(string, data->parentIdTag);
					strcat(string, "\"");

					firstIdtagInfoField=0;
				}

				if (!firstIdtagInfoField) strcat(string, ",");
				if (enums_as_integers)
				{
					strcat(string, "\"status\":");
					strcat(string, convert(data->status));
				}
				else
				{
					strcat(string, "\"status\":\"");
					strcat(string, AuthorizationStatusTexts[data->status]);
					strcat(string, "\"");
				}

				//Cerramos el idtaginfo
				strcat(string, "}");
			}

			strcat(string, "}");
			list=list->next;
		}
	}
	else
	{
		//Se ha definido un max numero de authorization lists
		for (int i=0; i<max; i++)
		{
			if (list)
			{
				if (!firstAuthorizationTag) strcat(string, ",");
				strcat(string, "{\"IdTag\":\"");
				strcat(string, list->idTag);
				strcat(string, "\"");

				struct authorization_record *data=list->entry;
				firstAuthorizationTag=0;
				int firstIdtagInfoField=1;

				if (data) //optional
				{
					strcat(string, ",\"IdTagInfo\":{");

					if (data->expiryDate){
						strcat(string, "\"expiryDate\":\""); //optional
						strcat(string, list->idTag); //optional
						strcat(string, "\"");

						firstIdtagInfoField=0;
					}

					if (strlen(data->parentIdTag)<1)
					{
						if (!firstIdtagInfoField) strcat(string, ",");

						strcat(string, "\"parentIdTag\":\"");
						strcat(string, list->idTag);
						strcat(string, "\"");

						firstIdtagInfoField=0;
					}

					if (!firstIdtagInfoField) strcat(string, ",");
					if (enums_as_integers)
					{
						strcat(string, "\"status\":");
						strcat(string, convert(data->status));
					}
					else
					{
						strcat(string, "\"status\":\"");
						strcat(string, AuthorizationStatusTexts[data->status]);
						strcat(string, "\"");
					}

					//Cerramos el idtaginfo
					strcat(string, "}");
				}
			}
		}
		//Cerramos el authorizationData
		strcat(string, "}");
	}

	//Solo cerramos el JSON de localAuthorizationList, si se recibio una authorization list. No podemos usar "list" porque la vamos cambiando.
	if (inicio_lista) strcat(string, "]");




	//Cerramos el payload
	strcat(string, "}");

	//Cerramos el mensaje
	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
// 5.16
//
char *prepareSetChargingProfileRequest(int UniqueId, int connector, char *chargingProfile, char *transactionId)
{
	int chargingProfileId=atoi(chargingProfile);
	struct ChargingProfile *profile=chargingProfiles[atoi(chargingProfile)];

	char *string=(char *) calloc(1024, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "SetChargingProfile");

	sprintf(string, "%s{\"ConnectorId\":%d,\"csChargingProfiles\":{",wrapper, connector);

	//Campo 1. El ID es el ultimo , y al que le sumamos uno
	int id=chargingProfileId++;
	strcat(string, "\"chargingProfileId\":");
	strcat(string, convert(id));

	//Campo 2. TransactionId (Opcional)
	if (transactionId)
	{
		strcat(string, ",\"transactionId\":\"");
		strcat(string, transactionId);
		strcat(string, "\"");
	}

	//Campo 3. StackLevel
	strcat(string, ",\"stackLevel\":");
	strcat(string, convert(profile->stackLevel));

	//Campo 4. ChargingProfilePurpose
	strcat(string, ",\"chargingProfilePurpose\":");

	if (enums_as_integers)
	{
		strcat(string, convert(profile->chargingProfilePurpose));
	}
	else
	{
		strcat(string, "\"");
		strcat(string, ChargingProfilePurposeTypeTexts[profile->chargingProfilePurpose]);
		strcat(string, "\"");
	}

	//Campo 5. ChargingProfileKindType
	strcat(string, ",\"chargingProfileKind\":");

	if (enums_as_integers)
	{
		strcat(string, convert(profile->chargingProfileKind));
	}
	else
	{
		strcat(string, "\"");
		strcat(string, ChargingProfileKindTypeTexts[profile->chargingProfileKind]);
		strcat(string, "\"");
	}

	//Campo 6. Recurrency Kind
	if (profile->recurrencyKind)
	{
		strcat(string, ",\"recurrencyKind\":");

		if (enums_as_integers)
		{
			strcat(string, convert(*profile->recurrencyKind));
		}
		else
		{
			strcat(string, "\"");
			strcat(string, RecurrencyKindTypeTexts[*profile->recurrencyKind]);
			strcat(string, "\"");
		}
	}

	char *validFrom;
	//Campo 7. Valid From. Optional
	if (profile->validFrom)
	{
		strftime (validFrom, 80, "%Y-%m-%dT%H:%M:%S.", profile->validFrom);

		strcat(string, ",\"validFrom\":\"");
		strcat(string, validFrom);
		strcat(string, "\"");

	}

	char *validTo;
	//Campo 8. Valid To. Optional
	if (profile->validTo)
	{
		strftime (validTo, 80, "%Y-%m-%dT%H:%M:%S.", profile->validTo);

		strcat(string, ",\"validTo\":\"");
		strcat(string, validTo);
		strcat(string, "\"");
	}

	//Campo 9. ChargingSchedule
	strcat(string, ",\"chargingSchedule\":{");
	int first=1;

	//Campo 9.1
	if (profile->chargingSchedule.duration)
	{
		strcat(string, "\"duration\":");
		strcat(string, convert(*profile->chargingSchedule.duration));
		first=0;
	}

	//Campo 9.2
	char *startSchedule;
	if (profile->chargingSchedule.startSchedule)
	{
		strftime (startSchedule, 80, "%Y-%m-%dT%H:%M:%S.", profile->chargingSchedule.startSchedule);

		if (!first) strcat(string, ",");
		strcat(string, "\"startSchedule\":\"");
		strcat(string, startSchedule);
		strcat(string, "\"");
		first=0;
	}

	//Campo 9.3
	if (!first) strcat(string, ",");

	strcat(string, "\"chargingRateUnit\":");
	if (enums_as_integers)
	{
		strcat(string, convert(profile->chargingSchedule.chargingRateUnit));
	}
	else
	{
		strcat(string, "\"");
		strcat(string, ChargingRateUnitTypeTexts[profile->chargingSchedule.chargingRateUnit]);
		strcat(string, "\"");
		first=0;
	}

	//Campo 9.4
	strcat(string, "\"chargingSchedulePeriod\":[");
	struct ChargingSchedulePeriod *temp=&profile->chargingSchedule.chargingSchedulePeriods;

	first=1;
	while (temp)
	{
		if (!first) strcat(string, ",");
		strcat(string, "{");
		//Campo 9.4.1
		strcat(string, "\"startPeriod\":");
		strcat(string, convert(temp->startPeriod));

		if (temp->limit)
		{
			strcat(string, ",\"limit\":");
			char *p=convertF(temp->limit);
			strcat(string, p);
		}

		if (temp->numPhases)
		{
			strcat(string, ",\"numberPhases\":");
			int np=temp->numPhases;
			strcat(string, convert(np));
		}

		strcat(string, "}");
		first=0;
		temp=temp->next;
	}

	//Cerramos la lista
	strcat(string, "]");

	//Campo 9.5 (optional)
	if (profile->chargingSchedule.minChargingRate)
	{
		strcat(string, ",\"minChargingRate\":");
		strcat(string, convertF(*profile->chargingSchedule.minChargingRate));
	}

	//Cerramos el Charging Schedule
	strcat(string, "}");

	//Cerramos el Charging Profile
	strcat(string, "}");

	//Cerramos el Payload
	strcat(string, "}");

	//Cerramos el mensaje
	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
// 5.17 TRIGGER MESSAGE
//
char *prepareTriggerMessageRequest(int UniqueId, int connector, int messageID)
{
	char *string=(char *) calloc(190, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "TriggerMessage");

	if (enums_as_integers)
	{
		if (connector>=0)
		{
			sprintf(string, "%s{\"RequestedMessage\":%d, \"connectorId\":%d}",wrapper, messageID, connector);
		}
		else
		{
			sprintf(string, "%s{\"RequestedMessage\":%d}",wrapper, messageID);
		}
	}
	else
	{
		if (connector>=0)
		{
			sprintf(string, "%s{\"RequestedMessage\":\"%s\", \"connectorId\":%d}",wrapper, MessageTrigger_texts[messageID], connector);
		}
		else
		{
			sprintf(string, "%s{\"RequestedMessage\":\"%s\"}",wrapper, MessageTrigger_texts[messageID]);
		}
	}

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
// 5.18 UNLOCK CONNECTOR
//
char *prepareUnlockConnectorRequest(int UniqueId, int connector)
{
	char *string=(char *) calloc(190, sizeof(char) );
	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "UnlockConnector");

	sprintf(string, "%s{\"connectorId\":%d}",wrapper, connector); //Mandatory

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}

//
//5.19 FIRMWARE UPDATE
//
char *prepareFirmwareUpdateRequest(int UniqueId, char *location, char *retrieveDate, int retries, int retryInterval)
{
	char *string=(char *) calloc(256, sizeof(char));

	char *payload=(char *) calloc(216, sizeof(char));

	char *wrapper=prepareWrapper_i(_CALLMESSAGE, UniqueId, "UpdateFirmware");

	//Compulsory
	strcpy(payload, "\"location\":\"");
	strncat(payload, location, 64); //Max length of URI=64. No lo pone pero lo digo yo. Ver 6.25
	strcat(payload, "\"");

	strcat(payload, ",\"retrieveDate\":\"");
	strncat(payload, retrieveDate, 32);
	strcat(payload, "\"");

	//optional
	if (retries>0)
	{
		strcat(payload, ",\"retries\":");
		strcat(payload, convert(retries));
	}

	//optional
	if (retryInterval>0)
	{
		strcat(payload, ",\"retryInterval\":");
		strcat(payload, convert(retryInterval));
	}

	sprintf(string, "%s{%s}",wrapper, payload);

	if (send_tags) strcat(string,"}");
	else strcat(string,"]");

	return string;
}
