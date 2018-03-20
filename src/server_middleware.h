/*
 * middleware.h
 *
 *  Created on: Nov 21, 2017
 *      Author: root
 */

#ifndef SERVER_MIDDLEWARE_H_
#define SERVER_MIDDLEWARE_H_

#include <time.h>
#include "ocpp_server.h"
#include "chargingProfile.h"
#include "authorizationLists.h"
//#include "data_store.h"
#include "aux.h"
//#include "ocpp_server_gtk.h"
//#include "server.h"

#define CONFIGURATION_KEY_LIST_SIZE 70
#define SERVER_CONFIGURATION_KEY_LIST_SIZE 36

/////////////////////////////////
//   CONFIGURATION
/////////////////////////////////
struct configurationKey{
	char *keyValue;
	char *stringValue;
};

static char *clientConfigurationKeys[CONFIGURATION_KEY_LIST_SIZE]={
				//9.1 CORE PROFILE
				"AllowOfflineTxForUnknownId","AuthorizationCacheEnabled","AuthorizeRemoteTxRequests","BlinkRepeat","ClockAlignedDataInterval",
				"ConnectionTimeOut","GetConfigurationMaxKeys","HeartbeatInterval","LightIntensity","LocalAuthorizeOffline","LocalPreAuthorize","MaxEnergyOnInvalidId",
				"MeterValuesAlignedData", "MeterValuesAlignedDataMaxLength", "MeterValuesSampledData", "MeterValuesSampledDataMaxLength", "MeterValuesSampleInterval", "MinimumStatusDuration",
				"NumberOfConnectors","ResetRetries","ConnectorPhaseRotation","ConnectorPhaseRotationMaxLength","StopTransactionOnEVSideDisconnect","StopTransactionOnInvalidId",
				"StopTxnAlignedData","StopTxnAlignedDataMaxLength","StopTxnSampledData","StopTxnSampledDataMaxLength","SupportedFeatureProfiles","SupportedFeatureProfilesMaxLength",
				"TransactionMessageAttempts","TransactionMessageRetryInterval","UnlockConnectorOnEVSideDisconnect","WebSocketPingInterval",
				//9.2 LOCAL AUTH LIST MANAGEMENT PROFILE
				"LocalAuthListEnabled","LocalAuthListMaxLength","SendLocalListMaxLength",
				//9.3 RESERVATION PROFILE
				"ReserveConnectorZeroSupported",
				//9.4 SMART CHARGING PROFILE
				"ChargeProfileMaxStackLevel","ChargingScheduleAllowedChargingRateUnit","ChargingScheduleMaxPeriods","ConnectorSwitch3to1PhaseSupported","MaxChargingProfilesInstalled",
				//THIS IMPLEMENTATION SPECIFIC CONFIGURATION KEYS
				"ProtocolName", "CentralSystemIP", "CentralSystemPort", "CentralSystemURL",  "MaxDataTransferBytes", "ProtocolVersion","ChargePointModel","ChargePointVendor",
				"ChargePointSerialNumber","ChargeBoxSerialNumber", "FirmwareVersion", "ChargePointICCID", "ChargePointIMSI", "ChargePointMeterType", "ChargePointMeterSerialNumber",
				"ChargeBoxIP","ChargeBoxPort","ChargeBoxURL","ChargeBoxID","LocalAuthListFile", "LocalAuthorizationCacheFile","ChargePointVendorID", "PermanentAttachment",
				"messageIdList","vendorIdList",
				//NOT USED
				NULL,NULL
		};

////////////////////////////////////////////////////////////////////////////////
//ESTO DEBERIA IR EN BASE DE DATOS.
//POR FALTA DE TIEMPO, COPIO-PEGO DEL CODIGO DEL CHARGEPOINT
//
//If n%10 is 3 the key RW
//If n%10 is 2 the key is Read Only
//A value over 50 means that it's a boolean (int)
//A value over 10 means that it's a string
//A value over 20 means that it's a CSL
//A value over 1000 means that reboot is required on change
//A value over 100 means that the value is optional
static int configurationKeyCharacteristics[CONFIGURATION_KEY_LIST_SIZE]={
		//9.1
		153,153,53,103,3,3,2,3 ,103,53,53,103,23,102,23,102 ,3,103,2,3,23,102,53,53 ,23,102,23,102,22,102,3,3, 53,103,
		//9.2
		53,2,2,
		//9.3
		152,
		//9.4
		2,22,2,152,2,
		//SPECIFIC
		1013,1013,1013,1013,13,1013,12,12,12,12,12,12,12,12,12,12,12,12,12,1113,1113,12,102,
		//NOT USED
		123,123,-1,-1
};
///////////////////////////////////////////////////////////////////////

