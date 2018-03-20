/*
 * ocpp_client.h
 *
 *  Created on: Nov 8, 2017
 *      Author: root
 */

#ifndef OCPP_SERVER_H_
#define OCPP_SERVER_H_

//#include "aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include "aux.h"
#include "chargingProfile.h"
#include "server_middleware.h"
#include "authorizationLists.h"

//Pag 10
enum messageTypes{
	_CALLMESSAGE=2,
	_CALLRESULT=3,
	_CALLERROR=4
};

//Pag 13
enum errorCodes{
	_NOT_IMPLEMENTED,
	_NOT_SUPPORTED,
	_INTERNAL_ERROR,
	_PROTOCOL_ERROR,
	_SECURITY_ERROR,
	_FORMATION_VIOLATION,
	_PROPERTY_CONSTRAINT_VIOLATION,
	_OCCURENCE_CONSTRAINT_VIOLATION,
	_TYPE_CONSTRAINT_VIOLATION,
	_GENERIC_ERROR
};

#define _PORT 5000

int _REGISTRATIONSTATUS;

//MESSAGE ACTIONS
enum action_IDs{
	AUTHORIZE=401,
	BOOT_NOTIFICATION=402,
	DATA_TRANSFER=403,
	DIAGNOSTICS_STATUS_NOTIFICATION=404,
	FIRMWARE_STATUS_NOTIFICATION=405,
	HEARTBEAT=406,
	METER_VALUES=407,
	START_TRANSACTION=408,
	STATUS_NOTIFICATION=409,
	STOP_TRANSACTION=410,
	CANCEL_RESERVATION=501,
	CHANGE_AVAILABILITY=502,
	CHANGE_CONFIGURATION=503,
	CLEAR_CACHE=504,
	CLEAR_CHARGING_PROFILE=505,
	CS_DATA_TRANSFER=506,
	GET_COMPOSITE_SCHEDULE=507,
	GET_CONFIGURATION=508,
	GET_DIAGNOSTICS=509,
	GET_LOCAL_LIST_VERSION=510,
	REMOTE_START_TRANSACTION=511,
	REMOTE_STOP_TRANSACTION=512,
	RESERVE_NOW=513,
	RESET_MSG=514,
	SEND_LOCAL_LIST=515,
	SET_CHARGING_PROFILE=516,
	TRIGGER_MESSAGE=517,
	UNLOCK_CONNECTOR=518,
	UPDATE_FIRMWARE=519,
	ERROR_MESSAGE=601
};

static const char* ActionTexts[] = {
		"Authorize", "BootNotification", "ChangeAvailability", "ChangeConfiguration", "ClearCache","DataTransfer","GetConfiguration",
		"Heartbeat","MeterValues","RemoteStartTransaction","RemoteStopTransaction","Reset","StartTransaction","StatusNotification",
		"StopTransaction","UnlockConnector","GetDiagnostics","DiagnosticsStatusNotification","FirmwareStatusNotification",
		"UpdateFirmware","GetLocalListVersion","SendLocalListVersion","CancelReservation","ReserveNow","ClearChargingProfile",
		"GetCompositeSchedule","SetChargingProfile","TriggerMessage"
};

#define _PORT 5000

///////////////////////////////////////////////////////////////
//CHARGEPOINT Authorization. Ver 7.2
enum AuthorizationStatus{
	_CP_AUTHORIZATION_ACCEPTED,
	_CP_AUTHORIZATION_BLOCKED,
	_CP_AUTHORIZATION_EXPIRED,
	_CP_AUTHORIZATION_INVALID,
	_CP_AUTHORIZATION_CONCURRENT_TX
};

static const char* AuthorizationStatusTexts[] = {
		"Accepted", "Blocked", "Expired", "Invalid", "ConcurrentTx"
};

///////////////////////////////////////////////////////////////
//CHARGEPOINT Availability. Ver 7.3
enum AvailabilityStatus{
	_CP_AVAILABILITY_ACCEPTED,
	_CP_AVAILABILITY_REJECTED,
	_CP_AVAILABILITY_SCHEDULED,
};

static const char* AvailabilityStatusTexts[] = {
		"Accepted", "Rejected", "Scheduled"
};

///////////////////////////////////////////////////////////////
//CHARGEPOINT ERRORCODE. Ver 7.4
enum availabilityType{
	_CP_AVAILABILITY_INOPERATIVE,
	_CP_AVAILABILITY_OPERATIVE
};

static const char* availabilityTypeTexts[] = {
		"Inoperative", "Operative"
};

