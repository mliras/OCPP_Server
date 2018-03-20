/*
 ============================================================================
 Name        : Websockets-server.c
 Author      : Luis Martin
 Version     : 0.1
 Copyright   : Your copyright notice
 Description : Websockets Server in C, Ansi-style
 Link        : Es necesario linkar websockets
 ============================================================================
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include "server.h"
#include "ocpp_ini_parser.h"

#define PORT 5000
#define SERVER "localhost"

///BOOT NOTIFICATION CONF

//static int debug=1;
//static int send_tags=1;
static int destroy_flag = 0;

void Log(char *text, struct chargePoint *cp, int error_code, int error_level)
{
	if (conn)
	{
		LogInDb(cp, error_code, error_level, text);
	}

	if (logInScreen)
	{
		char *msg=(char *)calloc(1,1024);
		strcpy(msg, "Message ");
		strcat(msg, convert(error_code));
		strcat(msg, ": ");
		strcat(msg, text);
		int color=0;
		switch (error_level)
		{
			case AL_ERROR:
				color=RED;
				break;
			case AL_WARNING:
				color=BROWN;
				break;
			case AL_INFO:
				color=BLUE;
				break;
		}

		appendLog(msg, color);
	}
}

//
//4.1 y 4.8
//
struct IdTagInfo *authorizeIdTag(const char* idTag)
{
	//Upon receipt of an Authorize.req PDU, the Central System SHALL respond with an Authorize.conf PDU.
	//This response PDU SHALL indicate whether or not the idTag is accepted by the Central System. If the
	//Central System accepts the idTag then the response PDU MAY include a parentIdTag and MUST include
	//an authorization status value indicating acceptance or a reason for rejection.

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

	info->status=_CP_AUTHORIZATION_ACCEPTED; //Aceptamos siempre!!

	return info;
}

static void INT_HANDLER(int signo) {
    destroy_flag = 1;
}

/* *
 * Esta funcion escribe el texto que se le pasa en str en el websocket
 */
int websocket_write_back(struct lws *wsi_in, char *str, int str_size_in)
{
    if (str == NULL || wsi_in == NULL)
        return -1;

    int n;
    int len;
    char *out = NULL;

    if (str_size_in < 1)
        len = strlen(str);
    else
        len = str_size_in;

    if (debug) printf(AZUL"[websocket_write_back] %s\n"RESET, str);

    char *p=str;
    for (int i=0; i<len; i++)
    {
    	p[i]+=128;
    }

    //Reservamos memoria
    out = (char *)calloc(1,sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));

    //Preparamos el buffer
    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len);

    //Y lo escribimos
    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

    //* ...y liberamos el buffer!!*/
    free(out);

    return n;
}

/* *
 * Esta funcion escribe el texto que se le pasa en str en el websocket
 */
int websocket_write_to_client(struct lws *wsi_in, char *str, int str_size_in)
{
    if (str == NULL || wsi_in == NULL)
        return -1;

    int n;
    int len;
    char *out = NULL;

    ////////Logging///////
    char *msg=(char *)calloc(1,2048);
    strcat(msg, "[websocket_write_to_client] ");
    strncat(msg, str, strlen(str));

    if (debug) printf(AMARILLO"%s\n"RESET, msg);
    LogInDb2(NULL,NULL,NULL,NULL, 1, AL_DEBUG, msg);
    free(msg);
    ////////Logging///////

    if (str_size_in < 1)
        len = strlen(str);
    else
        len = str_size_in;



    //if (debug) printf("ANTES DE CIFRAR, STR ES %s",str);
    //ciframos +128
      char *p=str;
      for (int i=0; i<len; i++)
      {
      	p[i]+=128;
      }
     //if (debug)  printf("DESPUES DE CIFRAR, STR ES %s",str);

    //Reservamos memoria
    out = (char *)calloc(1,sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));

///////////////////
//    printf("EN CLARO ES: %s", str);
//    //Se hace un doble encriptado del texto enviado:
//    char *textoencriptado=encrypt(encrypt(str,13),12);
 //   printf("ENCRIPTADO2 ES: %s", textoencriptado);
//* Preparamos el buffer*/
//    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, textoencriptado, len );
///////////////////

    //* Preparamos el buffer*/
    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );

    //* Y lo escribimos*/
    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);



    //* ...y liberamos el buffer!!*/
    free(out);

    return n;
}


