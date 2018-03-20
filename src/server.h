/*
 * client.h
 *
 *  Created on: Nov 8, 2017
 *      Author: root
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>

#include "ocpp_server_gtk.h"
#include "aux.h"

#define _XOPEN_SOURCE 500


#define VERDE "\033[0;32;32m"
#define CYAN "\033[0;36m"
#define ROJO "\033[0;32;31m"
#define AMARILLO "\033[1;33m"
#define AZUL "\033[0;32;34m"
#define CYAN_L "\033[1;36m"
#define MARRON "\033[0;33m"
#define RESET "\033[0m"

#define RED 0
#define GREEN 1
#define BLUE 2
#define BLACK 4
#define BROWN 5

#define _CALLMESSAGE 2
#define _CALLRESULT 3
#define _CALLERROR 4


static int authorizationCache=0;
static int conexionEstablecida=0;
static int writable=0;

static int service_callback(struct lws *wsi,enum lws_callback_reasons reason, void *user,void *in, size_t len);
//Auxiliar functions

int checkValidChargePoint(json_object *Payload);
int checkWellKnownVendorId(const char *vendorId); //Ver Pag 36
int checkMessageId(const char *messageId); //Ver Pag 36
void Log(char *text, struct chargePoint *cp, int error_code, int error_level);
struct IdTagInfo *authorizeIdTag(const char* idTag);

#endif /* SERVER_H_ */