///////////////////////////////////////////////////////////////
//CANCEL RESERVATION STATUS. Ver 7.5
enum CancelReservationStatus{
	_CR_ACCEPTED,
	_CR_REJECTED
};

static const char* CancelReservationStatusTexts[] = {
		"Accepted", "Rejected"
};

///////////////////////////////////////////////////////////////
//CHARGEPOINT ERRORCODE. Ver 7.6
enum ChargePointErrorCode{
	_CP_ERROR_CONNECTORLOCKFAILURE,
	_CP_ERROR_GROUNDFAILURE,
	_CP_ERROR_HIGHTEMPERATURE,
	_CP_ERROR_INTERNALERROR,
	_CP_ERROR_LOCALLISTCONFLICT,
	_CP_ERROR_NOERROR,
	_CP_ERROR_OTHERERROR,
	_CP_ERROR_OVERCURRENTFAILURE,
	_CP_ERROR_OVERVOLTAGE,
	_CP_ERROR_POWERMETERFAILURE,
	_CP_ERROR_POWERSWITCHFAILURE,
	_CP_ERROR_READERFAILURE,
	_CP_ERROR_RESETFAILURE,
	_CP_ERROR_UNDERVOLTAGE,
	_CP_ERROR_WEAKSIGNAL
};

static const char* ChargePointErrorCodeTexts[] = {
		"ConnectorLockFailure",
		"GroundFailure",
		"HighTemperature",
		"InternalError",
		"LocalListConflict",
		"NoError",
		"OtherError",
		"OverCurrentFailure",
		"OverVoltage",
		"PowerMeterFailure",
		"PowerSwitchFailure",
		"ReaderFailure",
		"ResetFailure",
		"UnderVoltage",
		"WeakSignal",
};

///////////////////////////////////////////////////////////////
//CHARGEPOINT STATUS. Ver 7.7
enum ChargePointStatus{
	_CP_STATUS_AVAILABLE,
	_CP_STATUS_PREPARING,
	_CP_STATUS_CHARGING,
	_CP_STATUS_SUSPENDEDEVSE,
	_CP_STATUS_SUSPENDEDEV,
	_CP_STATUS_FINISHING,
	_CP_STATUS_RESERVED,
	_CP_STATUS_UNAVAILABLE,
	_CP_STATUS_FAULTED
};

static const char* ChargePointStatusTexts[] = {
"Available",
"Preparing",
"Charging",
"SuspendedEVSE",
"SuspendedEV",
"Finishing",
"Reserved",
"Unavailable",
"Faulted"
};

///////////////////////////////////////////////////
//7.20
enum clearCacheStatus{
	_CCS_ACCEPTED,
	_CCS_REJECTED
};

static const char* clearCacheStatustexts[] = {
	"Accepted",
	"Rejected",
};

///////////////////////////////////////////////////
//7.21
enum clearChargingProfileStatus{
	_CCPS_ACCEPTED,
	_CCPS_UNKNOWN
};

static const char* clearChargingProfileStatustexts[] = {
	"Accepted",
	"Unknown",
};

///////////////////////////////////////////////////
//7.22
enum configurationStatus{
	_CC_ACCEPTED,
	_CC_REJECTED,
	_CC_REBOOT_REQUIRED,
	_CC_NOTSUPPORTED,
};

static const char * configurationStatus_texts[]={
	"Accepted",
	"Rejected",
	"RebootRequired",
	"NotSupported"
};

///////////////////////////////////////////////////
//7.23
enum dataTransferStatus{
	_DT_ACCEPTED,
	_DT_REJECTED,
	_DT_UNKNOWN_MESSAGEID,
	_DT_UNKNOWN_VENDORID,
};

static const char * dataTransferStatus_texts[]={
	"Accepted",
	"Rejected",
	"UnknownMessageId",
	"UnknownVendorId"
};

////DIAGNOSTICSNOTIFICATION. Ver 7.24
#define _IDLE_STATUS 0
#define _UPLOADED_STATUS 1
#define _UPLOADFAILED_STATUS 2
#define _UPLOADING_STATUS 3

////FIRMWARESTATUSNOTIFICATION. Ver 7.25
#define _DOWNLOADED_STATUS 0
#define _DOWNLOADFAIL_STATUS 1
#define _DOWNLOADING_STATUS 2
#define _IDLEFW_STATUS 3
#define _INSTALLATIONFAILED_STATUS 4
#define _INSTALLING_STATUS 5
#define _INSTALLED_STATUS 6

///////////////////////////////////////////////////
//7.29
typedef struct KeyValue{
	CiString50Type key;
	int readonly;
	CiString500Type value;
} KeyVal;

