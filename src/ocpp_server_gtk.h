/*
 * gtk.h
 *
 *  Created on: Nov 9, 2017
 *      Author: root
 */

#ifndef OCPP_SERVER_GTK_H_
#define OCPP_SERVER_GTK_H_


#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkmain.h>
#include <gtk/deprecated/gtktable.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkcomboboxtext.h>
#include "server_middleware.h"
#include "authorizationLists.h"
#include "data_store.h"


static int logMutex=0;
int currentLocalListRow;
int logInScreen;

//GtkWidget *window;
GtkWindow *window;
GtkWidget *table;
GtkBuilder      *builder, *dialogbuilder;

//SOLAPA 1
GtkWidget *reservationscombo, *cancelreservationbtn, *clearauthorizationcachebtn, *chargepointsavailabilitycombo1;

//SOLAPA 2.1
GtkWidget *changeavailabilityconnectorscombo, *changeavailabilitynewstatuscombo, *changeavailabilitybtn, *chargepointsavailabilitycombo;

//SOLAPA 2.2
GtkWidget *changeconfigurationkeycombo, *changeconfigurationvaluetext, *changeconfigurationbtn, *chargepointsconfigurationcombo;

//SOLAPA 3.1
GtkWidget *activetransactionscombo, *chargingprofilecombo, *setchargingprofilebtn, *setchargingprofilechargepointscombo1, *chargingprofileconnectorcombo;

//SOLAPA 3.2
GtkWidget *clearchargingprofilechargepointscombo, *clearchargingprofilescurrentidscombo, *clearchargingprofileconnectorscombo, *clearchargingprofilebtn, *clearchargingprofilepurposecombo, *clearchargingprofilestacklevelcombo;

//SOLAPA 3.3
GtkWidget *createchargingprofilestacklevel, *createchargingprofilepurpose,*createchargingprofilekind,*createchargingprofilerecurrency,*createchargingprofilevalidfrom,*createchargingprofilevalidto;
GtkWidget *createchargingprofileperiodduration,*createchargingprofilechargingrateunit,*createchargingprofileminchargingrate,*createchargingprofilestartperiod1,*createchargingprofilelimit1;
GtkWidget *createchargingprofilenumphases1,*createchargingprofilestartperiod2,*createchargingprofilelimit2,*createchargingprofilenumphases2,*createchargingprofilestartperiod3,*createchargingprofilelimit3;
GtkWidget *createchargingprofilenumphases3,*createchargingprofilebtn,*createchargingprofileaddperiodbtn;

//SOLAPA 4
GtkWidget *chargepointsdatatransfercombo, *datatransfermessageidtext, *datatransferdatatext, *datatransferbtn;

//SOLAPA 5.1
GtkWidget *chargepointsgetlocallistcombo, *getlocallistbtn;

//SOLAPA 5.2
GtkWidget *chargepointssendlocallistcombo, *sendlocallistcombo, *sendlocallisttypecombo, *sendlocallistbtn;

//SOLAPA 5.3
GtkWidget *CreateListidTag[9];
GtkWidget *CreateListexpiryDate[9];
GtkWidget *CreateListparentIdTag[9];
GtkWidget *CreateListStatus[9];

GtkWidget *createListName, *createlocallistbtn, *createlocallistnextrowbtn, *createlocallistremoverowbtn;

//SOLAPA 6.1
GtkWidget *chargepointsgetlocallistcombo, *getlocallistbtn;

//SOLAPA 7.1
GtkWidget *remotestarttransactioncombo;
GtkWidget *remotestarttransactionconnectorcombo;
GtkWidget *remotestarttransactionidtagcombo;
GtkWidget *remotestarttransactionstacklevelcombo;
GtkWidget *remotestarttransactionpurposecombo;
GtkWidget *remotestarttransactionkindcombo;
GtkWidget *remotestarttransactionrecurrencycombo;
GtkWidget *remotestarttransactionvalidfromtext;
GtkWidget *remotestarttransactionvalidtotext;
GtkWidget *remotestarttransactionbtn;
GtkWidget *remotestarttransactionchargingprofilecheckbox;
GtkWidget *remotestarttransactiondurationtext;
GtkWidget *remotestarttransactionstartscheduletext;
GtkWidget *remotestarttransactionchargingrateunitcombo;
GtkWidget *remotestarttransactionstartperiodtext;
GtkWidget *remotestarttransactionlimittext;
GtkWidget *remotestarttransactionnumberphasestext;
GtkWidget *remotestarttransactionminchargingratetext;

