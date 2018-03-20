/*
 * data_store.h
 *
 *  Created on: Dec 15, 2017
 *      Author: root
 */

#ifndef DATA_STORE_H_
#define DATA_STORE_H_

#include "aux.h"
#include "ocpp_server.h"
#include "server_middleware.h"

#include <mysql/mysql.h>

enum alarm_levels{
	AL_DEBUG,
	AL_INFO,
	AL_WARNING,
	AL_ERROR,
	AL_CRITICAL,
};

//Possible values:
//None --> 0
//File --> 1
//Database --> 2
int data_store_type;

//////////////////////////////////
//DATABASE
void connectDB(char *server, char *user, char *pass);
void LogInDb(struct chargePoint *cp, int alarm_code, int alarm_level, char *texto);
void LogInDb2(char *model, char *vendor, char *IP, char *SN, int alarm_code, int alarm_level, char *texto);

//////////////////////////////////
//FILES
int connectFiles();
int readConfigurationFromFile();
int  readVendorIDsFromFile();
int readChargePointModelsFromFile();

//////////////////////////////////
//Vendors and models
void updateVendorIDsInDB(char *old_vendorID, char *new_vendorID);
void writeChargePointModelinDB(MYSQL *conn, char *Vendor, char *Model, int numConnectors, char *meterType);
void readVendorIDsFromDB();
void readChargePointModelsFromDB();
int getModelIdFromDatabase(char *Model, char *Vendor);
void readVendorIDsFromDB();
void readConfigurationFromDB();
void readChargePointModelsFromDB();

//////////////////////////////////
//IdTags
char *getIdTagsFromDatabase();
char *getParentIdTagFromDatabase(char *idtag);

//////////////////////////////////
//AuthLists
void updateListVersionInDb(struct chargePoint *cp, int version);
int isValidAuthlistVersion(int version);
void readAuthorizationListsFromDB();
void readChargingProfilesFromDB();
void insertAuthorizationListInDb(struct authorization_list *list);
void insertEntryInListInDb(struct authorization_list_entry *entry, int list_id);

//////////////////////////////////
//Transactions
void removeTransactionFromDb(int transactionId);
void insertTransactionIntoDb(struct chargePoint *cp,int transactionId, int connector, double meterStart, char *currentTime);
int getLastTransactionId();
void updateTransactionStopTimeOnDb(int transactionId);

//////////////////////////////////
//Connections
void insertConnectionIntoDB(const char *chargePointModel, const char *chargePointVendor, const char *chargePointSerialNumber, char *ip, char *chargeBoxSerialNumber, char *firmwareVersion, char *iccid, char *imsi, char *meterSerialNumber, char *meterType, char *currentTime);

//////////////////////////////////
//Meter Values
void insertMeterValueIntoDB(int connectorId, int transactionId, const char *timeStamp, char *value, int context, int format,int measurand,int phase, int location, int unit);

//////////////////////////////////
//Chargepoints
void insertChargePointIntoDatabase2(char *IP, char *SN, char *Model, char *Vendor, char *CB_SN, char *FW_Version, char *ICCID, char *IMSI, char *Meter_SN, char *Description);
void insertChargePointIntoDatabase(struct chargePoint *cp, char *CB_SN, char *FW_Version, char *ICCID, char *IMSI, char *Meter_SN, char *Description);
int getChargepointIdFromDatabase2(char *IP, char *SN, char *Model, char *Vendor);
int getChargepointIdFromDatabase(struct chargePoint *cp);

//////////////////////////////////
//Reservations
void insertReservationIntoDatabase(char *vendor, char *model, char *IP, int Connector, int id, char *expiryDate, char *idTag);
void updateReservationInDB(int reservation_id, int status);
void expireReservationIdFromDb(int reservation_id);
int getLastReservationIdFromDb();

//////////////////////////////////
//ConfigurationKeys
void readConfigurationKeysfromDB(MYSQL *conn);
void insertValidKeysInDb(struct chargePoint *cp, KeyVal validkeys[70]);

#endif /* DATA_STORE_H_ */
