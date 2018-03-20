/*
 * aux.h
 *
 *  Created on: Nov 9, 2017
 *      Author: root
 */

#ifndef AUX_H_
#define AUX_H_
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mysql/mysql.h>

#define MAX_NUM_CHARGEPOINTS 256
#define MAX_NUM_CHARGINGPROFILES 256

static int debug=1;
static int mutex=0;
static int UniqueId=700000;
static int chargingProfileId=30000;
static int lastSentAcceptedUniqueId=700000;

typedef struct {
		int Messagetype;
		int UniqueId;
		char *payload;
		int sent;
		struct lws *wsi;
		struct queue_node *next;
}queue_node;

typedef struct CiString25Type{
	char x[25];
} CiString25Type;

typedef struct CiString20Type{
	char x[20];
} CiString20Type;

typedef struct CiString50Type{
	char x[50];
} CiString50Type;

typedef struct CiString500Type{
	char x[500];
} CiString500Type;

//Database connection
MYSQL *conn;

//This is the message queue
queue_node *Message_queue;
void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);
int charCounter(char* pString, char c);
char** str_split(char* a_str, const char a_delim);
char *trimwhitespace(char *str);
char *strlwr(char * s);
char *strlwr_ex(char * s);
void Dequeue();
void Dequeue_i(int uniqueId);
void Enqueue(queue_node *new_node);
char *convert(int i);
char *convertF(float myFloat);
int getMessageIdFromUniqueId(const char* UniqueId_str);
char *getCurrentTime();
char *getExpiryTime(int n, char type);
char *getPayloadFromMessage(int uniqueId);
char * replace(char const * const original,char const * const pattern,char const * const replacement);

char *encrypt(char *text, int last);
char *decrypt(char *text, int last);

//Devuelve el siguiente UniqueId siempre que se haya recibido su respuesta
int getNextUniqueID();
char *getNextUniqueID_char();
char *getRandomString(int length, int mins, int mays, int nums);

char *encrypt(char *text, int last);
char *decrypt(char *text, int last);

#endif /* AUX_H_ */