//Esta es la funcion de callback que se ejecuta cuando se recibe una peticion del cliente
static int service_callback(struct lws *wsi,enum lws_callback_reasons reason, void *user,void *in, size_t len)
{
	enum json_tokener_error err;
    char *message=NULL;
    json_object * jobj;
    const char *Action, *idTag;
    const char *UniqueId;
    char name[64];
    char ip[16];
    char *str;

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
        	if (debug) printf(AMARILLO"[Callback] Conexion establecida\n"RESET);
        	conexionEstablecida=1;
            break;

        //* Si recibimos datos del cliente. Aqui habra que poner todo el código del JSON*/
        case LWS_CALLBACK_RECEIVE:
        	jobj = json_tokener_parse_verbose((char *)in, &err);
        	if (jobj!=NULL)
        	{
        		json_object *obj_MessageTypeId;
        		json_object *obj_UniqueId;
        	    json_object *obj_Payload;
        	    json_object *obj_Action;

        	   // printf("B");
        		if (send_tags)
        		{
        		    obj_MessageTypeId = json_object_object_get(jobj, "MessageTypeId");
        		    obj_UniqueId = json_object_object_get(jobj, "UniqueId");
        		}
        		else
        		{
        			obj_MessageTypeId=json_object_array_get_idx(jobj, 0);
        			obj_UniqueId=json_object_array_get_idx(jobj, 1);
        		}

        		if (!obj_UniqueId||!obj_MessageTypeId) message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);

        		//Aqui solo entra si tenemos MessgeType y UniqueId
        		if (!message)
        		{
        		  //Obtenemos los valores reales
        		  int messageType=json_object_get_int(obj_MessageTypeId);
        		  UniqueId=json_object_get_string(obj_UniqueId);

//////////////////////////////////////////////////////////////////////////////////////////
        		  //Si hemos recibido una RESPUESTA....
        		  if (messageType==_CALLRESULT)
        		  {
       				if (send_tags) obj_Payload = json_object_object_get(jobj, "Payload");
       			    else obj_Payload=json_object_array_get_idx(jobj, 2);

        			if (debug) printf(MARRON"[Callback] Server recibio UN CALLRESULT:%s del wsi %x\n"RESET,(char *)in, wsi);

        			int fd = lws_get_socket_fd(wsi);
        			lws_get_peer_addresses(wsi, fd, name, 64, ip, 16);

        		    int action=getMessageIdFromUniqueId(UniqueId);

        		  //	if (debug) printf("ACTION encontrado en la cola ES: %d", action);
        		    //5.1
        		    if (action==CANCEL_RESERVATION)
        		    {
        		    	json_object *obj_status = json_object_object_get(obj_Payload, "status");
        		    	if (obj_status )
        		    	{
        		    		int status;
        		    		if (enums_as_integers)
        		    		{
        		    			status=json_object_get_int(obj_status);
        		    		}
        		    		else
        		    		{
        		    			const char *st=json_object_get_string(obj_status);
        		    			for (int i=0; i<sizeof(CancelReservationStatusTexts); i++)
        		    			{
        		    				if (strcmp(CancelReservationStatusTexts[i], st)==0)
        		    				{
        		    					status=i;
        		    				}
        		    			}
        		    		}

        		    		struct chargePoint *cp=NULL;
        		    		cp=getChargePointFromWSI(wsi);

        		    		if (status==_CR_ACCEPTED)
        		    		{
        		    			if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50101, AL_INFO, "Cancel Reservation REQ Replied: SUCCESS");
        		    			else LogInDb2(NULL,NULL,ip,NULL, 50101, AL_INFO, "Cancel Reservation REQ Replied: SUCCESS");
        		    		}
        		    		else
        		    		{
        		    			//Lo que no se es qué ocurre si esta rejected  <-- NOT IMPLEMENTED
        		    			//LOG REASON WHY IT'S BEEN REJECTED (_CP_RESERVATION_FAULTED,_CP_RESERVATION_OCCUPIED,_CP_RESERVATION_REJECTED,_CP_RESERVATION_UNAVAILABLE,)<-- NOT IMPLEMENTED.
        		    			if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50102, AL_WARNING, "Cancel Reservation REQ Replied: REJECTED");
        		    			else LogInDb2(NULL,NULL,ip, NULL, 50102, AL_WARNING, "Cancel Reservation REQ Replied: REJECTED");
        		    		}

        		    		//Leemos la peticion para saber cual es el reservationId
        		    		int reservationId=-1;

        		    		char *request=getPayloadFromMessage(atoi(UniqueId));
        		    		json_object *obj_req = json_tokener_parse_verbose(request, &err);

        		    		if(obj_req)
        		    		{
        		    			json_object *obj_reqpayl;

        		    			if (send_tags) obj_reqpayl=json_object_object_get(obj_req , "Payload");
        		    			else obj_reqpayl=json_object_array_get_idx(obj_req, 3);

        		    			json_object *obj_resId= json_object_object_get(obj_reqpayl, "reservationId");
        		    			reservationId=json_object_get_int(obj_resId);
        		    			free(obj_req);
        		    			free(obj_reqpayl);
        		    			free(obj_resId);
        		    		}
        		    		/////////////////////////

        		    		//En cualquier caso, eliminamos la reserva:
        		    		removeReservation(reservationId);

        		    		message=NULL;
        		    	}
        		    	else
        		    	{
        		    		message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
        		    	}
        		    }

        		    /////////////////////////////////////////////////////////////////////////////
        		    //5.2
        		    else if (action==CHANGE_AVAILABILITY)
           		    {
           		    	json_object *obj_status = json_object_object_get(obj_Payload, "status");
        		        if (obj_status )
        		        {
        		        		int status;
        		        	    if (enums_as_integers)
        		        	    {
        		        	    	status=json_object_get_int(obj_status);
        		        	    }
        		        	    else
        		        	    {
        		        	    	const char *st=json_object_get_string(obj_status);
        		        	        for (int i=0; i<sizeof(AvailabilityStatusTexts); i++)
        		        	        {
        		        	        	if (strcmp(AvailabilityStatusTexts[i], st)==0)
        		        	        	{
        		        	        		status=i;
        		        	        	}
        		        	        }
        		        	    }


        		        	    /////////////////////////////////////////////////////////////////////////////
        		        	    //No se explica lo que debe hacer el servidor ante esto. Por ahora, logamos.
        		        	    //Log Response

            		    		struct chargePoint *cp=NULL;
            		    		cp=getChargePointFromWSI(wsi);

        		        	    switch(status)
        		        	    {
        		        	    	case _CP_AVAILABILITY_ACCEPTED:
        		        	    		if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50201, AL_INFO, "Change Availability (5.2) REQ Replied: ACCEPTED");
        		        	    		else LogInDb2(NULL,NULL,NULL,ip, 50201, AL_INFO, "Change Availability (5.2) REQ Replied: ACCEPTED");
        		        	    		break;
        		        	    	case _CP_AVAILABILITY_REJECTED:
        		        	    		if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50202, AL_WARNING, "Change Availability (5.2) REQ Replied: REJECTED");
        		        	    		else LogInDb2(NULL,NULL,NULL,ip, 50202, AL_WARNING, "Change Availability (5.2) REQ Replied: REJECTED");
        		        	    		break;
        		        	    	case _CP_AVAILABILITY_SCHEDULED:
        		        	    		if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50203, AL_WARNING, "Change Availability (5.2) REQ Replied: SCHEDULED");
        		        	    		else LogInDb2(NULL,NULL,NULL,ip, 50203, AL_WARNING, "Change Availability (5.2) REQ Replied: SCHEDULED");
        		        	    		break;
        		        	    	default:
        		        	    		if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50204, AL_ERROR, "Change Availability (5.2) REQ Replied with Error");
        		        	    		else LogInDb2(NULL,NULL,NULL,ip, 50204, AL_ERROR, "Change Availability (5.2) REQ Replied with Error");
        		        	    }

        		        	    free(cp);
        		        	    message=NULL;
        		        }
        		        else
        		        {
        		        	message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
        		        }
           		    }

           		    /////////////////////////////////////////////////////////////////////////////
           		    //5.3
           		    else if (action==CHANGE_CONFIGURATION)
           		    {
           		       //Extraemos los campos de la respuesta
           		 	   json_object *obj_status = json_object_object_get(obj_Payload, "status");
           		       if (obj_status )
           		       {
           		    	int status;
           		        if (enums_as_integers)
           		        {
           		        	status=json_object_get_int(obj_status);
           		        }
           		        else
           		        {
           		        	const char *st=json_object_get_string(obj_status);
           		        	for (int i=0; i<sizeof(configurationStatus_texts); i++)
           		        	{
           		        		if (strcmp(configurationStatus_texts[i], st)==0)
           		        		{
           		        			status=i;
           		        		}
           		        	}
           		        }

           		        //Obtenemos el chargepoint para loguear
           				struct chargePoint *cp=NULL;
           				cp=getChargePointFromWSI(wsi);

           		        switch(status)
           		        {
           		        	case _CC_ACCEPTED:
		        	    		if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50301, AL_INFO, "Change Configuration Key (5.3) REQ Replied: ACCEPTED");
		        	    		else LogInDb2(NULL,NULL,NULL,ip, 50301, AL_INFO, "Change Configuration Key (5.3) REQ Replied: ACCEPTED");
           		            	break;
           		            case _CC_REJECTED:
		        	    		if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50302, AL_WARNING, "Change Configuration Key (5.3) REQ Replied: REJECTED");
		        	    		else LogInDb2(NULL,NULL,NULL,ip, 50302, AL_WARNING, "Change Configuration Key (5.3) REQ Replied: REJECTED");
           		            	break;
           		            case _CC_REBOOT_REQUIRED:
		        	    		if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50303, AL_WARNING, "Change Configuration Key (5.3) REQ Replied: REBOOT REQUIRED");
		        	    		else LogInDb2(NULL,NULL,NULL,ip, 50303, AL_WARNING, "Change Configuration Key (5.3) REQ Replied: REBOOT REQUIRED");

		        	    		char *cp_token=getChargePointStringFromWSI(wsi);
           		            	//Si necesita ser reseteado, se envia el mensaje de reset. NO SE SI ESTO DEBERIA HACERLO EL OPERADOR
           		            	//send_reset_request(cp_token, _RT_SOFT);
           		            	break;
           		            case _CC_NOTSUPPORTED:
           		            	if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50304, AL_ERROR, "Change Configuration Key (5.3) REQ Replied: NOT SUPPORTED");
           		            	else LogInDb2(NULL,NULL,NULL,ip, 50304, AL_ERROR, "Change Configuration Key (5.3) REQ Replied: NOT SUPPORTED");
           		                break;
           		            default:
           		            	if (cp) LogInDb2(cp->Model,cp->Vendor,ip, cp->SN, 50399, AL_ERROR, "Change Configuration Key (5.3) REQ Replied with Error");
           		            	else LogInDb2(NULL,NULL,NULL,ip, 50399, AL_ERROR, "Change Configuration Key (5.3) REQ Replied with Error");
           		        }
           		        message=NULL;
           		       }
           		       else
           		       {
           		     	message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
           		       }
           		    }

        		    /////////////////////////////////////////////////////////////////////////////
           		    //5.4
           		    else if (action==CLEAR_CACHE)
           		    {
           		    	//Extraemos los campos de la respuesta:
           		    	json_object *obj_status = json_object_object_get(obj_Payload, "status");
           		    	if (obj_status )
           		    	{
           		    		int status;
           		    		if (enums_as_integers)
           		    	    {
           		    	    	status=json_object_get_int(obj_status);
           		    	    }
           		    	    else
           		    	    {
           		    	    	const char *st=json_object_get_string(obj_status);
           		    	        for (int i=0; i<sizeof(clearCacheStatustexts); i++)
           		    	        {
           		    	        	if (strcmp(clearCacheStatustexts[i], st)==0)
           		    	        	{
           		    	        		status=i;
           		    	        	}
           		    	        }
           		    	    }

           		    		//Obtenemos el chargepoint para los logs
            		    	struct chargePoint *cp=NULL;
            		    	cp=getChargePointFromWSI(wsi);

            		    	if (cp) LogInDb(cp, 50401, AL_INFO, "Data Transfer (5.4) REQ Replied with ");
            		    	else LogInDb(NULL, 50401, AL_INFO, "Data Transfer (5.4) REQ Replied with ");

            		    	///No hacemos nada ante la respuesta:
            		    	message=NULL;
            		    }
            		    else
            		    {
            		    	message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
            		    }
            		}

        		    /////////////////////////////////////////////////////////////////////////////
        		    //5.5
        		    else if (action==CLEAR_CHARGING_PROFILE)
        		    {
        		    	json_object *obj_status = json_object_object_get(obj_Payload, "status");
        		        if (obj_status )
        		        {
        		        	int status;
        		            if (enums_as_integers)
        		            {
        		            	status=json_object_get_int(obj_status);
        		            }
        		            else
        		            {
        		            	const char *st=json_object_get_string(obj_status);
        		               	for (int i=0; i<sizeof(clearChargingProfileStatustexts); i++)
        		               	{
        		               		if (strcmp(clearChargingProfileStatustexts[i], st)==0)
        		               		{
        		               		   status=i;
        		               		 }
        		               	}
        		            }

        		            /////////////////////////////////////////////////////////
        		            //No se explica lo que debe hacer el servidor ante esto.
        		            //LogResponse
        		            struct chargePoint *cp=NULL;
        		            cp=getChargePointFromWSI(wsi);

        		            if (status==_CCPS_ACCEPTED)
        		            {
        		            	LogInDb(cp, 50501, AL_INFO, "Clear Charging Profile (5.5) REQ Replied: ACCEPTED");
        		            }
        		            else if (status==_CCPS_UNKNOWN)
        		            {
        		            	LogInDb(cp, 50502, AL_INFO, "Clear Charging Profile (5.5) REQ Replied: UNKNOWN CHARGING PROFILE");
        		            }
        		            else
        		            {
        		            	LogInDb(cp, 50599, AL_INFO, "Clear Charging Profile (5.5) REQ Replied with MESSAGE ERROR");
        		            }

        		            ///
        		            message=NULL;
        		      }
        		      else
        		      {
        		    	  	  message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
        		      }
        		    }
           		    /////////////////////////////////////////////////////////////////////////////
           		    //5.6
           		    else if (action==DATA_TRANSFER)
           		    {
						json_object *obj_status = json_object_object_get(obj_Payload, "status");
						if (obj_status )
						{
							int status;
							if (enums_as_integers)
							{
								status=json_object_get_int(obj_status);
							}
							else
							{
								const char *st=json_object_get_string(obj_status);
								for (int i=0; i<sizeof(dataTransferStatus_texts)/sizeof(dataTransferStatus_texts[0]); i++)
								{
									if (strcmp(dataTransferStatus_texts[i], st)==0)
									{
										status=i;
									}
								}
							}

							///////////////////////
							//EL SERVIDOR AL RECIBIR ESTA RESPUESTA simplemente Lo Logamos:
							 struct chargePoint *cp=NULL;
							 cp=getChargePointFromWSI(wsi);
							 LogInDb(cp, 50401, AL_INFO, "Data Transfer (5.6) REQ Replied with ");
           		    	}
           		    }


        		    /////////////////////////////////////////////////////////////////////////////
        		    //5.8
        		    else if (action==GET_CONFIGURATION)
        		    {
        		    	//Declaracion de variables
        		    	json_object *obj_elem, *obj_key, *obj_ro, *obj_value;
        		    	const char *key, *value;
        		    	KeyVal wrongkeys[CONFIGURATION_KEY_LIST_SIZE];
        		    	KeyVal validkeys[CONFIGURATION_KEY_LIST_SIZE];
        		    	for (int i=0; i<CONFIGURATION_KEY_LIST_SIZE; i++) validkeys[i].readonly=-1;

        		    	int found=0;

        		    	//Obtenemos las claves buenas
        		    	json_object *obj_confkeys = json_object_object_get(obj_Payload, "configurationKey");
        		        if (obj_confkeys )
        		        {
        		        	found=1;

        		            int num=json_object_array_length(obj_confkeys);

        		            //validkeys=(KeyVal *)calloc(num, sizeof(KeyVal));

        		            for (int i=0; i<num; i++)
        		            {
        		            	obj_elem=json_object_array_get_idx (obj_confkeys, i);
        		            	//const char *value=json_object_get_string(val);

        		            	//read values
        		            	obj_key=json_object_object_get (obj_elem, "key");
        		            	obj_value=json_object_object_get(obj_elem, "value");
        		            	obj_ro=json_object_object_get(obj_elem, "readonly");

        		            	key=json_object_get_string (obj_key);
        		            	value=json_object_get_string (obj_value);
        		            	validkeys[i].readonly=json_object_get_int(obj_ro);

        		            	strncpy(validkeys[i].key.x, key, 50);
        		            	strncpy(validkeys[i].value.x, value, 500);
        		            }
        		        }

        		        ///////////////////////////
        		        //Obtenemos las claves "malas"
        		        json_object *obj_badkeys = json_object_object_get(obj_Payload, "unknownKey");

        		        if (obj_badkeys)
        		        {
        		        	found=1;
        		        	int num=json_object_array_length(obj_confkeys);
        		        	//wrongkeys=(KeyVal *)calloc(num, sizeof(KeyVal));

        		        	for (int i=0; i<num; i++)
        		        	{
        		        		obj_elem=json_object_array_get_idx (obj_confkeys, i);
        		        		key=json_object_get_string (obj_elem);

        		        		strncpy(wrongkeys[i].key.x, key, sizeof(wrongkeys[i].key));
        		        		wrongkeys[i].value.x[0]='\0';
        		        		wrongkeys[i].readonly=-1;
        		        	}
        		        }
        		        //...aunque no se para que podría interesar tener esta información


        		        if (found)
        		        {
							/////////////////////////////////////////////////////////
							//LogResponse
							struct chargePoint *cp=NULL;
							cp=getChargePointFromWSI(wsi);

							//LogInDb(cp, 50801, AL_INFO, "Get Configuration (5.8) REQ Replied with ");

							LogInDb2(cp->Model,cp->Vendor,cp->IP,cp->SN, 50801, AL_INFO, "Get Configuration (5.8) REQ Replied with ");

							insertValidKeysInDb(cp, validkeys);
							//insertWrongKeysInDb(wrongkeys);
							message=NULL;
        		        }
        		        else
        		        {
        		        	//Si no llegan claves buenas ni claves malas, devuelve mensaje de error
        		        	message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
        		        }
        		    }
        		    /////////////////////////////////////////////////////////////////////////////
        		    //5.9
        		    else if (action==GET_DIAGNOSTICS)
        		    {
        		    	json_object *obj_filename = json_object_object_get(obj_Payload, "fileName");
        		    	struct chargePoint *cp=NULL;
        		    	cp=getChargePointFromWSI(wsi);
        		    	char *dbmsg=(char *)calloc(1,512);
        		    	strcpy(dbmsg, "GET DIAGNOSTICS (5.9) REQ Replied");
        		    	if (obj_filename)
        		    	{
        		    		const char *fn=json_object_get_string(obj_filename);

        		    		///////////////////////
        		    		//NO QUEDA CLARO LO QUE DEBE HACER EL SERVIDOR AL RECIBIR ESTA RESPUESTA
        		    		//Lo Logamos

        		    		strcat (dbmsg, "OK. Uploaded File: ");
        		    		strncat (dbmsg, fn, 400);
        		    		LogInDb(cp, 50901, AL_INFO, dbmsg);
        		    		manageDiagnosticsFile(fn);
        		    	}
        		    	else
        		    	{
        		    		strcat (dbmsg, "with Error. No filename found.");
        		    		LogInDb(cp, 50901, AL_ERROR, dbmsg);
        		    	}

        		    	free(dbmsg);
        		    }

        		    /////////////////////////////////////////////////////////////////////////////
        		    //5.10
        		    else if (action==GET_LOCAL_LIST_VERSION)
        		    {
        		    	//Lo mostramos por pantalla:
        		    	int version=-1;
        		    	struct chargePoint *cp=NULL;
        		    	cp=getChargePointFromWSI(wsi);

        		    	json_object *obj_listVersion = json_object_object_get(obj_Payload, "listVersion");
        		    	if (obj_listVersion)
        		    	{
        		    		version= json_object_get_int(obj_listVersion);

        		    		//LOG
        		    		char *text=(char *)calloc(1, 256);
        		    		sprintf(text, "GET LOCAL LIST VERSION (5.10) REQ Replied. ListVersion=%d.", version);
        		    		Log(text, cp, 51001, AL_INFO);
        		    		free(text);
        		    		updateListVersionInDb(cp, version);
        		    	}
        		    	else
        		    	{
        		    		Log("GET LOCAL LIST VERSION (5.10) REPLY ERROR.", cp, 51099, AL_ERROR);
        		    	}
        		    }

        		    /////////////////////////////////////////////////////////////////////////////
        		    //5.11
        		    else if (action==GET_LOCAL_LIST_VERSION)
        		    {
        		    	//Lo mostramos por pantalla:
        		        int status=-1;
        		        const char *status_str=NULL;

        		        struct chargePoint *cp=NULL;
        		        cp=getChargePointFromWSI(wsi);

        		        json_object *obj_status = json_object_object_get(obj_Payload, "status");
        		        if (obj_status)
        		        {
        		        	//LOG
        		        	char *text=(char *)calloc(1, 256);
        		        	if (enums_as_integers)
        		        	{
        		        		status=json_object_get_int(obj_status);
        		        	   	sprintf(text, "REMOTE START TRANSACTION (5.11) REQ Replied. status=%d.", status);
        		        	}
        		        	else
        		        	{
        		        		status_str=json_object_get_string(obj_status);
        		        		sprintf(text, "REMOTE START TRANSACTION (5.11) REQ Replied. status=%d.", status_str);
        		        	}

        		           	Log(text, cp, 51101, AL_INFO);
        		           	free(text);
        		        }
        		        else
        		        {
        		        	Log("REMOTE START TRANSACTION (5.11) REPLY ERROR.", cp, 51199, AL_ERROR);
        		        }
        		    }
        		    /////////////////////////////////////////////////////////////////////////////
        		    //5.13
        		    else if (action==RESERVE_NOW)
        		    {
        		    	json_object *obj_status = json_object_object_get(obj_Payload, "status");
        		    	if (obj_status )
        		    	{
        		    		int status;
        		    		if (enums_as_integers)
        		    		{
        		    			status=json_object_get_int(obj_status);
            		    		if (status==_CP_RESERVATION_ACCEPTED)
            		    		{
            		    			acceptLastPendingReservation(ip, _CP_RESERVATION_ACCEPTED);
            		    			LogInDb2(NULL,NULL,NULL,ip, 51301, AL_INFO, "Reserve Now REQ Replied: ACCEPTED");
            		    		}
            		    		else
            		    		{
            		    			//Si esta SCHEDULED tambien lo elimino, porque aun no esta reservado
            		    			removeLastPendingReservation(ip);
            		    		 	//LOG REASON WHY IT'S BEEN REJECTED (_CP_RESERVATION_FAULTED,_CP_RESERVATION_OCCUPIED,_CP_RESERVATION_REJECTED,_CP_RESERVATION_UNAVAILABLE,)<-- NOT IMPLEMENTED.
            		    			LogInDb2(NULL,NULL,NULL,ip, 51302, AL_WARNING, "Reserve Now REQ Replied: REJECTED");
            		    		}
        		    		}
        		    		else
        		    		{
        		    			const char *status=json_object_get_string(obj_status);
        		    			if (strcmp(status, ReservationStatus_texts[_CP_RESERVATION_ACCEPTED])==0)
        		    			{
        		    				acceptLastPendingReservation(ip, _CP_RESERVATION_ACCEPTED);
        		    				LogInDb2(NULL,NULL,NULL,ip, 51301, AL_INFO, "Reserve Now REQ Replied: ACCEPTED");
        		    			}
        		    			else
        		    			{
        		    				removeLastPendingReservation(ip);
        		    				//LOG REASON WHY IT'S BEEN REJECTED (_CP_RESERVATION_FAULTED,_CP_RESERVATION_OCCUPIED,_CP_RESERVATION_REJECTED,_CP_RESERVATION_UNAVAILABLE,)<-- NOT IMPLEMENTED.
        		    				LogInDb2(NULL,NULL,NULL,ip, 51302, AL_WARNING, "Reserve Now REQ Replied: REJECTED");
        		    			}
        		    		}
        		    		message=NULL;
        		    	}


        		    	 /////////////////////////////////////////////////////////////////////////////
        		    	 //5.15
        		    	 else if (action==SEND_LOCAL_LIST)
        		    	 {
        		    	 	json_object *obj_status = json_object_object_get(obj_Payload, "status");
        		    	    if (obj_status )
        		    	    {
        		    	    	int status;
        		    	        if (enums_as_integers)
        		    	        {
        		    	        	status=json_object_get_int(obj_status);
        		    	        }
        		    	        else
        		    	        {
        		    	        	const char *status_str=json_object_get_string(obj_status);
        		    	        	if (strcmp(status_str, ReservationStatus_texts[_CP_RESERVATION_ACCEPTED])==0) status=0;
        		    	        	if (strcmp(status_str, ReservationStatus_texts[_CP_RESERVATION_FAULTED])==0) status=1;
        		    	        	if (strcmp(status_str, ReservationStatus_texts[_CP_RESERVATION_OCCUPIED])==0) status=2;
        		    	        	if (strcmp(status_str, ReservationStatus_texts[_CP_RESERVATION_REJECTED])==0) status=3;
        		    	        	if (strcmp(status_str, ReservationStatus_texts[_CP_RESERVATION_UNAVAILABLE])==0) status=4;
        		    	        }

        		    	      	struct chargePoint *cp=NULL;
        		    	        cp=getChargePointFromWSI(wsi);
        		    	        char *request=getPayloadFromMessage(atoi(UniqueId));

        		    	        if (status==_UPS_ACCEPTED)
        		    	        {
        		    	            	LogInDb2(NULL,NULL,NULL,ip, 51501, AL_INFO, "Send Local List REQ Replied: ACCEPTED");
        		    	        }
        		    	        else
        		    	        {
        		    	            	//LOG REASON WHY IT'S BEEN REJECTED (_CP_RESERVATION_FAULTED,_CP_RESERVATION_OCCUPIED,_CP_RESERVATION_REJECTED,_CP_RESERVATION_UNAVAILABLE,)<-- NOT IMPLEMENTED.
        		    	            	LogInDb2(NULL,NULL,NULL,ip, 51502, AL_WARNING, "Send Local List REQ Replied: REJECTED");

        		    	            	//Pag 56. If the status is Failed or Version Mismatch and the updateType was Differential, then Central System should retry sending
        		    	            	//the full local authorization list with updateType FULL.

        		    	            	//Obtain updateType From Request

        		    	            	json_object *obj_req = json_tokener_parse_verbose(request, &err);
        		    	            	json_object *obj_reqpayl;
        		    	            	if (send_tags) obj_reqpayl=json_object_object_get(obj_req , "Payload");
        		    	            	else obj_reqpayl=json_object_array_get_idx(obj_req, 3);
        		    	            	json_object *obj_updateType= json_object_object_get(obj_reqpayl, "updateType");
        		    	            	int updateType=-1;
        		    	            	if (enums_as_integers)
        		    	            	{
        		    	            		updateType=json_object_get_int(obj_updateType);
        		    	            	}
        		    	            	else
        		    	            	{
        		    	            		char *updateType_str=json_object_get_string(obj_updateType);
        		    	            		if (updateType_str && strcmp(updateType_str,"Differential")==0) updateType=0;
        		    	            		if (updateType_str && strcmp(updateType_str,"Full")==0) updateType=1;
        		    	            	}
        		    	            	free(obj_req);
        		    	            	free(obj_reqpayl);
        		    	            	free(obj_updateType);
        		    	            	/////////

        		    	            	if ((status==_UPS_FAILED || status==_UPS_VERSIONMISMATCH) && updateType==_UT_DIFFERENTIAL)
        		    	            	{
        		    	            		char * in=replace(request,"updateType\":0","updateType\":1");
        		    	            		websocket_write_back(wsi ,message, -1);
        		    	            	}
        		    	            	else
        		    	            	{
        		    	            		if ((status==_UPS_FAILED || status==_UPS_VERSIONMISMATCH) && updateType==_UT_FULL)
        		    	            		{
        		    	            			//Como no sabemos que lista local tenia antes, la ponemos a NULL y le pedimos
        		    	            			//al chargepoint que nos mande la version

        		    	            			setAuthListToChargePoint(cp, NULL);
        		    	            			send_getlocallist_request(getChargePointString(cp));
        		    	            		}
        		    	            	}
        		    	           	}
        		    	        }

        		    	        message=NULL;
        		    	  }
        		    }
        		    else
        		    {
        		    	//if (debug) printf("Acción aun no implementada: %d", action);
        		    }

        		    //
        		    //Tras tratar el mensaje de entrada, lo eliminamos de la cola.
    		    	char *ptr;
    		    	long thisuniqueId= strtol(UniqueId, &ptr, 10);

    		    //	if (debug) printf("\n2.- lastsentaccepted es: %d y thisuniqueID es: %d", lastSentAcceptedUniqueId, thisuniqueId );

    		    	if (lastSentAcceptedUniqueId==thisuniqueId)
    		    	{
    		    		lastSentAcceptedUniqueId++;
    		    	    writable=1;
    		    	    //Dequeue_i(thisuniqueId);
    		    	    Dequeue();
    		    	}
        		  }
        		  //Si hemos recibido una PETICION...
        		  else if (messageType==_CALLMESSAGE)
        		  {
        			if (debug) printf(CYAN_L"[Callback] Server recibio un CALL MESSAGE:%s del wsi %x\n"RESET,(char *)in, wsi);

        			if (send_tags)
        			{
        			    //Comprobamos que es una respuesta
        			    obj_Action = json_object_object_get(jobj, "Action");
        			    obj_Payload = json_object_object_get(jobj, "Payload");
        			}
        			else
        			{
        			    obj_Action=json_object_array_get_idx(jobj, 2);
        			    obj_Payload=json_object_array_get_idx(jobj, 3);
        			}


        			if (obj_Action && obj_Payload)
        			{
         				Action=json_object_get_string(obj_Action);

        				struct chargePoint *cp=NULL;
        				cp=getChargePointFromWSI(wsi);

						//4.1- AUTHORIZE
						if (strcmp(Action, "Authorize") == 0)
						{
							json_object *obj_idTag = json_object_object_get(obj_Payload, "idTag");
							if (obj_idTag)
							{
								idTag=json_object_get_string(obj_idTag);
								//struct idTagInfo *info=authorizeIdTag(idTag);
								struct IdTagInfo *info=verifyIdentifierValidity(wsi, idTag);
								message=prepareAuthorizeResponse(obj_UniqueId, info);
							}
							else
							{
								char *message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
							}
						}
						//4.2 BOOT NOTIFICATION
						else if (strcmp(Action, "BootNotification") == 0)
						{
							int fd = lws_get_socket_fd(wsi);
							lws_get_peer_addresses(wsi, fd, name, 64, ip, 16);

							LogInDb(cp, 40201, AL_INFO, "Boot Notification received");
							message=respondBootNotification(obj_UniqueId, obj_Payload, ip, wsi);
						}
						//4.3
						else if (strcmp(Action, "DataTransfer") == 0)  ///SECCION 4.3
						{
							LogInDb(cp, 40301, AL_INFO, "Data Transfer received");
							message=prepareDataTransferResponse(obj_UniqueId, obj_Payload);
						}
						//4.4
						else if (strcmp(Action, "DiagnosticsStatusNotification") == 0)  ///SECCION 4.4
						{
							LogInDb(cp, 40401, AL_INFO, "Diagnostics Status Notification Request received from ChargePoint");
							message=prepareDiagnosticsStatusNotificationResponse(UniqueId);
						}
						//4.5
						else if (strcmp(Action, "FirmwareStatusNotification") == 0)  ///SECCION 4.5
						{
							LogInDb(cp, 40501, AL_INFO, "Firmware Status Notification Request received from ChargePoint");
							message=prepareFirmwareStatusNotificationResponse(UniqueId);
						}
						//4.6
						else if (strcmp(Action, "Heartbeat") == 0)  ///SECCION 4.6
						{
							message=prepareHeartbeatResponse(UniqueId);
						}
						//4.7
						else if (strcmp(Action, "MeterValues") == 0)  ///SECCION 4.7
						{
							LogInDb(cp, 40701, AL_INFO, "Meter Values Request received from ChargePoint");
							//
							json_object *obj_connectorId = json_object_object_get(obj_Payload, "connectorId");
							json_object *obj_meterValue = json_object_object_get(obj_Payload, "meterValue");
							json_object *obj_transactionId= json_object_object_get(obj_Payload, "transactionId");

							if (!obj_connectorId || ! obj_meterValue)
							{
								message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
							}
							else
							{
								int connectorId=json_object_get_int(obj_connectorId);
								int transactionId=-1;
								if (obj_transactionId) transactionId=json_object_get_int(obj_transactionId);

								message=manageMeterValues(UniqueId, connectorId, transactionId, obj_meterValue);
							}
						}
						//4.8
						else if (strcmp(Action, "StartTransaction") == 0)  ///SECCION 4.8
						{
							json_object *obj_idTag = json_object_object_get(obj_Payload, "idTag");
							json_object *obj_connectorId = json_object_object_get(obj_Payload, "connectorId");
							json_object *obj_meterStart = json_object_object_get(obj_Payload, "meterStart");
							json_object *obj_timeStamp= json_object_object_get(obj_Payload, "timestamp");
							int connector=0;
							if (obj_connectorId) connector=json_object_get_int(obj_connectorId);

							//Connector debe ser mayor que cero segun pone en pag 79.

							if (obj_idTag && obj_connectorId && obj_meterStart && obj_timeStamp && connector!=0)  //Campos obligatorios
							{
								idTag=json_object_get_string(obj_idTag);

								int meterStart=json_object_get_int(obj_meterStart);
								const char *timeStamp=json_object_get_string(obj_timeStamp);

								int reservationId=-1;
								json_object *obj_reservationId = json_object_object_get(obj_Payload, "reservationId"); //campo opcional
								if (obj_reservationId) reservationId=json_object_get_int(obj_reservationId);

								LogInDb(cp, 40801, AL_INFO, "Start Transaction Request received from ChargePoint");
								message=respondStartTransactionRequest(UniqueId, connector, idTag, meterStart, reservationId, timeStamp, wsi);
							}
							else
							{
								message=prepareErrorResponse(NULL, _PROTOCOL_ERROR);
							}
						}
						//4.9
						else if (strcmp(Action, "StatusNotification") == 0)  ///SECCION 4.9
						{
							//manageStatusNotification(obj_Payload); <-- NOT IMPLEMENTED
							LogInDb(cp, 40901, AL_INFO, "Status Notification Request received from ChargePoint");
							message=prepareStatusNotificationResponse(UniqueId);
						}
						//4.10
						else if (strcmp(Action, "StopTransaction") == 0)  ///SECCION 4.10
						{
							json_object *obj_meterStop = json_object_object_get(obj_Payload, "idTag");
							json_object *obj_timeStamp = json_object_object_get(obj_Payload, "timestamp");
							json_object *obj_transactionId = json_object_object_get(obj_Payload, "transactionId");
							json_object *obj_reason = json_object_object_get(obj_Payload, "reason");

							if (obj_meterStop && obj_timeStamp && obj_transactionId)
							{
									json_object *obj_idTag = json_object_object_get(obj_Payload, "idTag");
									json_object *obj_transactionData = json_object_object_get(obj_Payload, "transactionData");

									int meterStop=json_object_get_int(obj_meterStop);
									const char *timestamp=json_object_get_string(obj_timeStamp);
									int transactionId=json_object_get_int(obj_transactionId);

									//reason
									char *reason=NULL;
									if (obj_reason)
									{
										int reasonId=json_object_get_int(obj_reason);

										if (enums_as_integers)
										{
											reason=strdup(Stopping_Transaction_Reason_texts[reasonId]);
										}
										else
										{
											reason=json_object_get_string(obj_reason);
										}
									}

									//idtag
									if (obj_idTag) idTag=json_object_get_string(obj_idTag);
									else idTag=NULL;

									const char *transactionData=NULL;
									if (obj_transactionData) transactionData=json_object_get_string(obj_transactionData);

									//Upon receipt of a StopTransaction.req PDU, the Central System SHALL respond with a StopTransaction.conf PDU.
									LogInDb(cp, 41001, AL_INFO, "Stop Notification Request received from ChargePoint");
									message=respondStopTransactionRequest(UniqueId, meterStop, timestamp, transactionId, reason, idTag, transactionData);
							}
							else
							{
								message=prepareStopTransactionResponse(UniqueId, NULL);
							}
						}
						//Codigo de accion no valido
						else
						{
							//By default: echo server
							//message=(char *) in;
							message=prepareErrorResponse(obj_UniqueId, _GENERIC_ERROR);
						}
        			}
        			//O bien no hay payload o action o algun campo
        			else
        			{
        				message=prepareErrorResponse(obj_UniqueId, _GENERIC_ERROR);
        			}
				}//linea 888
				else//Otro tipo de mensaje, ni envio ni respuesta
				{
						  ///if (debug) addLog((char *)in, LOG_ERROR);

						  const char *UniqueId_str=json_object_get_string(obj_UniqueId);

						  char *ptr;
						  long thisuniqueId= strtol(UniqueId_str, &ptr, 10);

				//		  if (debug) printf("\nlastSentAcceptedUniqueId es %d y thisuniqueId es %d", lastSentAcceptedUniqueId, thisuniqueId);

						  if (thisuniqueId==0) //Si recibimos un mensaje de error procedente de un mensaje recbido sin ID, eliminamos el ultimo mensaje
						  {

						  }

							if (lastSentAcceptedUniqueId==thisuniqueId)
							{
								lastSentAcceptedUniqueId++;
								writable=1;
								Dequeue();
							}
							else
							{
								//RECORREMOS LA COLA Y ELIMINAMOS EL QUE ESTE MAL <--NO IMPLEMENTADO
								Dequeue();
							}

							if (debug) printf("Se recibio mensaje JSON con MessageTypeId diferente a 2 o 3. El mensaje JSON fue: %s. Devolvemos mensaje de error.", (char *)in);

							message=prepareErrorResponse(obj_UniqueId, _GENERIC_ERROR);
				}
         	  } //if (!message)
        	  else
        	  {
        		  //Aqui se llega enc aso de error, pero el mensaje lo habia preparado ya antes
        		  //message=prepareErrorResponse(obj_UniqueId, _GENERIC_ERROR);
        	  }

        	  //Finalmente mandamos el mensaje

        	  websocket_write_back(wsi ,message, -1);
        	}
        	else
        	{
        	//	printf("LLEGA: %s", (char *)in);
        		char *message=prepareErrorResponse(NULL, err);
        		websocket_write_back(wsi ,message, -1);
        	}

            break;
    case LWS_CALLBACK_SERVER_WRITEABLE :
    	 if ((Message_queue) && (conexionEstablecida))
    	 {
    		 queue_node *p=Message_queue;
    	  	 while (p && p->sent) p=p->next;

    		 if (p)
    		 {
    			 char *este_message=p->payload;
    			 struct lws *este_wsi=p->wsi;
    			 websocket_write_to_client(este_wsi ,este_message, -1);
    			 p->sent=1;
    	        		//lws_write( wsi, &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], received_payload.len, LWS_WRITE_TEXT );
    	        		//lws_write(wsi ,message, strlen(message), LWS_WRITE_TEXT);

    		 }
    	}
    	 break;
    case LWS_CALLBACK_CLOSED:
    		conexionEstablecida=0;

    		///////////////////////////////////////////////////////////
    		//Tenemos que cerrar las transacciones de este chargepoint:
    		int i;
    		struct chargePoint *cp=NULL;
    		cp=getChargePointFromWSI(wsi);

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
    		if (i<MAX_NUM_CHARGEPOINTS)
    		{
    				struct transaction *t=NULL;
    				struct transaction *ant=NULL;
    				t=currentTransactions[i];
    				while (t)
    				{
    					ant=t;
    					t=t->next;
    					free(ant);
    				}

    		    	currentTransactions[i]=NULL;
    		}

    		if (debug) printf(AMARILLO"[Callback] Cierre del cliente.\n"RESET);

    		break;

    default:
            break;
    }

    return 0;
}


