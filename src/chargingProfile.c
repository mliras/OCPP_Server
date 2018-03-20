/*
 * chargingProfile.c
 *
 *  Created on: Dec 19, 2017
 *      Author: root
 */
#include "chargingProfile.h"

void chargingProfileInitialize()
{
		for (int i=0; i<MAX_NUM_CHARGINGPROFILES; i++) currentChargingProfile[i]=NULL;

	for (int i=0; i<MAX_NUM_CHARGINGPROFILES; i++) chargingProfiles[i]=NULL;
}

void show_chargingProfiles()
{
	printf("\nCHARGING PROFILES");
	printf("\n=================");
	for (int i=0; i<MAX_NUM_CHARGINGPROFILES; i++)
	{
		printf("\nCharging Profile %d: ", i);
		if (!chargingProfiles[i]) printf("NULL");
		else
		{
			printf("\n       Charging Profile Id: %d", chargingProfiles[i]->chargingProfileId);
			printf("\n       Charging Profile Kind: %d", chargingProfiles[i]->chargingProfileKind);
			printf("\n       Charging Profile Purpose: %d", chargingProfiles[i]->chargingProfilePurpose);
			printf("\n       StackLevel: %d", chargingProfiles[i]->stackLevel);
		}
	}
}