struct configurationKey configurationValues[SERVER_CONFIGURATION_KEY_LIST_SIZE];
static char *configurationKeys[SERVER_CONFIGURATION_KEY_LIST_SIZE]={
		"ProtocolName", "CentralSystemIP", "CentralSystemPort", "CentralSystemURL",  "MaxDataTransferBytes", "ProtocolVersion","ChargePointModel","ChargePointVendor",
		"ChargePointSerialNumber","ChargeBoxSerialNumber", "FirmwareVersion", "ChargePointICCID", "ChargePointIMSI", "ChargePointMeterType", "ChargePointMeterSerialNumber",
		"ChargeBoxIP","ChargeBoxPort","ChargeBoxURL","ChargeBoxID","LocalAuthListFile", "LocalAuthorizationCacheFile","ChargePointVendorID","DatabaseServer", "DatabasePort", "DatabaseUser", "DatabasePassword",
		"HeartbeatInterval","LogAlarmLevel",
		//NOT USED
		NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};

typedef struct chargePointModel{
	CiString20Type chargePointVendor;
	CiString20Type chargePointModel;
	int numConnectors;
};

struct chargePoint{
	char *IP; //obligatorio
	char *SN; //opcional
	char *Model; //obligatorio
	char *Vendor; //obligatorio
	struct lws *wsi; //obligatorio
};

//This will be loaded on init
struct chargePointModel allowedChargePointModels[MAX_NUM_CHARGEPOINTS];

//This array stores the different connected charge points
struct chargePoint connected_ChargePoints[MAX_NUM_CHARGEPOINTS];

//Valor de chargepoints conectados actualmente al sistema central. Admite
//como maximo 256. Este valor de 256 se puede cambiar en el DEFINE MAX_NUM_CHARGEPOINTS
int numCurrentlyConnected;

/////////////////////////////////////
// VENDOR IDs
//
#define MAXVENDORS 256
int currentnumvendors;
struct vendorIdData{
	char *vendor;
	char *vendorId;
};

struct vendorIdData *vendorIds[MAXVENDORS];
char *getVendorId(char *vendor);
void setVendorId(char *vendor, char *vendorId);

/////////////////////////////////////
//3.9 GROUPS

typedef struct idTagRecord{
	char *idTag;
	struct idTagRecord *next;
};

typedef struct parentIdTagRecord{
	char *parentIdTag;
	struct idTagRecord *idTags;
	struct parentIdTagRecord *next;
} parentIdTagRecord;

static struct parentIdTagRecord *idTagGroups;

/////////////////////////////////////
//3.10 RESERVATIONS
//
struct reservationsList{
	char *idTag;
	char *Vendor;
	char *Model;
	char *IP;
	int Connector;
	char *expiryDate;
	int status;
	int reservation_id;
	struct reservationsList *next;
};

//Accepted reservations
struct reservationsList *reservations;

//Reservations pending to be accepted
struct reservationsList *pending_reservations;

int numReservations;
int numReservationId;

/////////////////////////////////////
//TRANSACTIONS
//
int transactionId; //<--This is the number of the next TransactionId requested

struct transaction{
	struct chargePoint *CP;
	int connectorId;
	int transactionId;
	int transactionStartValue;
	int transactionEndValue;
	struct transaction *next;
};

struct transaction *currentTransactions[MAX_NUM_CHARGEPOINTS];

/////////////////////////////////////
//AUXILIAR FUNCTIONS
//
int ChargePointNeedsChargingProfile();

//CONFIGURATION KEYS
int getConfigurationKeyType(char *key);
char* getConfigurationKeyStringValue(char *key);
int getConfigurationKeyIntValue(char *key);
int modifyConfigurationKey(char *key, char *value);
int containsCSL(char *key, const char *value);

//WSI
struct lws *getwsifromChargePointString(char *chargePoint);
struct chargePoint *getChargePointStructFromWSI(struct lws *wsi);
char *getChargePointStringFromWSI(struct lws *wsi);
struct chargePoint *getChargePointFromWSI(struct lws *wsi);
struct chargePoint *getChargePointFromString(char *chargePoint);