struct per_session_data {
    int fd;
};


static struct lws_protocols protocols[] = {
	/* first protocol must always be HTTP handler */
	{
		"OCPP_Protocol",		/* name */
		service_callback,		/* callback */
		4096,					/* per_session_data_size */
		4096,					/* max frame size / rx buffer */
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

int main(void)
{
	/////////////////////////////
	//INITIALIZATION
	if (ini_parse("/etc/ocpp/ocpp_server.ini", handler, NULL) < 0) {
	   printf("Can't load '/etc/ocpp/ocpp_server.ini'\n");
	   return 1;
	}

	for (int i=0; i<64; i++){
	   	reservationList[i]=NULL;
	   	connectorId[i]=NULL;
	}

	middleware_initialize();
	Message_queue=NULL;
	chargingProfileInitialize();

	///////////////////////////////
	//DATABASE

	connectDB(getConfigurationKeyStringValue("DatabaseServer"), getConfigurationKeyStringValue("DatabaseUser"), getConfigurationKeyStringValue("DatabasePassword"));

	if (conn)
	{
	//	printf("SE CONECTA  A LA BD");
		readChargePointModelsFromDB();
		readVendorIDsFromDB();
		readAuthorizationListsFromDB();
		readChargingProfilesFromDB();
	}
	else if (connectFiles())
	{
		//printf("LEE DE FICHEROS");
		if (readChargePointModelsFromFile()==-1)
		{
		   	printf("Error al cargar el fichero de modelos de charge points");
		   	exit(3);
		}
	}
	else
	{
		exit(4);
	}

	///////////////////////////////
	//WEBSOCKETS
	const char *interface = NULL;
    struct lws_context_creation_info info;
    struct lws_protocols protocol;
    struct lws_context *context;
    struct lws *wsi = NULL;
    struct lws_vhost *vhost;

    // Modificar esto cuando usemos SSL
    const char *cert_path = NULL;
    const char *key_path = NULL;
    char ca_path[1024] = "";
    int uid = -1, gid = -1;
    int opts = 0;

    // no special options

    struct sigaction act;
    act.sa_handler = INT_HANDLER;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction( SIGINT, &act, 0);

    //* setup de websocket protocol. Le damos un nombre y le decimos que se debe ejecutar cuando reciba una solicitud */

    protocol.name = "OCPP_Protocol";
    protocol.callback = service_callback;
    protocol.per_session_data_size=sizeof(struct per_session_data);
    protocol.rx_buffer_size = 0;

    //* Informacion de contexto del websocket*/
    memset(&info, 0, sizeof info);
    info.port = PORT; //Puerto de escucha
    info.iface = interface;
    info.protocols = &protocol;

    //Esto esta deprecado pero no se otra forma de hacerlo

    info.extensions = lws_get_internal_extensions();

	//info.ssl_cert_filepath = "/root/eclipse-workspace/example-server/Debug/cert.pem";
	//info.ssl_private_key_filepath = "/root/eclipse-workspace/example-server/Debug/key.pem";  //Fichero .key
	info.ssl_cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:"
				       "ECDHE-RSA-AES256-GCM-SHA384:"
				       "DHE-RSA-AES256-GCM-SHA384:"
				       "ECDHE-RSA-AES256-SHA384:"
				       "HIGH:!aNULL:!eNULL:!EXPORT:"
				       "!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:"
				       "!SHA1:!DHE-RSA-AES128-GCM-SHA256:"
				       "!DHE-RSA-AES128-SHA256:"
				       "!AES128-GCM-SHA256:"
				       "!AES128-SHA256:"
				       "!DHE-RSA-AES256-SHA256:"
				       "!AES256-GCM-SHA384:"
						"!AES256-SHA256";
    info.gid = uid;
    info.uid = gid;
    info.options=opts;
    info.max_http_header_pool = 256;
    info.options = opts | LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
    info.options |=LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT|LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;

    ////////////////////////////////////////////////////////////////////////////
    //* Creamos el contexto websocket*/
    context = lws_create_context(&info);
    if (context == NULL) {
    	if (debug) printf(ROJO"[Main] Error en la creacion del contexto Websocket. Please check if there's another process using port 5000. Asegurarse que memset esta puesto\n"RESET);
        return -1;
    }

    vhost = lws_create_vhost(context, &info);

	//Finalmente creamos el hilo del GUI
	gtk_init(1, NULL);
    pthread_t pidGUI;
    pthread_create(&pidGUI, NULL, drawGUI, NULL);
    pthread_detach(pidGUI);

    //////////////////////////////////////////////////////////////////////////////
    //* Comprueba cada 300ms si debe enviar o recibir algo*/


   // printf("LA ULTIMA TRANS ES: %d", getLastTransactionId());

    while ( !destroy_flag ) {
        lws_service(context, 300);
        lws_callback_on_writable_all_protocol(context,&protocols[0]);
    }

    //Al salir, duerme 10ms y destruye el contexto
    usleep(10);
 //   lws_context_destroy(context);

    return 0;
}


