/*
 * chargingProfile.h
 *
 *  Created on: Dec 19, 2017
 *      Author: root
 */

#ifndef CHARGINGPROFILE_H_
#define CHARGINGPROFILE_H_

#include "aux.h"

#define MAX_PERIODS 32



///////////
//Ver 7.37
enum RecurrencyKindType{
	_RKT_DAILY,
	_RKT_WEEKLY,
};

static const char *RecurrencyKindTypeTexts[]={
	"Daily",
	"Weekly",
};

///////////
//Ver 7.9
enum ChargingProfileKindType{
	_CPKT_ABSOLUTE,
	_CPKT_RECURRING,
	_CPKT_RELATIVE,
};

static const char *ChargingProfileKindTypeTexts[]={
	"Absolute",
	"Recurring",
	"Relative"
};

///////////
//Ver 7.10
enum ChargingProfilePurposeType{
	_CPPT_CHARGEPOINTMAXPROFILE,
	_CPPT_TXDEFAULTPROFILE,
	_CPPT_TXPROFILE,
};

static const char *ChargingProfilePurposeTypeTexts[]={
	"ChargePointMaxProfile",
	"TxDefaultProfile",
	"TxProfile"
};

///////////
//Ver 7.11
enum ChargingProfileStatus{
	_CPS_ACCEPTED,
	_CPS_REJECTED,
	_CPS_NOTSUPPORTED,
};

static const char *ChargingProfileStatusTexts[]={
	"Accepted",
	"Rejected",
	"NotSupported"
};

///////////
//Ver 7.12
enum ChargingRateUnitType{
	_CRUT_W,
	_CRUT_A
};

static const char *ChargingRateUnitTypeTexts[]={
	"W",
	"A",
};

///////////
//Ver 7.14
struct ChargingSchedulePeriod{
	int startPeriod;
	float limit;
	int *numPhases;
	struct ChargingSchedulePeriod *next;
};

///////////
//7.13
struct ChargingSchedule{
	int *duration; //[0..1]
	struct tm *startSchedule; //[0..1]
	enum ChargingRateUnitType chargingRateUnit;
	struct ChargingSchedulePeriod chargingSchedulePeriods;
	float *minChargingRate;
};

///////////
//Ver 7.8
struct ChargingProfile{
	int chargingProfileId;
	int *transactionId;  //[0..1] - Only valid if ChargingProfilePurpose is set to TxProfile,the transactionId MAY be used to match the	profile to a specific transaction.
	int stackLevel; //[1..1]
	enum ChargingProfilePurposeType chargingProfilePurpose;//[1..1]
	enum ChargingProfileKindType chargingProfileKind;//[1..1]
	enum RecurrencyKindType *recurrencyKind;  //[0..1]
	struct tm *validFrom; //[0..1]
	struct tm *validTo; //[0..1]
	struct ChargingSchedule chargingSchedule; //[1..1]
};

struct ChargingProfile *currentChargingProfile[MAX_NUM_CHARGEPOINTS];
struct ChargingProfile *chargingProfiles[MAX_NUM_CHARGINGPROFILES];

void chargingProfileInitialize();
void show_chargingProfiles();
int addChargingProfile(int chargepoint, struct ChargingProfile *ChPr, int connector);
void replaceChargingProfile(int chargepoint, struct ChargingProfile *ChPr, int connector);
void removeChargingProfile(int chargepoint, struct ChargingProfileList *temp2);

#endif /* CHARGINGPROFILE_H_ */