//SOLAPA 7.2
GtkWidget *remotestoptransactioncombo1,*remotestoptransactioncombo2, *remotestoptransactionbtn;

//SOLAPA 8.1
GtkWidget *chargepointsgetconfigurationcombo, *getconfigurationkeycombo, *getconfigurationbtn, *getconfigurationkeytext, *getconfigurationaddtolistbtn;

//SOLAPA 8.2
GtkWidget *chargepointsgetdiagnosticscombo, *diagnosticslocationtext, *diagnosticsretriestext, *diagnosticsretryintervaltext, *diagnosticsstarttimetext, *diagnosticsstopttimetext, *getdiagnosticsbtn, *getstarttimebtn, *getstoptimebtn;

//SOLAPA 9.1 - RESERVE NOW
GtkWidget *chargepointsavailabilitycombo2, *changeavailabilityconnectorscombo2, *reservenowparentidtagtext, *reservenowidtagcombo, *reservenowbtn, *reservereservationidtext;

//SOLAPA 9.2 - UNLOCK CONNECTOR
GtkWidget *chargepointsunlockcombo, *connectorsunlockcombo, *unlockconnectorbtn;

//SOLAPA 10 - RESET
GtkWidget *chargepointsresetcombo, *chargepointsresettypecombo, *chargepointsresetbtn;

//SOLAPA 11 - TRIGGER MESSAGE
GtkWidget *chargepointstriggercombo, *connectorstriggercombo, *messagetriggercombo, *messagetriggerbtn;

//SOLAPA 12 - FIRMWARE UPDTE
GtkWidget *chargepointsfirmwarecombo, *firmwarelocationtext, *firmwareretriestext, *firmwareretryintervaltext, *firmwareretrievedatetext, *firmwarebtn;

//LOGS
GtkWidget *LogTextView2;

//Auxiliary functions
void appendLog(char *text, int colorpantalla);
int drawGUI();
void setGUIData();
const gchar *getGUIData(GtkWidget* textbox);
void addLog(char *text, int warning_level);
void getRandomIdTagGUI( GtkWidget *widget,gpointer data );
void getRandomReservationGUI( GtkWidget *widget,gpointer data );
void addGUIChargePoint(char *Vendor, char *Model, char *IP, char *SN);
void removeGUIChargePoint(int n, char *SN);

//OCPP request functions

// 5.1 CANCEL RESERVATION
void sendCancelReservation( GtkWidget *widget, gpointer   data );
//5.2 CHANGE AVAILABILITY
void sendChangeAvailability( GtkWidget *widget, gpointer   data );
// 5.3 CHANGE CONFIGURATION
void sendChangeConfiguration( GtkWidget *widget, gpointer   data );
// 5.4 CLEAR CACHE
void sendClearCache( GtkWidget *widget, gpointer   data );
// 5.5 CLEAR CACHING PROFILE
void clearChargingProfile( GtkWidget *widget, gpointer   data );
// 5.6 DATA TRANSFER
void sendDataTransfer( GtkWidget *widget, gpointer   data );
// 5.7 GET COMPOSITE SCHEDULE
void getCompositeSchedule( GtkWidget *widget, gpointer   data );
// 5.8 GET CONFIGURATION
void getConfiguration( GtkWidget *widget, gpointer   data );
// 5.9 GET DIAGNOSTICS
void getDiagnostics( GtkWidget *widget, gpointer   data );
// 5.10 GET LOCAL LIST VERSION
void getLocalListVersion( GtkWidget *widget, gpointer   data );
// 5.11 REMOTE START TRANSACTION
void sendRemoteStartTransaction( GtkWidget *widget, gpointer   data );
// 5.12 REMOTE STOP TRANSACTION
void sendRemoteStopTransaction( GtkWidget *widget, gpointer   data );
// 5.13 RESERVE NOW
void sendReserveNow( GtkWidget *widget, gpointer   data );
// 5.14 RESET
void sendReset( GtkWidget *widget, gpointer   data );
// 5.15 SEND LOCAL LIST
void sendLocalList( GtkWidget *widget, gpointer   data );
// 5.16 CLEAR CACHING PROFILE
void setChargingProfile( GtkWidget *widget, gpointer   data );
// 5.17 TRIGGER MESSAGE
void sendTriggerMessage( GtkWidget *widget, gpointer   data );
// 5.18 UNLOCK CONNECTOR
void sendUnlockConnector( GtkWidget *widget, gpointer   data );
// 5.19 UPDATE FIRMWARE
void sendUpdateFirmware( GtkWidget *widget, gpointer   data );

#endif /* OCPP_SERVER_GTK_H_ */