///////////////////////////////////////////////////
//7.32
enum MessageTrigger{
	_MT_BOOTNOTIFICATION,
	_MT_DIAGNOSTICSSTATUSNOTIFICATION,
	_MT_FIRMWARESTATUSNOTIFICATION,
	_MT_HEARTBEAT,
	_MT_METERVALUES,
	_MT_STATUSNOTIFICATION,
};

static const char * MessageTrigger_texts[]={
	"BootNotification",
	"DiagnosticsStatusNotification",
	"FirmwareStatusNotification",
	"Heartbeat",
	"MeterValues",
	"StatusNotification",
};

//////////////////////////////////////////////
////LOCATION Ver 7.30
enum Location{
		_MV_LOCATION_BODY,
		_MV_LOCATION_CABLE,
		_MV_LOCATION_EV,
		_MV_LOCATION_INLET,
		_MV_LOCATION_OUTLET
};

static const char* Location_texts[]={
		"Body",
		"Cable",
		"EV",
		"Inlet",
		"Outlet"
};

//////////////////////////////////////////////
////MEASURAND Ver 7.31
enum Measurand{
_MV_MEASURAND_CURRENT_EXPORT,
_MV_MEASURAND_CURRENT_IMPORT,
_MV_MEASURAND_CURRENT_OFFERED,
_MV_MEASURAND_ENERGY_ACTIVE_EXPORT_REGISTER,
_MV_MEASURAND_ENERGY_ACTIVE_IMPORT_REGISTER,
_MV_MEASURAND_ENERGY_REACTIVE_EXPORT_REGISTER,
_MV_MEASURAND_ENERGY_REACTIVE_IMPORT_REGISTER,
_MV_MEASURAND_ENERGY_ACTIVE_EXPORT_INTERVAL,
_MV_MEASURAND_ENERGY_ACTIVE_IMPORT_INTERVAL,
_MV_MEASURAND_ENERGY_REACTIVE_IMPORT_INTERVAL,
_MV_MEASURAND_ENERGY_REACTIVE_EXPORT_INTERVAL,
_MV_MEASURAND_FREQUENCY,
_MV_MEASURAND_POWER_ACTIVE_EXPORT,
_MV_MEASURAND_POWER_ACTIVE_IMPORT,
_MV_MEASURAND_POWER_FACTOR,
_MV_MEASURAND_POWER_OFFERED,
_MV_MEASURAND_POWER_REACTIVE_EXPORT,
_MV_MEASURAND_POWER_REACTIVE_IMPORT,
_MV_MEASURAND_RPM,
_MV_MEASURAND_SOC,
_MV_MEASURAND_TEMPERATURE,
_MV_MEASURAND_VOLTAGE,
};

static const char* Measurand_texts[]={
"Current.Export",
"Current.Import",
"Current.Offered",
"Energy.Active.Export.Register",
"Energy.Active.Import.Register",
"Energy.Reactive.Export.Register",
"Energy.Reactive.Import.Register",
"Energy.Active.Export.Interval",
"Energy.Active.Import.Interval",
"Energy.Reactive.Import.Interval",
"Energy.Reactive.Export.Interval",
"Frequency",
"Power.Active.Export",
"Power.Active.Import",
"Power.Factor",
"Power.Offered",
"Power.Reactive.Export",
"Power.Reactive.Import",
"RPM",
"SoC",
"Temperature",
"Voltage"
};

////PHASE Ver 7.34
enum Phase{
_MV_PHASE_L1,
_MV_PHASE_L2,
_MV_PHASE_L3,
_MV_PHASE_N,
_MV_PHASE_L1_N,
_MV_PHASE_L2_N,
_MV_PHASE_L3_N,
_MV_PHASE_L1_L2,
_MV_PHASE_L2_L3,
_MV_PHASE_L3_L1,
};

static const char * Phase_texts[]={
"L1",
"L2",
"L3",
"N",
"L1-N",
"L2-N",
"L3-N",
"L1-L2",
"L2-L3",
"L3-L1"
};

////////////////////////////////////////
////CONTEXT Ver 7.35
enum Context{
_MV_CONTEXT_INTERRUPTION_BEGIN,
_MV_CONTEXT_INTERRUPTION_END,
_MV_CONTEXT_OTHER,
_MV_CONTEXT_SAMPLE_CLOCK,
_MV_CONTEXT_SAMPLE_PERIODIC,
_MV_CONTEXT_TRANSACTION_BEGIN,
_MV_CONTEXT_TRANSACTION_END,
_MV_CONTEXT_TRIGGER
};