//Diagnostics file
void manageDiagnosticsFile(char *fn);

//INITIALIZATION
void middleware_initialize();
queue_node * create_empty_queue_node(int messageType, const char *chargePoint);

struct IdTagInfo *verifyIdentifierValidity(struct lws *wsi_in, const char* idTag);

//////
//CHARGEPOINTS & CONNECTORS
int addConnectedChargePoint(json_object *Payload, struct lws *wsi);
char *getChargePointString(struct chargePoint *cp);
int checkValidChargePoint(json_object *Payload);
int getConnectorNumber(char *chargePointVendor, char *chargePointModel);

//////
//GROUPS
int addIdtagtoGroup( char *parentIdtag, char *IdTag);
int removeIdtagFromGroup(char *parentIdtag, char *IdTag);
struct idTagRecord* getIdTagsFromGroup( char *parentIdtag);
int isIdTagsInGroup( char *parentIdtag, char *IdTag);
int addParentIdTag(char *parentIdtag, char *IdTag);
int removeParentIdTag(char *parentIdtag);

//////
//RESERVATIONS
char *getReservation(int i);
int getReservationID(int i);
int getNextReservationId();
void reserve(char *IP, int status);
int addPendingReservation(char *vendor, char *model, char *IP, int Connector, int id, char *expiryDate, char *idTag);
void checkReservationExpirations();
void acceptLastPendingReservation(char *IP, int status);
int removeLastPendingReservation(char *IP);
void removeReservation(int reservationId);

//////
//TRANSACTIONS
int addTransaction(struct chargePoint *cp, int transactionId,  int connector, int meterStart);
int removeTransaction(int transactionId);

//////////////////////////////////
//4.2
char *respondBootNotification(json_object * obj_UniqueId, json_object * Payload, char ip[16], struct lws *wsi);

//4.7
char *manageMeterValues(const char *UniqueId, int connectorId, int transactionId, json_object *obj_meterValue);

//4.8
char *respondStartTransactionRequest(const char *UniqueId, int connector, const char *idTag, int meterStart, int reservationId, const char *timeStamp, struct lws *wsi);

//4.10
char *respondStopTransactionRequest(const char *UniqueId, int meterStop, char *timestamp, int transactionId, char *reason, char *idTag, char *transactionData);

//5.1
void send_cancel_reservation_request(const char *chargepoint, const char *reservationID);

//5.2
void send_changeAvailability_request(const char *chargepoint, int connector, int new_availability);

//5.3
void send_change_configuration_request(char *chargePoint, char *key, char *value);

//5.4
void send_clearcache_request(const char *chargePoint);

//5.5
void send_clearchargingprofile_request(const char *chargePoint, const char *chargingprofileid, int connectorid, int purpose, int stacklevel);

//5.6
void send_data_transfer_request(const char *chargePoint, char *messageId, char *data);

//5.8
void send_getconfiguration_request(const char *chargePoint, char *key);

//5.9
void send_getdiagnostics_request(const char *chargePoint, const char *location, const char *retries_str, const char *retryInterval_str, const char *startTime, const char *stopTime);

//5.10
void send_getlocallist_request(char *chargePoint);

//5.11
void send_remotestarttransaction_request(const char *chargePoint, const char *idTag, const char *connectorId_str, const char *stacklevel_str, const int purpose, const int kind, const int recurrency, const char *validfrom, const char *validto, int chargingProfile, const char *duration_str, const char *startSchedule, int chargingRateUnit, struct ChargingSchedulePeriod periodos[], int num_periods, const char *minChargingRate);

//5.12
void send_remotestoptransaction_request(const char *chargePoint, const char* transactionId);

//5.13
void send_reservenow_request(char *chargePoint, int connector, char *idTag, char *reservationText, char *parentIdTag);

//5.14
void send_reset_request(const char *chargePoint, int resetType);

//5.15
void send_locallist_request(const char *chargePoint, const char *listName, int type);

//5.16
void send_setchargingprofile_request(const char *chargePoint, int connector, const char *transactionId, const char *chargingProfile);

//5.17
void send_messagetrigger_request(const char *chargePoint, int connector, int messageID);

//5.18
void send_unlockconnector_request(const char *chargePoint, int connector);

//5.19
void send_firmwareupdate_request(const char *chargePoint, const char *location, const char *retrieveDate, const char *retries_str, const char *retryInterval_str);


#endif /* SERVER_MIDDLEWARE_H_ */