static const char * Context_texts[]={
"Interruption.Begin",
"Interruption.End",
"Other",
"Sample.Clock",
"Sample.Periodic",
"Transaction.Begin",
"Transaction.End",
"Trigger"
};

////////////////////////////////////////////////////////////////////////
// Ver 7.36
enum Stopping_Transaction_Reasons{
	_ST_EMERGENCY_STOP,
	_ST_EV_DISCONNECTED,
	_ST_HARD_RESET,
	_ST_LOCAL,
	_ST_OTHER,
	_ST_POWERLOSS,
	_ST_REBOOT,
	_ST_REMOTE,
	_ST_SOFT_RESET,
	_ST_UNLOCK_COMMAND,
	_ST_DEAUTHORIZED
};

static const char* Stopping_Transaction_Reason_texts[]={
	"EmergencyStop",
	"EVDisconnected",
	"HardReset",
	"Local",
	"Other",
	"PowerLoss",
	"Reboot",
	"Remote",
	"SoftReset",
	"UnlockCommand",
	"DeAuthorized"
};

//////////////////////////////////////////////
//REGISTRATIONSTATUSVALUESANDTEXTS. 7.38
enum Registration_Status{
	_RS_NOTCONNECTED,
	_RS_ACCEPTED,
	_RS_REJECTED,
	_RS_PENDING
};

static const char* Registration_Status_texts[]={
		"NotConnected",
		"Accepted",
		"Rejected",
		"Pending"
};

////DIAGNOSTICSNOTIFICATION
#define _ACCEPTED_STATUS 0
#define _REJECTED_STATUS 1
#define _REBOOTREQUIRED_STATUS 2
#define _NOTSUPPORTED_STATUS 3


//CHARGEPOINT Resrevation. Ver 7.40
enum ReservationStatus{
	_CP_RESERVATION_ACCEPTED,
	_CP_RESERVATION_FAULTED,
	_CP_RESERVATION_OCCUPIED,
	_CP_RESERVATION_REJECTED,
	_CP_RESERVATION_UNAVAILABLE,
};

static const char* ReservationStatus_texts[]={
		"Accepted","Faulted", "Occupied", "Rejected", "Unavailable"
};

//7.42
enum resetType{
	_RT_HARD,
	_RT_SOFT,
};

static const char* ResetType_texts[]={
		"Hard","Soft"
};



////////////////////////////////////////////////////////////////////////////
//7.45
////UNIT //From Schemas document:
enum UnitOfMeasure{
	_MV_UNIT_WH ,
	_MV_UNIT_KWH,
	_MV_UNIT_VARH ,
	_MV_UNIT_KVARH,
	_MV_UNIT_W ,
	_MV_UNIT_KW ,
	_MV_UNIT_VA ,
	_MV_UNIT_KVA ,
	_MV_UNIT_VAR ,
	_MV_UNIT_KVAR,
	_MV_UNIT_A,
	_MV_UNIT_V,
	_MV_UNIT_CELSIUS,
	_MV_UNIT_FAHRENHEIT,
	_MV_UNIT_KELVIN,
	_MV_UNIT_PERCENTAGE,
};

static const char* UnitOfMeasureTexts[]={
	"Wh",
	"kWh",
	"varh",
	"kvarh",
	"W",
	"kW",
	"VA",
	"kVA",
	"var",
	"kvar",
	"A",
	"V",
	"Celsius",
	"Fahrenheit",
	"K",
	"Percent"
};

///////////////////////////////////////////////////////////////////
//7.49
enum ValueFormat{
	_VF_RAW,
	_VF_SIGNEDDATA,
};

static const char* Format_Texts[]={
		"Raw",
		"SignedData"
};

////////////////////////////////////////////////////////////////////////////
//7.43
typedef struct SampledValue{
	char *value;
	enum Context context;
	enum UnitOfMeasure unit;
	enum ValueFormat formato;
	enum Measurand measurand;
	enum Location location;
	enum Phase phase;
};

//7.47
enum UpdateStatus{
	_UPS_ACCEPTED,
	_UPS_FAILED,
	_UPS_NOTSUPPORTED,
	_UPS_VERSIONMISMATCH
};

static const char* UpdateStatus_texts[]={
		"Accepted","Failed", "NotSupported", "VersionMismatch"
};

//7.48
enum UpdateType{
	_UT_DIFFERENTIAL,
	_UT_FULL
};

static const char* UpdateType_texts[]={
		"Differential","Full"
};

//7.28
struct IdToken{
	char IdToken[20];
};

//7.27
struct IdTagInfo{
	char *expiryDate;
	struct IdToken parentIdtag;
	enum AuthorizationStatus status;
};

char * reservationList[64];
char * connectorId[64];

//No me quedaba claro si en el wrapper, los tags deben enviarse o no... asi que añado una
//variable para que se pueda cambiar y lo codifico de ambas maneras
static int send_tags=0;

//No me quedaba claro si los enumerados deben enviarse como textos o como enteros... asi
//que añado una variable para que se pueda cambiar y lo codifico de ambas maneras
static int enums_as_integers=1;

///////////////////////////////////////////////////////////////
//Auxiliary
char *prepareWrapper(int MessageType, const char *UniqueId, char *action);
char *prepareWrapper_i(int MessageType, int UniqueId, char *action);
char *prepareResponseWrapper_i(int MessageType, int UniqueId, char *action);
char *prepareResponseWrapper(int MessageType, char * UniqueId, char *action);
char *prepareErrorResponse(json_object * obj_UniqueId, int error_code);

int checkDataTransferShouldbeRejected(json_object * Payload);

////////////////////////////////////////////////////////////////////////////////////////////
//4.1
char * prepareAuthorizeResponse(json_object * obj_UniqueId, struct IdTagInfo *idTagInfo);
//4.2
char * prepareBootNotificationResponse(json_object * obj_UniqueId, json_object * Payload, char *status);
//4.3
char* prepareDataTransferResponse(json_object * obj_UniqueId, json_object * obj_Payload);
int checkDataTransferShouldbeRejected(json_object * Payload);
//4.4
char *prepareDiagnosticsStatusNotificationResponse(const char *UniqueId);
//4.5
char *prepareFirmwareStatusNotificationResponse(const char *UniqueId);
//4.6
char *prepareHeartbeatResponse(const char *UniqueId);
//4.7
char *prepareMeterValuesResponse(const char *UniqueId);
//4.8
char *prepareStartTransactionResponse(const char *UniqueId, struct IdTagInfo *info, int transactionId);
//4.9
char *prepareStatusNotificationResponse(const char *UniqueId);
//4.10
char *prepareStopTransactionResponse(const char *UniqueId, struct IdTagInfo *info);
///////////////////////////////////////////////////////////////////////////////////////
//5.1
char *prepareCancelReservationRequest(int UniqueId, char *reservationID);
//5.2
char *prepareChangeAvailabilityRequest(int UniqueId, int connector, int new_availability);
//5.3
char *prepareChangeConfigurationRequest(int UniqueId, char *key, char *value);
//5.4
char *prepareClearCacheRequest(int UniqueId);
//5.5
char *prepareClearChargingProfileRequest(int UniqueId, const char *chargingprofileid, const char *connectorid_str, const char *purpose_str, const char *stacklevel_str);
//5.6
char *prepareDataTransferRequest(int UniqueId, char *vendorId, char *messageId, char *data);
//5.7

//5.8
char *prepareGetConfigurationRequest(int UniqueId, char *key);
//5.9
char *prepareGetDiagnosticsRequest(int UniqueId, const char *location, int retries, int retryInterval, const  char *startTime, const char *stopTime);
//5.10
char *prepareGetLocalListVersionRequest(int UniqueId);
//5.11
char *prepareRemoteStartTransactionRequest(int UniqueId, char *idTag, int connectorId, int transaction, int stackLevel, int purpose, int kind, int recurrency, char *validfrom, char *validto, int chargingProfile, int duration, char *startSchedule, int chargingRateUnit, struct ChargingSchedulePeriod periodos[], int num_periods, float minChargingRate);
//5.12
char *prepareRemoteStartTransactionStopRequest(int UniqueId, int transactionId);
//5.13
char *prepareReserveNowRequest(int UniqueId, int connector, char *expiryDate, char *idTag, int reservationId, char *parentIdTag);
//5.14
char *prepareResetRequest(int UniqueId, int resetType);
//5.15
char *prepareSendLocalListRequest(int UniqueId, int version, struct authorization_list_entry *list , int type);
//5.16
char *prepareSetChargingProfileRequest(int UniqueId, int connector, char *chargingProfile, char *transactionId);
//5.17
char *prepareTriggerMessageRequest(int UniqueId, int connector, int messageID);
//5.18
char *prepareUnlockConnectorRequest(int UniqueId, int connector);
//5.19
char *prepareFirmwareUpdateRequest(int UniqueId, char *location, char *retrieveDate, int retries, int retryInterval);

#endif /* OCPP_SERVER_H_ */
