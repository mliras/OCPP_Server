

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "ocpp_server_gtk.h"
#include <glib-2.0/glib.h>
#include <time.h>

typedef char   gchar;

#define FALSE 0
#define TRUE 1

#define RED 0
#define GREEN 1
#define BLUE 2
#define BLACK 4
#define BROWN 5

int contador;


void appendLog(char *text, int colorpantalla)
{
	GtkTextIter end;
	GtkTextIter start;
	GtkTextTag *colors;
	char *color_str;

	GtkTextBuffer *buf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(LogTextView2));
	if (contador%3==0)
	{
		gtk_text_buffer_set_text (buf,"",1);
	}

	if (colorpantalla==RED) color_str="#FF0000";
	if (colorpantalla==GREEN) color_str="#00FF00";
	if (colorpantalla==BLUE) color_str="#0000FF";
	if (colorpantalla==BROWN) color_str="#855723";

	colors=gtk_text_buffer_create_tag (buf, NULL,"foreground", color_str,NULL);

	gtk_text_buffer_get_end_iter (buf, &end);
	//gtk_text_buffer_get_start_iter (buf, &start);

	if (contador%3==0)
	{
		gtk_text_buffer_insert_with_tags (buf,&end,text,strlen(text)-1,colors,NULL);
	}

	//gtk_text_buffer_insert(buf, &end, text, strlen(text)-1);
	//gtk_text_buffer_insert(buf, &start, "\n",1);
	//gtk_text_view_set_buffer(GTK_TEXT_VIEW(LogTextView2), buf);

	//gtk_entry_set_attributes (LogTextView,PangoAttrList *attrs);

	//gtk_entry_set_text(GTK_ENTRY(LogTextView2),text);
	//gtk_entry_set_text(GTK_ENTRY(LogTextView2),"\n");

	contador++;
}

void show_info(char *texto) {

  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,texto);
  gtk_window_set_title(GTK_WINDOW(dialog), "Information");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

void CloseDialog (GtkWidget *widget, gpointer data)
{
    /* --- Remove grab --- */
    gtk_grab_remove (GTK_WIDGET (data));

    /* --- Close it. --- */
    gtk_widget_destroy (GTK_WIDGET (data));
}
/*
void updateStartTransactionTab( GtkWidget *widget,gpointer   data )
{
	int connector=gtk_combo_box_get_active(connectorId);
	const gchar *value;

	if (connector==0){
		value=gtk_entry_get_text(GTK_ENTRY (connector1));
	} else if (connector==1){
		value=gtk_entry_get_text(GTK_ENTRY (connector2));
	} else if (connector==2){
		value=gtk_entry_get_text(GTK_ENTRY (connector3));
	}

	gtk_entry_set_text (startConnectorText, value);
}

/*
void getReservationIdGUI( GtkWidget *widget,gpointer   data )
{
  //  g_signal_connect(G_OBJECT(getReservationIdBtn),"clicked",G_CALLBACK(getReservationIdGUI),G_OBJECT(getReservationIdBtn));

	int reservationId=getReservationId();

	char *num=(char *)malloc(sizeof(char)*5);
	sprintf(num, "%d", reservationId);
	gtk_entry_set_text (startReservationText, num);

}

*/

//Solapa 9.1
//Checks if the Reserve Now Button should be activated or not
void checkReserveNowButtonActivation( GtkWidget *widget,gpointer   data )
{
	const gchar *reservationId=gtk_entry_get_text(GTK_ENTRY (reservereservationidtext));
	const gchar *idTag=gtk_combo_box_text_get_active_text(GTK_ENTRY (reservenowidtagcombo));

	if (reservationId && idTag)
	{
		if (strlen(reservationId)>0 && strlen(idTag)>2)
		{
			gtk_widget_set_sensitive (GTK_WIDGET(reservenowbtn), TRUE);
		}
		else
		{
			gtk_widget_set_sensitive (GTK_WIDGET(reservenowbtn), FALSE);
		}
	}
}

void activateWidget( GtkWidget *widget,gpointer   data )
{
	gtk_widget_set_sensitive (GTK_WIDGET(data), TRUE);
}

void readIdTags( GtkWidget *widget,gpointer target )
{
	char *idtags;
	char** tokens;

	idtags=getIdTagsFromDatabase();

	if (idtags)
	{
		gtk_widget_set_sensitive (GTK_WIDGET(target), TRUE);
		tokens = str_split(idtags, ',');
		while (*tokens) gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(target),*(tokens++));
	}
	else show_info("No idtags found!!");
}

//Lee el idtag indicado en el combobox "widget", busca su parentidtag y lo escribe en "target"
void readParentIdTag(GtkWidget *widget,gpointer target )
{
	gchar *idtag=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

	char *parent=NULL;

	if (idtag) parent=getParentIdTagFromDatabase(idtag);

	if (parent) gtk_entry_set_text(GTK_ENTRY(target), parent);

}

void destroy( GtkWidget *widget,gpointer   data )
{
  gtk_main_quit  ();
}

//
//Esta funcion añade cada nuevo chargepoint se conecta a los combo box etiquetados como "chargepoint" de cada solapa
//
void addGUIChargePoint(char *Vendor, char *Model, char *IP, char *SN)
{
		char *text=(char *)calloc(100, sizeof(char));
		strncpy(text, Vendor, 20);
		strcat(text, "_");
		strncat(text, Model, 20);
		strcat(text, "_");
		strncat(text, IP, 16);

		if (SN)
		{
			strcat(text, "_");
			strncat(text, SN, 20);
		}

		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo),text);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo1),text);

		//SOLAPA 2.2
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsconfigurationcombo),text);

		//SOLAPA 3.1
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(setchargingprofilechargepointscombo1),text);

		//SOLAPA 3.2
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(clearchargingprofilechargepointscombo),text);

		//SOLAPA 5
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsdatatransfercombo),text);

		//SOLAPA 6
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsgetlocallistcombo),text);

		//SOLAPA 6.2
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointssendlocallistcombo),text);

		//SOLAPA 7.1
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(remotestarttransactioncombo),text);

		//SOLAPA 7.2
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(remotestoptransactioncombo1),text);

		//SOLAPA 8.2
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsgetdiagnosticscombo),text);

		//SOLAPA 8
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsgetconfigurationcombo),text);

		//SOLAPA 9.1
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo2),text);

		//SOLAPA 10
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsresetcombo),text);

		//SOLAPA 11
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointstriggercombo),text);

		//SOLAPA 12
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chargepointsfirmwarecombo),text);
}

void removeGUIChargePoint(int n, char *SN)
{
		gchar *text;

		//GtkTreeModel *modelo=gtk_combo_box_get_model (chargepointsavailabilitycombo);
		//int length=gtk_tree_path_get_depth (modelo);

		int borrado=0;
		for (int i=0; i<n && !borrado; i++)
		{
			gtk_combo_box_set_active (GTK_COMBO_BOX(chargepointsavailabilitycombo), i);
			text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo));

			if (strstr(text, SN) != NULL)
			{
				gtk_combo_box_text_remove (GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo), i);
				borrado=1;
			}
		}
}

//
// SOLAPA 1
//

void updateReservationCombos( GtkWidget *widget,gpointer data )
{
	char *vendor, *model;
	char** tokens;

	gtk_widget_set_sensitive (GTK_WIDGET(data) , TRUE);
	gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT(data));

	char *text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

	if (text)
	{
	 tokens = str_split(text, '_');

	 if (tokens)
	 {
		vendor=*(tokens);
		model=*(tokens+1);
	 }

	 printf("num Resrevationses %d y model: %s", numReservations, model);

	 if (numReservations==0)
	 {
		show_info("Currently no reservations made");
		//gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(data), "NO RESERVATIONS");
		gtk_combo_box_set_active (GTK_COMBO_BOX_TEXT(widget), -1);
		gtk_widget_set_sensitive (GTK_WIDGET(data) , FALSE);
	 }
	 else
	 {
	  for (int i=0; i<numReservations; i++)
	  {
		//char *res=);
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(data), convert(getReservationID(i)));
		//free(res);
	  }
	 }
	}//fin strlen(text)>0

	gtk_widget_set_sensitive (clearauthorizationcachebtn, FALSE);
}


//
//SOLAPA 2
//
void updateChangeCombos3( GtkWidget *widget,gpointer data )
{
	char *vendor, *model;
	char** tokens;
	//combo=changeavailabilityconnectorscombo;
	//GtkWidget *combo=GTK_COMBO_BOX_TEXT(data);
	gtk_widget_set_sensitive (GTK_WIDGET(data) , TRUE);
	gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT(data));

	//show_info("SE HA CAMBIADO");
	char *text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

//	printf("text es %s", text);

	tokens = str_split(text, '_');

	if (tokens)
	{
		vendor=*(tokens);
		model=*(tokens+1);
	}



	for (int i=0; i<getConnectorNumber(vendor, model); i++)
	{
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(data), convert(i+1));
	}
}

void updateChangeCombos2( GtkWidget *widget,gpointer   data )
{
	gtk_widget_set_sensitive (changeavailabilitynewstatuscombo , TRUE);
}

//SOLAPA 7.2
void populateTransactions( GtkWidget *widget,gpointer   data )
{
	char** tokens;
	char *IP, *SN;
	gtk_widget_set_sensitive (GTK_WIDGET(data), TRUE);

	const gchar *chargePoint;
	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remotestoptransactioncombo1));

	//Extraemos IP y SN
	tokens = str_split(chargePoint, '_');

	if (tokens)
	{
			IP=*(tokens+2);
			SN=*(tokens+3);
	}

	int i;
	for (i=0; i<MAX_NUM_CHARGEPOINTS; i++)
	{
			if (connected_ChargePoints[i].IP && connected_ChargePoints[i].SN)
			{
				if ((strcmp(connected_ChargePoints[i].IP,IP)==0) && strcmp(connected_ChargePoints[i].SN,SN)==0)
				{
					break;
				}
			}
	}

	//Si no encuentra el Chargepoint devuelve 1
	if (i==MAX_NUM_CHARGEPOINTS)
	{
			show_info("No se ha encontrado ese chargepoint entre los chargepoints conectados.");
			return;
	}

	//Si ya hay una transaction devuelve 2
	if (!currentTransactions[i])
	{
			show_info("No parece haber una transaccion para este chargepoint");
			return;
	}
	else
	{
		struct transaction *t=currentTransactions[i];
		while (t)
		{
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(remotestoptransactioncombo2),convert(t->transactionId));
			t=t->next;
		}
	}
}

void activateStartTransactionWidgets(GtkWidget *widget,gpointer data )
{
	gboolean activado=gtk_toggle_button_get_mode (GTK_TOGGLE_BUTTON(remotestarttransactionchargingprofilecheckbox));

	gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionstacklevelcombo) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionpurposecombo) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionkindcombo) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionrecurrencycombo) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionvalidfromtext) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionvalidtotext) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactiondurationtext) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionstartscheduletext) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionchargingrateunitcombo) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionstartperiodtext) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionlimittext) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionnumberphasestext) , activado);
			gtk_widget_set_sensitive (GTK_WIDGET(remotestarttransactionminchargingratetext) , activado);
}


//SOLAPA 3.1, 9.1, 9.2, 11,
void updateConnectorsCombos( GtkWidget *widget,gpointer data )
{
	char *vendor, *model;
	char** tokens;
	//combo=changeavailabilityconnectorscombo;
	//GtkWidget *combo=GTK_COMBO_BOX_TEXT(data);
	gtk_widget_set_sensitive (GTK_WIDGET(data) , TRUE);
	gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT(data));

	//show_info("SE HA CAMBIADO");
	char *text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

	//printf("text es %s", text);

	tokens = str_split(text, '_');

	if (tokens)
	{
		vendor=*(tokens);
		model=*(tokens+1);
	}

	for (int i=0; i<getConnectorNumber(vendor, model); i++)
	{
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(data), convert(i+1));

	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.4 CLEAR CACHE
//
void sendClearCache( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint;
	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo2));
	if (strlen(chargePoint)<1)
	{
			show_info("Please choose a Charge Point");
			return;
	}

	send_clearcache_request(chargePoint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.5 CLEAR CACHING PROFILE
//
void clearChargingProfile( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint, *chargingprofileid;
	int connectorid, purpose, stacklevel;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(clearchargingprofilechargepointscombo));

	if (strlen(chargePoint)<1)
	{
			show_info("Please choose a Charge Point");
			return;
	}

	chargingprofileid=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(clearchargingprofilescurrentidscombo));
	connectorid=gtk_combo_box_get_active(GTK_COMBO_BOX(clearchargingprofileconnectorscombo));

	if (strlen(chargingprofileid)<1 && strlen(chargingprofileid)<1)
	{
			show_info("Please choose either a Charging Profile Id or a Chargepoint's connector id");
			return;
	}

	purpose=gtk_combo_box_get_active(GTK_COMBO_BOX(clearchargingprofilepurposecombo));
	stacklevel=gtk_combo_box_get_active(GTK_COMBO_BOX(clearchargingprofilestacklevelcombo));

	send_clearchargingprofile_request(chargePoint, chargingprofileid, connectorid, purpose, stacklevel);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.6 DATA TRANSFER
//
void sendDataTransfer( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint, *messageId, *datos;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsdatatransfercombo));

	if (strlen(chargePoint)<1)
	{
		show_info("Please choose a Charge Point");
		return;
	}

	messageId=gtk_entry_get_text(GTK_ENTRY (datatransfermessageidtext));
	datos=gtk_entry_get_text(GTK_ENTRY (datatransferdatatext));

	char *Idmensaje=strdup(messageId);
	char *mensaje=strdup(datos);

	gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(chargepointsdatatransfercombo),-1);
	gtk_entry_set_text(GTK_ENTRY(datatransfermessageidtext),"");
	gtk_entry_set_text(GTK_ENTRY(datatransferdatatext),"");

	if (!Idmensaje || !mensaje || strlen(Idmensaje)<1 || strlen(mensaje)<1)
	{
		show_info("Please fill both fields (messageId and data)");
	}
	else
	{
		send_data_transfer_request(chargePoint, Idmensaje, mensaje);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.7 GET COMPOSITE SCHEDULE
//
void getCompositeSchedule( GtkWidget *widget, gpointer   data )
{
//	send_authorize_request(getGUIData(idTagtext));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.8 GET CONFIGURATION
//

void getConfigurationAddKeyToList( GtkWidget *widget, gpointer   data )
{
	const gchar *list, *key;
	key=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(getconfigurationkeycombo));
	list=gtk_entry_get_text(GTK_ENTRY(getconfigurationkeytext));



	if (key && strlen(key)>0)
	{
		if (list && strlen(list)<1)
		{
			gtk_entry_set_text(GTK_ENTRY(getconfigurationkeytext), key);
		}
		else
		{
			gchar *tmp=(char *)calloc(1, strlen(key)+strlen(list)+3);
			strncpy(tmp, list, strlen(list));
			strcat(tmp, ",");
			strncat(tmp, key, strlen(key));
			gtk_entry_set_text(GTK_ENTRY(getconfigurationkeytext), tmp);
			free(tmp);
		}
	}
}

void getConfiguration( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint, *keylist;
	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsgetconfigurationcombo));
	if (strlen(chargePoint)<1)
	{
			show_info("Please choose a Charge Point");
			return;
	}

	keylist=gtk_entry_get_text(GTK_ENTRY (getconfigurationkeytext));

	send_getconfiguration_request(chargePoint, keylist);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.9 GET DIAGNOSTICS
//

void getCurrentTimeAct( GtkWidget *widget, gpointer   data )
{
	char *currentTime=getCurrentTime();

	gtk_entry_set_text(GTK_ENTRY(data), currentTime);
}

void getDiagnostics( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint, *location;
	const gchar *retries=NULL;
	const gchar *retryInterval=NULL;
	const gchar *startTime=NULL;
	const gchar *stopTime=NULL;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsgetdiagnosticscombo));

	if (strlen(chargePoint)<1)
	{
		show_info("Please choose a Charge Point");
		return;
	}

	location=gtk_entry_get_text(GTK_ENTRY(diagnosticslocationtext));
	if ((!location)||(strlen(location)<1))
	{
		show_info("Please write a location where the file will be sent to (FTP IP Address)");
		return;
	}

	retries=gtk_entry_get_text(GTK_ENTRY(diagnosticsretriestext));
	if ((retries)&&strlen(retries)>0)
	{
		if (!checkIsNumber(retries))
		{
			show_info("Retries should be a number");
			return;
		}
	}
	else
	{
			retries=NULL;
	}

	retryInterval=gtk_entry_get_text(GTK_ENTRY(diagnosticsretryintervaltext));
	if ((retryInterval)&&strlen(retryInterval)>0)
	{
		if (!checkIsNumber(retryInterval))
		{
			show_info("Retry Interval should be a number");
			return;
		}
	}
	else
	{
		retryInterval=NULL;
	}

	startTime=gtk_entry_get_text(GTK_ENTRY(diagnosticsstarttimetext));
	if ((!startTime)||(strlen(startTime)<1))
	{
		startTime=NULL;
	}

	stopTime=gtk_entry_get_text(GTK_ENTRY(diagnosticsstopttimetext));
	if ((!stopTime)||(strlen(stopTime)<1))
	{
		stopTime=NULL;
	}

	send_getdiagnostics_request(chargePoint, location, retries, retryInterval, startTime, stopTime);
}


//3.1
void updateTransactionsCombos( GtkWidget *widget,gpointer data )
{
	const gchar *chargePoint;
	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(setchargingprofilechargepointscombo1));

	int connector=gtk_combo_box_get_active(GTK_COMBO_BOX(chargingprofileconnectorcombo))+1;

	char *vendor, *model, *SN, *IP;
	char** tokens;

	//combo=changeavailabilityconnectorscombo;
	//GtkWidget *combo=GTK_COMBO_BOX_TEXT(data);
//	gtk_widget_set_sensitive (GTK_WIDGET(data) , TRUE);
	//gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT(data));

	//show_info("SE HA CAMBIADO");

	//printf("text es %s", text);


	tokens = str_split(chargePoint, '_');

	if (tokens)
	{
		vendor=*(tokens);
		model=*(tokens+1);
		SN=*(tokens+2);
		IP=*(tokens+3);
	}

	struct transaction *t;
	for (int i=0; i<MAX_NUM_CHARGEPOINTS; i++)
	{
		//Recorremos por completo el array de transacciones.

		t=currentTransactions[i];
		while (t)
		{
			//Si tienen serial number y es el mismo, es la unica comprobacion que hacemos.
			if (SN && t->CP->SN)
			{
				if (strcmp(t->CP->SN, SN)==0)
				{
					if (strcmp(t->connectorId, connector)==0)
					{
						gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(activetransactionscombo), t->transactionId);
					}
				}
			}
			else
			{
				//En caso contrario hay que mirar Vednor, Model e IP

				if (strcmp(t->CP->Vendor, vendor)==0)
				{
					if (strcmp(t->CP->Model, model)==0)
					{
						if (strcmp(t->CP->IP, IP)==0)
						{
							if (strcmp(t->connectorId, connector)==0)
							{
								gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(activetransactionscombo), t->transactionId);
							}
						}
					}
				}
			}

			t=t->next;
		}
	}
}




	//SOLAPA 3.1
	void populateChargingProfiles( GtkWidget *widget,gpointer   data )
	{
		int found=0;
		for (int i=0; i<sizeof(chargingProfiles)/sizeof(chargingProfiles[0]); i++)
		{
			//printf("%d \n", sizeof(chargingProfiles));
			//printf("%d \n", chargingProfiles[i]);
			if (chargingProfiles[i])
			{
				gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(chargingprofilecombo), convert(chargingProfiles[i]->chargingProfileId));
				found=1;
			}
		}

		if (!found) show_info("No charging Profiles defined yet.");
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 5.1 CANCEL RESERVATION
	//
	void sendCancelReservation( GtkWidget *widget, gpointer   data )
	{
		const gchar *chargePoint;
		chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo1));

		const gchar *reservationID;
		reservationID=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(reservationscombo));

		if (!chargePoint)
		{
			show_info("Seleccione un Charge Point");
			gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo1),-1);
			gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(reservationscombo),-1);
			return;
		}

		send_cancel_reservation_request(chargePoint, reservationID);

		gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo1),-1);
		gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(reservationscombo),-1);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//5.2 CHANGE AVAILABILITY
	//
	void sendChangeAvailability( GtkWidget *widget, gpointer   data )
	{
		const gchar *chargePoint;
		chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo));

		int type=gtk_combo_box_get_active(changeavailabilityconnectorscombo);

		if (type<0)
		{
			show_info("Please introduce a type value.");
			return;
		}

		send_changeAvailability_request(chargePoint,gtk_combo_box_get_active (changeavailabilityconnectorscombo)+1, type);

		//Los deshabilitamos para la proxima vez
		//gtk_entry_set_text()
		gtk_combo_box_set_active(changeavailabilityconnectorscombo,-1);
		gtk_combo_box_set_active(changeavailabilitynewstatuscombo,-1);
		//gtk_widget_set_sensitive (changeavailabilityconnectorscombo , FALSE);
		//gtk_widget_set_sensitive (changeavailabilitynewstatuscombo , FALSE);
		gtk_widget_set_sensitive (changeavailabilitybtn , FALSE);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 5.3 CHANGE CONFIGURATION
	//

	void sendChangeConfiguration( GtkWidget *widget, gpointer data)
	{
		const gchar *chargePoint, *key, *value;

		chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsconfigurationcombo));

		if (strlen(chargePoint)<1)
		{
			show_info("Please choose a Charge Point");
			return;
		}

		key=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(changeconfigurationkeycombo));
		value=gtk_entry_get_text(GTK_ENTRY (changeconfigurationvaluetext));

		if (!key || !value || strlen(key)<1 || strlen(value)<1)
		{
			show_info("Please fill both fields (key and value)");
		}
		else
		{
			send_change_configuration_request(chargePoint, key, value);
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.10 GET LOCAL LIST VERSION
//
void getLocalListVersion( GtkWidget *widget, gpointer data)
{
	const gchar *chargePoint;
	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsgetlocallistcombo));

	if (strlen(chargePoint)<1)
	{
		show_info("Please choose a Charge Point");
		return;
	}

	//Show Display
	char *text=(char *)calloc(256, 1);
	sprintf(text, "Se solicita GET LOCAL LIST al chargepoint %s", chargePoint);
	appendLog(text, BLUE);
	free(text);

	//Send Message
	send_getlocallist_request(chargePoint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.11 REMOTE START TRANSACTION
//
void sendRemoteStartTransaction( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint, *connectorId, *idTag, *stacklevel, *validfrom, *validto, *duration, *startSchedule, *minChargingRate, *startPeriod, *limit, *numPhases;
	int purpose, kind, recurrency, chargingRateUnit;
	gboolean chargingProfile;
	int array_size=2;
	struct ChargingSchedulePeriod periodos[array_size];

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remotestarttransactioncombo));
	connectorId=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remotestarttransactionconnectorcombo));
	idTag=gtk_combo_box_text_get_active_text(GTK_ENTRY(remotestarttransactionidtagcombo));

	if ((!chargePoint) || (strlen(chargePoint)<1))
		{
				show_info("Please choose a Charge Point");
				return;
		}

		if ((!idTag) || (strlen(idTag)<3))
		{
				show_info("Please write a valid idTag");
				return;
		}

		if ((!connectorId) || (strlen(connectorId)<1))
		{
				if (connectorId) free(connectorId);
				connectorId=NULL;
		}

	chargingProfile=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(remotestarttransactionchargingprofilecheckbox));
	if (chargingProfile)
	{


		stacklevel=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remotestarttransactionstacklevelcombo));
		purpose=gtk_combo_box_get_active(GTK_COMBO_BOX(remotestarttransactionpurposecombo));
		kind=gtk_combo_box_get_active(GTK_COMBO_BOX(remotestarttransactionkindcombo));

		recurrency=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(remotestarttransactionrecurrencycombo));
		validfrom=gtk_entry_get_text(GTK_ENTRY(remotestarttransactionvalidfromtext));
		validto=gtk_entry_get_text(GTK_ENTRY(remotestarttransactionvalidtotext));

		duration=gtk_entry_get_text(GTK_ENTRY(remotestarttransactiondurationtext));
		startSchedule=gtk_entry_get_text(GTK_ENTRY(remotestarttransactionstartscheduletext));
		chargingRateUnit=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(remotestarttransactionchargingrateunitcombo));
		startPeriod=gtk_entry_get_text(GTK_ENTRY(remotestarttransactionstartperiodtext));
		limit=gtk_entry_get_text(GTK_ENTRY(remotestarttransactionlimittext));
		numPhases=gtk_entry_get_text(GTK_ENTRY(remotestarttransactionnumberphasestext));
		minChargingRate=gtk_entry_get_text(GTK_ENTRY(remotestarttransactionminchargingratetext));

		remotestarttransactiondurationtext=gtk_entry_get_text(GTK_ENTRY(remotestarttransactiondurationtext));

		if ((!limit) || (strlen(limit)<1))
		{
			show_info("Please choose a LIMIT value for the charging schedule Period");
					return;
		}


		if (stacklevel<0)
		{
				show_info("Please choose a STACK LEVEL for the charging profile");
				return;
		}

		if (purpose==-1)
		{
				show_info("Please choose a PURPOSE for the charging profile");
				return;
		}

		if (kind==-1)
		{
				show_info("Please choose a KIND for the charging profile");
				return;
		}

		if ((!validfrom) || (strlen(validfrom)<1))
		{
				if (validfrom) free(validfrom);
				validfrom=NULL;
		}

		if ((!validto) || (strlen(validto)<1))
		{
				if (validto) free(validto);
				validto=NULL;
		}

		if ((!duration) || (strlen(duration)<1))
		{
				if (duration) free(duration);
				duration=NULL;
		}
		else
		{
			if (!checkIsNumber(duration))
			{
				show_info("Duration should be a number");
				return;
			}
		}

		if ((!startSchedule) || (strlen(startSchedule)<1))
		{
				if (startSchedule) free(startSchedule);
				startSchedule=NULL;
		}
		else
		{
			if (!checkIsNumber(startSchedule))
			{
				show_info("Start Schedule should be a number");
				return;
			}
		}


		float f;
		if ((minChargingRate) && (strlen(minChargingRate)>0) && (sscanf(minChargingRate, "%f", &f) != 1))
		{
				show_info("The Min Charging Rate is not valid");
				minChargingRate=NULL;
				f=-1.0;
		}

		if ((!numPhases) || (strlen(numPhases)<1))
		{
				if (numPhases) free(numPhases);
				numPhases=NULL;
		}

		if (chargingRateUnit==-1)
		{
				show_info("Please choose a Charging Rate Unit for the charging Schedule");
				return;
		}

		if ((!startPeriod) || (strlen(startPeriod)<1))
		{
				show_info("Please choose a START PERIOD for the charging schedule");
				return;
		}



		//startPeriod, limit y numberPhases son realmente un array. Aunque los campos que tenemos son solo los de uno de ellos, voy a meter los mismos valores en dos
		//entradas del array (para simular varios valores). En una implementacion real, esto hay que hacerlo bien.

		for (int i=0; i<2; i++)
		{
			periodos[i].limit=atof(limit);

			periodos[i].startPeriod=convert(startPeriod);
			if (numPhases)
			{
				int *n=calloc(1, sizeof(int));
				*n=convert(numPhases);
				periodos[i].numPhases=n;
			}
		}
	}

	//////////
	//////////
	if (chargingProfile)
	{
			send_remotestarttransaction_request(chargePoint, idTag, connectorId, stacklevel, purpose, kind, recurrency, validfrom, validto, (int)chargingProfile, duration, startSchedule, chargingRateUnit, periodos, array_size, minChargingRate);
	}
	else
	{
			send_remotestarttransaction_request(chargePoint, idTag, connectorId, NULL, NULL, NULL, NULL, NULL,NULL, (int)chargingProfile, NULL, NULL, -1, NULL, 0, NULL);
	}

	//LIMPIAR LOS CAMPOS <-- NOT IMPLEMENTED
	gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(remotestarttransactionstacklevelcombo),-1);
	gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(remotestarttransactionpurposecombo),-1);
	gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(remotestarttransactionkindcombo),-1);
	gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(remotestarttransactionrecurrencycombo),-1);
	gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(remotestarttransactionchargingrateunitcombo),-1);

	gtk_entry_set_text(GTK_ENTRY(remotestarttransactionvalidfromtext) ,"");
	gtk_entry_set_text(GTK_ENTRY(remotestarttransactionvalidtotext ),"");
	gtk_entry_set_text(GTK_ENTRY(remotestarttransactiondurationtext ),"");
	gtk_entry_set_text(GTK_ENTRY(remotestarttransactionstartscheduletext) ,"");
	gtk_entry_set_text(GTK_ENTRY(remotestarttransactionstartperiodtext ),"");
	gtk_entry_set_text(GTK_ENTRY(remotestarttransactionlimittext ),"");
	gtk_entry_set_text(GTK_ENTRY(remotestarttransactionnumberphasestext ),"");
	gtk_entry_set_text(GTK_ENTRY(remotestarttransactionminchargingratetext ),"");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(remotestarttransactionchargingprofilecheckbox), FALSE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.12 REMOTE STOP TRANSACTION
//
void sendRemoteStopTransaction( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint, *transactionId;
	char** tokens;
	char *vendor, *model, *IP, *SN;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remotestoptransactioncombo1));
	transactionId=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(remotestoptransactioncombo2));

	if (chargePoint && transactionId)
	{
		send_remotestoptransaction_request(chargePoint,transactionId);
	}
	else
	{
		show_info("Could not obtain information about chargepoint and transaction ID");

		//Limpiar los campos
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.13 RESERVE NOW
//
void sendReserveNow( GtkWidget *widget, gpointer   data )
{
	const gchar *parentIdTag, *idTag, *chargePoint, *reservationId;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo2));

	int connector=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(changeavailabilityconnectorscombo2))+1;

	//El ID DE LA RESERVA EN VEZ DE COGERLA DE LA GUI, A PARTIR DE AHORA SE COGE DE BD:
	//reservationId=gtk_entry_get_text(GTK_ENTRY (reservereservationidtext));
	reservationId=convert(getLastReservationIdFromDb()+1);

	parentIdTag=gtk_entry_get_text(GTK_ENTRY (reservenowparentidtagtext));
	idTag=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT (reservenowidtagcombo));


	if (!checkIsNumber(reservationId) || strlen(reservationId)>8)
	{
		show_info("The reservation Id should be a numeric value lower than 99999999.");
		return;
	}

	if (strlen(parentIdTag)<3)
	{
		//free(parentIdTag);
		parentIdTag=NULL;
	}

	if (strlen(reservationId)<3)
	{
			//free(parentIdTag);
			parentIdTag=NULL;
	}

	if (strlen(idTag)<3)
	{
		show_info("Please insert a valid idTag Token value. idTags, reservationIds and parentIdTags should be at least characters long.");
	}
	else
	{

		send_reservenow_request(chargePoint, connector, idTag, reservationId, parentIdTag);

		gtk_combo_box_set_active(changeavailabilityconnectorscombo2,-1);
		gtk_combo_box_set_active(reservenowidtagcombo,-1);
		gtk_entry_set_text(reservenowparentidtagtext ,"");
		gtk_entry_set_text(reservereservationidtext,"");
		gtk_widget_set_sensitive (changeavailabilitybtn , FALSE);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.14 RESET
//
void sendReset( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint;
	int resetType;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsresetcombo));

	if (strlen(chargePoint)<1)
	{
		show_info("Please choose a Charge Point");
		return;
	}

	resetType=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(chargepointsresettypecombo));

	if (resetType<0)
	{
		show_info("Please choose a reset Type");
		return;
	}

	send_reset_request(chargePoint, resetType);
}


int newLocalListRow( GtkWidget *widget, gpointer   data )
{
	const gchar *IdTag, *expiryDate, *parentIdTag;
	int status;

	IdTag=gtk_entry_get_text(GTK_ENTRY (CreateListidTag[currentLocalListRow]));
	expiryDate=gtk_entry_get_text(GTK_ENTRY (CreateListexpiryDate[currentLocalListRow]));
	parentIdTag=gtk_entry_get_text(GTK_ENTRY (CreateListparentIdTag[currentLocalListRow]));
	status=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(CreateListStatus[currentLocalListRow]));

	if (strlen(IdTag)<3)
	{
			show_info("Please write a valid IdTag");
			return 0;
	}

	if (status<0)
	{
		if ((strlen(parentIdTag)>2) || (strlen(expiryDate)>2))
		{
			show_info("If you write an expiryDate or parentIdTag, you need to choose an status for that authorization");
			return 0;
		}
	}

	if (expiryDate)
	{
		struct tm* tm=(struct tm *)calloc(1, sizeof(struct tm));
		strptime(expiryDate, "%Y-%m-%dT%H:%M:%S.", tm);

		if (!tm)
		{
			show_info("Malformed expiry Date");
			return 0;
		}
	}

	//It's a valid row. Now I'll lock it and show the next row
	gtk_widget_set_sensitive (GTK_ENTRY(CreateListidTag[currentLocalListRow]) , FALSE);
	gtk_widget_set_sensitive (GTK_ENTRY(CreateListexpiryDate[currentLocalListRow]) , FALSE);
	gtk_widget_set_sensitive (GTK_ENTRY(CreateListparentIdTag[currentLocalListRow]) , FALSE);
	gtk_widget_set_sensitive (GTK_COMBO_BOX_TEXT(CreateListStatus[currentLocalListRow]) , FALSE);

	if (currentLocalListRow<9) currentLocalListRow++;

	if (currentLocalListRow<9)
	{
		gtk_widget_set_visible (GTK_ENTRY(CreateListidTag[currentLocalListRow]) , TRUE);
		gtk_entry_set_text(GTK_ENTRY(CreateListexpiryDate[currentLocalListRow]), getCurrentTime());
		gtk_widget_set_visible (GTK_ENTRY(CreateListexpiryDate[currentLocalListRow]) , TRUE);

		gtk_widget_set_visible (GTK_ENTRY(CreateListparentIdTag[currentLocalListRow]) , TRUE);
		gtk_widget_set_visible (GTK_COMBO_BOX_TEXT(CreateListStatus[currentLocalListRow]) , TRUE);
	}

	return 1;
}

int removeLocalListRow( GtkWidget *widget, gpointer   data )
{
	gtk_widget_set_visible (GTK_ENTRY(CreateListidTag[currentLocalListRow]), FALSE);
	gtk_widget_set_visible (GTK_ENTRY(CreateListexpiryDate[currentLocalListRow]), FALSE);
	gtk_widget_set_visible (GTK_ENTRY(CreateListparentIdTag[currentLocalListRow]), FALSE);
	gtk_widget_set_visible (GTK_COMBO_BOX_TEXT(CreateListStatus[currentLocalListRow]), FALSE);

	gtk_entry_set_text (GTK_ENTRY(CreateListidTag[currentLocalListRow]), "");
	gtk_entry_set_text(GTK_ENTRY(CreateListexpiryDate[currentLocalListRow]), "");
	gtk_entry_set_text(GTK_ENTRY(CreateListparentIdTag[currentLocalListRow]), "");
	gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(CreateListStatus[currentLocalListRow]), -1);

	currentLocalListRow--;
	gtk_widget_set_sensitive (CreateListidTag[currentLocalListRow] , TRUE);
	gtk_widget_set_sensitive (CreateListexpiryDate[currentLocalListRow] , TRUE);
	gtk_widget_set_sensitive (CreateListparentIdTag[currentLocalListRow] , TRUE);
	gtk_widget_set_sensitive (CreateListStatus[currentLocalListRow] , TRUE);

	return 1;
}

//
//Esta funcion es llamada cuando se pulsa el boton "Create list"
//devuelve 0 si no puede crear la lista y el numero de entradas, si la consiguio crear
//
int createLocalList( GtkWidget *widget, gpointer data)
{
	const gchar *listName;
	const gchar *IdTag, *expiryDate, *parentIdTag;
	int status;
	int position;

	listName=gtk_entry_get_text(GTK_ENTRY (createListName));

	if (strlen(listName)<3)
	{
		show_info("Please write a list name");
		return 0;
	}

	if (currentLocalListRow==0)
	{
		show_info("Please press the button 'New row' before creating the list");
		return 0;
	}

	//Creamos la lista vacia. Nos devuelve la posicion dentro de la lista para poder referenciarla...
	position=createAuthorizationList(listName);

	if (position<0)
	{
		show_info("Hubo algun error al crear la lista");
		return -1;
	}

	struct authorization_list *list=authorization_lists[position];

	//Y rellenarla:
	for (int i=0; i<currentLocalListRow; i++)
	{
		IdTag=gtk_entry_get_text(GTK_ENTRY (CreateListidTag[i]));
		expiryDate=gtk_entry_get_text(GTK_ENTRY (CreateListexpiryDate[i]));
		parentIdTag=gtk_entry_get_text(GTK_ENTRY (CreateListparentIdTag[i]));
		status=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(CreateListStatus[i]));

		//Añadimos una entrada a la lista
		addAuthorizationDataToList(list, IdTag, parentIdTag, expiryDate, status);
	}

	insertAuthorizationListInDb(list);
	//show_info("Lista creada correctamente");
	//if (debug) showAuthorizationLists();
	//Vaciamos los campos:

	gtk_entry_set_text(createListName,"");
	for (int i=0; i<=currentLocalListRow; i++)
	{
		gtk_entry_set_text(GTK_ENTRY (CreateListidTag[i]),"");
		gtk_entry_set_text(GTK_ENTRY (CreateListexpiryDate[i]),"");
		gtk_entry_set_text(GTK_ENTRY (CreateListparentIdTag[i]),"");
		gtk_combo_box_set_active(GTK_COMBO_BOX_TEXT(CreateListStatus[i]),-1);

		gtk_widget_set_sensitive(GTK_ENTRY (CreateListidTag[i]) , TRUE);
		gtk_widget_set_sensitive(GTK_ENTRY (CreateListexpiryDate[i]) , TRUE);
		gtk_widget_set_sensitive(GTK_ENTRY (CreateListparentIdTag[i]) , TRUE);
		gtk_widget_set_sensitive(GTK_COMBO_BOX_TEXT(CreateListStatus[i]) , TRUE);

		if (i>0)
		{
			 gtk_widget_set_visible (GTK_ENTRY (CreateListidTag[i]) , FALSE);
			 gtk_widget_set_visible(GTK_ENTRY (CreateListexpiryDate[i]) , FALSE);
			 gtk_widget_set_visible(GTK_ENTRY (CreateListparentIdTag[i]) , FALSE);
			 gtk_widget_set_visible(GTK_COMBO_BOX_TEXT(CreateListStatus[i]) , FALSE);
		}
	}

	currentLocalListRow=0;
	gtk_entry_set_text(GTK_ENTRY (CreateListexpiryDate[0]),getCurrentTime());

	return currentLocalListRow;
}

void populateLocalLists( GtkWidget *widget, gpointer   data )
{
	int num=0;
	//sendlocallistcombo=GTK_WIDGET(gtk_builder_get_object(builder, "sendlocallistcombo"));

	//if (debug) showAuthorizationLists();

	for (int i=0; i<MAX_NUM_AUTH_LISTS; i++)
	{
		if (authorization_lists[i])
		{
			printf("BBBBB: %s", authorization_lists[i]->name);
			gtk_combo_box_text_append_text(GTK_COMBO_BOX(sendlocallistcombo), authorization_lists[i]->name);
			//gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(sendlocallistcombo), 0, authorization_lists[i]->name);
			//gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sendlocallistcombo),"H");
			num++;
		}
	}

	if (num==0)
	{
		show_info("The authorization list is empty. Please create a new local list.");
		return;
	}
}

//
// 5.15 SEND LOCAL LIST
//
void sendLocalList( GtkWidget *widget, gpointer   data )
{
	const gchar *listName;
	const gchar *chargePoint;
	int type;

	listName=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(sendlocallistcombo));
	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointssendlocallistcombo));
	type=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(sendlocallisttypecombo));

	//The local list is NOT mandatory. The other two are.
	if (!chargePoint || (strlen(chargePoint)<2)||type==-1)
	{
				show_info("Invalid data");
				return;
	}

	send_locallist_request(chargePoint, listName, type);
}


//
// 5.16 SET CACHING PROFILE
//
void setChargingProfile( GtkWidget *widget, gpointer data)
{
	const gchar *chargePoint, *transactionId, *chargingProfile;
	int connector;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(setchargingprofilechargepointscombo1));

	connector=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(chargingprofileconnectorcombo))+1;

	transactionId=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(activetransactionscombo));

	chargingProfile=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargingprofilecombo));

	if ((strlen(chargePoint)<2) || (strlen(chargingProfile)<2) || connector<0)
	{
		show_info("Invalid data");
		return;
	}

	send_setchargingprofile_request(chargePoint, connector, transactionId, chargingProfile);
}

//
// 5.17 TRIGGER MESSAGE
//
void sendTriggerMessage(GtkWidget *widget, gpointer data)
{
	const gchar *chargePoint;
	int connector;
	int messageID;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointstriggercombo));
	connector=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(connectorstriggercombo))+1;
	messageID=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(messagetriggercombo));

	if ((chargePoint && (strlen(chargePoint)<1))||messageID==-1)
	{
			show_info("Invalid data");
			return;
	}

	send_messagetrigger_request(chargePoint, connector, messageID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 5.18 UNLOCK CONNECTOR
//
void sendUnlockConnector( GtkWidget *widget, gpointer data)
{
	const gchar *chargePoint;
	int connector;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsavailabilitycombo2));

	connector=gtk_combo_box_get_active(GTK_COMBO_BOX_TEXT(changeavailabilityconnectorscombo2))+1;

	if ((strlen(chargePoint)<1)||connector==0)
	{
		show_info("Invalid data");
		return;
	}

	send_unlockconnector_request(chargePoint, connector);
}

//
// 5.19 UPDATE FIRMWARE
//
void sendUpdateFirmware( GtkWidget *widget, gpointer   data )
{
	const gchar *chargePoint, *location;
	const gchar *retries=NULL;
	const gchar *retryInterval=NULL;
	const gchar *retrieveDate=NULL;

	chargePoint=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(chargepointsfirmwarecombo));

	//campo obligatorio
	if (strlen(chargePoint)<1)
	{
		show_info("Please choose a Charge Point");
		return;
	}

	//campo obligatorio
	location=gtk_entry_get_text(GTK_ENTRY(firmwarelocationtext));
	if ((!location)||(strlen(location)<1))
	{
		show_info("Please write a location where the file will be sent to (FTP IP Address)");
		return;
	}

	//campo obligatorio
	retrieveDate=gtk_entry_get_text(GTK_ENTRY(firmwareretrievedatetext));
	if ((!retrieveDate)||(strlen(retrieveDate)<1))
	{
		show_info("Please introduce a valid retrieve Date");
		return;
	}
	else
	{
		char *buf=(char *)calloc(1, 80);
		struct tm tp;
		strptime(retrieveDate, "%Y-%m-%dT%H:%M:%S.", &tp);

		//Ver https://stackoverflow.com/questions/11428014/c-validation-in-strptime
		mktime (&tp);
		strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.", &tp);

		if (strcmp(buf, retrieveDate)!=0)
		{
			show_info("Please introduce a valid retrieve Date");
			return;
		}
	}

	//campo opcional
	retries=gtk_entry_get_text(GTK_ENTRY(firmwareretriestext));
	if ((retries)&&strlen(retries)>0)
	{
		if (!checkIsNumber(retries))
		{
			show_info("Retries should be a number");
			return;
		}
	}
	else
	{
		retries=NULL;
	}

	//campo opcional
	retryInterval=gtk_entry_get_text(GTK_ENTRY(firmwareretryintervaltext));
	if ((retryInterval)&&strlen(retryInterval)>0)
	{
		if (!checkIsNumber(retryInterval))
		{
			show_info("Retry Interval should be a number");
			return;
		}
	}
	else
	{
		retryInterval=NULL;
	}

	send_firmwareupdate_request(chargePoint, location, retrieveDate, retries, retryInterval);

//	send_authorize_request(getGUIData(idTagtext));
}



void addLog(char *texto, int warning_level)
{
	//const gchar *text;
	int GUI=1;
	GtkEntryBuffer *logsbuffer;
	GtkTextIter end;

	//while(logMutex) usleep(10);

	//logMutex=1;
	//if (file) writeLogToFile(text, warning_level); <--NOT IMPLEMENTED

	//if (screen)	printf("[LOG] %s", text);
/*
	if (GUI)
	{
		logsbuffer = gtk_text_view_get_buffer (GTK_ENTRY(logstext));
	//	text=gtk_entry_get_text(GTK_ENTRY (logstext));

		gtk_text_buffer_get_end_iter (logsbuffer, &end);
		gtk_text_buffer_insert (logsbuffer, &end, texto, (int) strlen(texto));
	}
*/
		/*GtkEntryBuffer *buffer;
		buffer = gtk_entry_get_buffer (GTK_ENTRY (textbox));
		t=gtk_entry_buffer_get_text (buffer);
	*/
		//text=gtk_entry_get_text(GTK_ENTRY (textbox));


	//buffer1 = gtk_text_view_get_buffer (GTK_ENTRY(logstext));
   	//gtk_text_buffer_create_tag(buffer1, "blue_fg", 	       "foreground", "blue", NULL);
	//logMutex=0;
}

const gchar *getGUIData(GtkWidget* textbox)
{

	const gchar *text;
	text=gtk_entry_get_text(GTK_ENTRY (textbox));
	return text;
}


void setGUIData()
{


}

int drawGUI()
{
	    gtk_init(0, NULL);

	    logInScreen=1;
	    builder = gtk_builder_new();
	    gtk_builder_add_from_file (builder, "/etc/ocpp/server.glade", NULL);

	    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
	    gtk_builder_connect_signals(builder, NULL);

	    //SOLAPA 1

		chargepointsavailabilitycombo1=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsavailabilitycombo1"));
	    reservationscombo=GTK_WIDGET(gtk_builder_get_object(builder, "reservationscombo"));
		cancelreservationbtn=GTK_WIDGET(gtk_builder_get_object(builder, "cancelreservationbtn"));
		clearauthorizationcachebtn=GTK_WIDGET(gtk_builder_get_object(builder, "clearauthorizationcachebtn"));

		//Cuando seleccionemos un chargepoint, debemos actualizar el combo de las reservas:
		g_signal_connect(G_OBJECT(chargepointsavailabilitycombo1),"changed",G_CALLBACK(updateReservationCombos),G_OBJECT(reservationscombo));
		g_signal_connect(G_OBJECT(chargepointsavailabilitycombo1),"changed",G_CALLBACK(activateWidget),G_OBJECT(clearauthorizationcachebtn));

		g_signal_connect(G_OBJECT(reservationscombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(cancelreservationbtn));

		g_signal_connect(G_OBJECT(cancelreservationbtn),"clicked",G_CALLBACK(sendCancelReservation),G_OBJECT(cancelreservationbtn));
		g_signal_connect(G_OBJECT(clearauthorizationcachebtn),"clicked",G_CALLBACK(sendClearCache),G_OBJECT(clearauthorizationcachebtn));

		 gtk_widget_set_sensitive (reservationscombo , FALSE);
		 gtk_widget_set_sensitive (cancelreservationbtn , FALSE);
		 gtk_widget_set_sensitive (clearauthorizationcachebtn , FALSE);

		 //SOLAPA 2
		GtkWidget *changeTab=GTK_WIDGET(gtk_builder_get_object(builder, "changeTab"));
		//g_signal_connect(G_OBJECT(changeTab),"select-page",G_CALLBACK(updateChangeCombos),G_OBJECT(changeTab));

	    //SOLAPA 2.1
		chargepointsavailabilitycombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsavailabilitycombo"));
		changeavailabilityconnectorscombo=GTK_WIDGET(gtk_builder_get_object(builder, "connectorsavailabilitycombo"));
		changeavailabilitynewstatuscombo=GTK_WIDGET(gtk_builder_get_object(builder, "changeavailabilitynewstatuscombo"));
		changeavailabilitybtn=GTK_WIDGET(gtk_builder_get_object(builder, "changeavailabilitybtn"));

		g_signal_connect(G_OBJECT(chargepointsavailabilitycombo),"changed",G_CALLBACK(updateChangeCombos3),G_OBJECT(changeavailabilityconnectorscombo));
		g_signal_connect(G_OBJECT(changeavailabilityconnectorscombo),"changed",G_CALLBACK(updateChangeCombos2),G_OBJECT(changeavailabilityconnectorscombo));
		g_signal_connect(G_OBJECT(changeavailabilitynewstatuscombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(changeavailabilitybtn));

		g_signal_connect(G_OBJECT(changeavailabilitybtn),"clicked",G_CALLBACK(sendChangeAvailability),G_OBJECT(changeavailabilitybtn));

		//Pag 48
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(changeavailabilitynewstatuscombo), "Available");
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(changeavailabilitynewstatuscombo), "Unavailable");
		gtk_widget_set_sensitive (changeavailabilityconnectorscombo , FALSE);
		gtk_widget_set_sensitive (changeavailabilitynewstatuscombo , FALSE);
		gtk_widget_set_sensitive (changeavailabilitybtn , FALSE);

		//updateChargePoints();

	    //SOLAPA 2.2
		chargepointsconfigurationcombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsconfigurationcombo"));
	    changeconfigurationkeycombo=GTK_WIDGET(gtk_builder_get_object(builder, "changeconfigurationkeycombo"));
		changeconfigurationvaluetext=GTK_WIDGET(gtk_builder_get_object(builder, "changeconfigurationvaluetext"));
		changeconfigurationbtn=GTK_WIDGET(gtk_builder_get_object(builder, "changeconfigurationbtn"));
		g_signal_connect(G_OBJECT(changeconfigurationbtn),"clicked",G_CALLBACK(sendChangeConfiguration),G_OBJECT(changeconfigurationbtn));

		for (int i=0; i<sizeof(clientConfigurationKeys)/sizeof(clientConfigurationKeys[0]); i++)
		{
			if ((clientConfigurationKeys[i]) && (strlen(clientConfigurationKeys[i])>0)) gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(changeconfigurationkeycombo), clientConfigurationKeys[i]);
		}

	    //SOLAPA 3.1
		setchargingprofilechargepointscombo1=GTK_WIDGET(gtk_builder_get_object(builder, "setchargingprofilechargepointscombo1"));
		activetransactionscombo=GTK_WIDGET(gtk_builder_get_object(builder, "activetransactionscombo"));
		chargingprofileconnectorcombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargingprofileconnectorcombo"));
		chargingprofilecombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargingprofilecombo"));

		gtk_widget_set_sensitive (activetransactionscombo, FALSE);
		gtk_widget_set_sensitive (chargingprofileconnectorcombo , FALSE);
		gtk_widget_set_sensitive (chargingprofilecombo , FALSE);


		g_signal_connect(G_OBJECT(setchargingprofilechargepointscombo1),"changed",G_CALLBACK(activateWidget),G_OBJECT(chargingprofileconnectorcombo));
		g_signal_connect(G_OBJECT(chargingprofileconnectorcombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(activetransactionscombo));
		g_signal_connect(G_OBJECT(setchargingprofilechargepointscombo1),"changed",G_CALLBACK(activateWidget),G_OBJECT(chargingprofilecombo));

		g_signal_connect(G_OBJECT(setchargingprofilechargepointscombo1),"changed",G_CALLBACK(updateConnectorsCombos),G_OBJECT(chargingprofileconnectorcombo));
		g_signal_connect(G_OBJECT(chargingprofileconnectorcombo),"changed",G_CALLBACK(updateTransactionsCombos),G_OBJECT(activetransactionscombo));
		g_signal_connect(G_OBJECT(setchargingprofilechargepointscombo1),"changed",G_CALLBACK(populateChargingProfiles),G_OBJECT(chargingprofileconnectorcombo));

		setchargingprofilebtn=GTK_WIDGET(gtk_builder_get_object(builder, "setchargingprofilebtn"));
		g_signal_connect(G_OBJECT(setchargingprofilebtn),"clicked",G_CALLBACK(setChargingProfile),G_OBJECT(setchargingprofilebtn));

	    //SOLAPA 3.2
		clearchargingprofilechargepointscombo=GTK_WIDGET(gtk_builder_get_object(builder, "clearchargingprofilechargepointscombo"));
		clearchargingprofilescurrentidscombo=GTK_WIDGET(gtk_builder_get_object(builder, "clearchargingprofilescurrentidscombo"));
		clearchargingprofileconnectorscombo=GTK_WIDGET(gtk_builder_get_object(builder, "clearchargingprofileconnectorscombo"));
		clearchargingprofilepurposecombo=GTK_WIDGET(gtk_builder_get_object(builder, "clearchargingprofilepurposecombo"));
		clearchargingprofilestacklevelcombo=GTK_WIDGET(gtk_builder_get_object(builder, "clearchargingprofilestacklevelcombo"));

		gtk_widget_set_sensitive (clearchargingprofilescurrentidscombo, FALSE);
		gtk_widget_set_sensitive (clearchargingprofileconnectorscombo , FALSE);

		g_signal_connect(G_OBJECT(clearchargingprofilechargepointscombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(clearchargingprofilescurrentidscombo));
		g_signal_connect(G_OBJECT(clearchargingprofilechargepointscombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(clearchargingprofileconnectorscombo));

		clearchargingprofilebtn=GTK_WIDGET(gtk_builder_get_object(builder, "clearchargingprofilebtn"));
		g_signal_connect(G_OBJECT(clearchargingprofilebtn),"clicked",G_CALLBACK(clearChargingProfile),G_OBJECT(clearchargingprofilebtn));

		//SOLAPA 3.3
		createchargingprofilestacklevel=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilestacklevel"));
		createchargingprofilepurpose=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilepurpose"));
		createchargingprofilekind=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilekind"));
		createchargingprofilerecurrency=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilerecurrency"));
		createchargingprofilevalidfrom=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilevalidfrom"));
		createchargingprofilevalidto=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilevalidto"));
		createchargingprofileperiodduration=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofileperiodduration"));
		createchargingprofilechargingrateunit=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilechargingrateunit"));
		createchargingprofileminchargingrate=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofileminchargingrate"));
		createchargingprofilestartperiod1=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilestartperiod1"));
		createchargingprofilelimit1=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilelimit1"));
		createchargingprofilenumphases1=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilenumphases1"));
		createchargingprofilestartperiod2=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilestartperiod2"));
		createchargingprofilelimit2=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilelimit2"));
		createchargingprofilenumphases2=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilenumphases2"));
		createchargingprofilestartperiod3=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilestartperiod3"));
		createchargingprofilelimit3=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilelimit3"));
		createchargingprofilenumphases3=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilenumphases3"));
		createchargingprofilebtn=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofilebtn"));
		createchargingprofileaddperiodbtn=GTK_WIDGET(gtk_builder_get_object(builder, "createchargingprofileaddperiodbtn"));



	    //SOLAPA 4.1 - DATA TRANSFER (5.6)
		chargepointsdatatransfercombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsdatatransfercombo"));
		datatransfermessageidtext=GTK_WIDGET(gtk_builder_get_object(builder, "datatransfermessageidtext"));
		datatransferdatatext=GTK_WIDGET(gtk_builder_get_object(builder, "datatransferdatatext"));
		datatransferbtn=GTK_WIDGET(gtk_builder_get_object(builder, "datatransferbtn"));
		g_signal_connect(G_OBJECT(datatransferbtn),"clicked",G_CALLBACK(sendDataTransfer),NULL);

		//SOLAPA 6.1 - GET LOCAL LIST
		chargepointsgetlocallistcombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsgetlocallistcombo"));
		getlocallistbtn=GTK_WIDGET(gtk_builder_get_object(builder, "getlocallistbtn"));
		g_signal_connect(G_OBJECT(getlocallistbtn),"clicked",G_CALLBACK(getLocalListVersion),NULL);

		//SOLAPA 6.2 (SEND LOCAL LIST)
		chargepointssendlocallistcombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointssendlocallistcombo"));
		sendlocallistcombo=GTK_WIDGET(gtk_builder_get_object(builder, "sendlocallistcombo"));
		sendlocallisttypecombo=GTK_WIDGET(gtk_builder_get_object(builder, "sendlocallisttypecombo"));

		sendlocallistbtn=GTK_WIDGET(gtk_builder_get_object(builder, "sendlocallistbtn"));
		g_signal_connect(G_OBJECT(chargepointssendlocallistcombo),"changed",G_CALLBACK(populateLocalLists),NULL);

		g_signal_connect(G_OBJECT(sendlocallistbtn),"clicked",G_CALLBACK(sendLocalList),NULL);

		//SOLAPA 6.3 (CREATE LOCAL LISTS)
		int currentLocalListRow=0;
		CreateListidTag[0]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag1"));
		CreateListidTag[1]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag2"));
		CreateListidTag[2]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag3"));
		CreateListidTag[3]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag4"));
		CreateListidTag[4]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag5"));
		CreateListidTag[5]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag6"));
		CreateListidTag[6]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag7"));
		CreateListidTag[7]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag8"));
		CreateListidTag[8]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListidTag9"));

		CreateListexpiryDate[0]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate1"));
		gtk_entry_set_text(GTK_ENTRY(CreateListexpiryDate[0]), getCurrentTime());
		CreateListexpiryDate[1]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate2"));
		CreateListexpiryDate[2]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate3"));
		CreateListexpiryDate[3]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate4"));
		CreateListexpiryDate[4]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate5"));
		CreateListexpiryDate[5]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate6"));
		CreateListexpiryDate[6]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate7"));
		CreateListexpiryDate[7]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate8"));
		CreateListexpiryDate[8]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListexpiryDate9"));

		CreateListparentIdTag[0]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag1"));
		CreateListparentIdTag[1]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag2"));
		CreateListparentIdTag[2]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag3"));
		CreateListparentIdTag[3]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag4"));
		CreateListparentIdTag[4]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag5"));
		CreateListparentIdTag[5]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag6"));
		CreateListparentIdTag[6]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag7"));
		CreateListparentIdTag[7]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag8"));
		CreateListparentIdTag[8]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListparentIdTag9"));

		CreateListStatus[0]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus1"));
		CreateListStatus[1]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus2"));
		CreateListStatus[2]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus3"));
		CreateListStatus[3]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus4"));
		CreateListStatus[4]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus5"));
		CreateListStatus[5]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus6"));
		CreateListStatus[6]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus7"));
		CreateListStatus[7]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus8"));
		CreateListStatus[8]=GTK_WIDGET(gtk_builder_get_object(builder, "CreateListStatus9"));

		createListName=GTK_WIDGET(gtk_builder_get_object(builder, "createListName"));
		createlocallistbtn=GTK_WIDGET(gtk_builder_get_object(builder, "createlocallistbtn"));
		createlocallistnextrowbtn=GTK_WIDGET(gtk_builder_get_object(builder, "createlocallistnextrowbtn"));
		createlocallistremoverowbtn=GTK_WIDGET(gtk_builder_get_object(builder, "createlocallistremoverowbtn"));

		g_signal_connect(G_OBJECT(createlocallistnextrowbtn),"clicked",G_CALLBACK(newLocalListRow),NULL);
		g_signal_connect(G_OBJECT(createlocallistremoverowbtn),"clicked",G_CALLBACK(removeLocalListRow),NULL);
		g_signal_connect(G_OBJECT(createlocallistbtn),"clicked",G_CALLBACK(createLocalList),NULL);


		//SOLAPA 7.1 - REMOTE START TRANSACTION
		remotestarttransactioncombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactioncombo"));
		remotestarttransactionconnectorcombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionconnectorcombo"));
		remotestarttransactionidtagcombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionidtagcombo"));

		remotestarttransactionchargingprofilecheckbox=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionchargingprofilecheckbox"));

		remotestarttransactionstacklevelcombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionstacklevelcombo"));
		remotestarttransactionpurposecombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionpurposecombo"));
		remotestarttransactionkindcombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionkindcombo"));
		remotestarttransactionrecurrencycombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionrecurrencycombo"));
		remotestarttransactionvalidfromtext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionvalidfromtext"));
		remotestarttransactionvalidtotext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionvalidtotext"));

		remotestarttransactiondurationtext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactiondurationtext"));
		remotestarttransactionstartscheduletext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionstartscheduletext"));
		remotestarttransactionchargingrateunitcombo=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionchargingrateunitcombo"));
		remotestarttransactionstartperiodtext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionstartperiodtext"));
		remotestarttransactionlimittext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionlimittext"));
		remotestarttransactionnumberphasestext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionnumberphasestext"));
		remotestarttransactionminchargingratetext=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionminchargingratetext"));

		remotestarttransactionbtn=GTK_WIDGET(gtk_builder_get_object(builder, "remotestarttransactionbtn"));

		g_signal_connect(G_OBJECT(remotestarttransactionchargingprofilecheckbox),"toggled",G_CALLBACK(activateStartTransactionWidgets),NULL);
		g_signal_connect(G_OBJECT(remotestarttransactioncombo),"changed",G_CALLBACK(updateConnectorsCombos),G_OBJECT(remotestarttransactionconnectorcombo));
		g_signal_connect(G_OBJECT(remotestarttransactioncombo),"changed",G_CALLBACK(readIdTags),G_OBJECT(remotestarttransactionidtagcombo));
		g_signal_connect(G_OBJECT(remotestarttransactionbtn),"clicked",G_CALLBACK(sendRemoteStartTransaction),NULL);

		//SOLAPA 7.2 - REMOTE STOP TRANSACTION
		remotestoptransactioncombo1=GTK_WIDGET(gtk_builder_get_object(builder, "remotestoptransactioncombo1"));
		remotestoptransactioncombo2=GTK_WIDGET(gtk_builder_get_object(builder, "remotestoptransactioncombo2"));
		remotestoptransactionbtn=GTK_WIDGET(gtk_builder_get_object(builder, "remotestoptransactionbtn"));

		gtk_widget_set_sensitive (remotestoptransactioncombo2, FALSE);
		gtk_widget_set_sensitive (remotestoptransactionbtn , FALSE);

		g_signal_connect(G_OBJECT(remotestoptransactioncombo1),"changed",G_CALLBACK(populateTransactions),G_OBJECT(remotestoptransactioncombo2));
		g_signal_connect(G_OBJECT(remotestoptransactioncombo2),"changed",G_CALLBACK(activateWidget),G_OBJECT(remotestarttransactionbtn));
		g_signal_connect(G_OBJECT(remotestoptransactionbtn),"clicked",G_CALLBACK(sendRemoteStopTransaction),NULL);

		//SOLAPA 8.1 - GET CONFIGURATION
		chargepointsgetconfigurationcombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsgetconfigurationcombo"));
		getconfigurationkeycombo=GTK_WIDGET(gtk_builder_get_object(builder, "getconfigurationkeycombo2"));
		getconfigurationkeytext=GTK_WIDGET(gtk_builder_get_object(builder, "getconfigurationkeytext"));
		getconfigurationbtn=GTK_WIDGET(gtk_builder_get_object(builder, "getconfigurationbtn"));
		getconfigurationaddtolistbtn=GTK_WIDGET(gtk_builder_get_object(builder, "getconfigurationaddtolistbtn"));

		g_signal_connect(G_OBJECT(getconfigurationbtn),"clicked",G_CALLBACK(getConfiguration),NULL);
		g_signal_connect(G_OBJECT(getconfigurationaddtolistbtn),"clicked",G_CALLBACK(getConfigurationAddKeyToList),NULL);

		for (int i=0; i<sizeof(clientConfigurationKeys)/sizeof(clientConfigurationKeys[0]); i++)
		{
			if ((clientConfigurationKeys[i]) && (strlen(clientConfigurationKeys[i])>0)) gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(getconfigurationkeycombo), clientConfigurationKeys[i]);
		}


	    //SOLAPA 8.2 - GET DIAGNOSTICS (5.9)
		chargepointsgetdiagnosticscombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsgetdiagnosticscombo"));
		diagnosticslocationtext=GTK_WIDGET(gtk_builder_get_object(builder, "diagnosticslocationtext"));
		diagnosticsretriestext=GTK_WIDGET(gtk_builder_get_object(builder, "diagnosticsretriestext"));
		diagnosticsretryintervaltext=GTK_WIDGET(gtk_builder_get_object(builder, "diagnosticsretryintervaltext"));
		diagnosticsstarttimetext=GTK_WIDGET(gtk_builder_get_object(builder, "diagnosticsstarttimetext"));
		diagnosticsstopttimetext=GTK_WIDGET(gtk_builder_get_object(builder, "diagnosticsstopttimetext"));
		getdiagnosticsbtn=GTK_WIDGET(gtk_builder_get_object(builder, "getdiagnosticsbtn"));
		getstarttimebtn=GTK_WIDGET(gtk_builder_get_object(builder, "getstarttimebtn"));
		getstoptimebtn=GTK_WIDGET(gtk_builder_get_object(builder, "getstoptimebtn"));

		g_signal_connect(G_OBJECT(getstarttimebtn),"clicked",G_CALLBACK(getCurrentTimeAct),diagnosticsstarttimetext);
		g_signal_connect(G_OBJECT(getstoptimebtn),"clicked",G_CALLBACK(getCurrentTimeAct),diagnosticsstopttimetext);

		g_signal_connect(G_OBJECT(getdiagnosticsbtn),"clicked",G_CALLBACK(getDiagnostics),NULL);

		//SOLAPA 9.1 - RESERVE NOW
		chargepointsavailabilitycombo2=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsavailabilitycombo2"));
		changeavailabilityconnectorscombo2=GTK_WIDGET(gtk_builder_get_object(builder, "connectorsavailabilitycombo2"));
		reservereservationidtext=GTK_WIDGET(gtk_builder_get_object(builder, "reservereservationidtext"));
		reservenowparentidtagtext=GTK_WIDGET(gtk_builder_get_object(builder, "reservenowparentidtagtext"));
		reservenowidtagcombo=GTK_WIDGET(gtk_builder_get_object(builder, "reservenowidtagcombo"));
		reservenowbtn=GTK_WIDGET(gtk_builder_get_object(builder, "reservenowbtn"));

		gtk_widget_set_sensitive (changeavailabilityconnectorscombo2 , FALSE);
		gtk_widget_set_sensitive (reservenowparentidtagtext , FALSE);
		gtk_widget_set_sensitive (reservenowidtagcombo , FALSE);
		gtk_widget_set_sensitive (reservereservationidtext , FALSE);
		gtk_widget_set_sensitive (reservenowbtn , FALSE);

		g_signal_connect(G_OBJECT(chargepointsavailabilitycombo2),"changed",G_CALLBACK(updateConnectorsCombos),G_OBJECT(changeavailabilityconnectorscombo2));
		g_signal_connect(G_OBJECT(chargepointsavailabilitycombo2),"changed",G_CALLBACK(activateWidget),G_OBJECT(reservereservationidtext));
		/////g_signal_connect(G_OBJECT(chargepointsavailabilitycombo2),"changed",G_CALLBACK(activateWidget),G_OBJECT(reservenowparentidtagtext));////
		g_signal_connect(G_OBJECT(chargepointsavailabilitycombo2),"changed",G_CALLBACK(readIdTags),G_OBJECT(reservenowidtagcombo));
		g_signal_connect(G_OBJECT(reservenowidtagcombo),"changed",G_CALLBACK(readParentIdTag),G_OBJECT(reservenowparentidtagtext));

		g_signal_connect(G_OBJECT(reservenowidtagcombo),"changed",G_CALLBACK(checkReserveNowButtonActivation),NULL);
		g_signal_connect(G_OBJECT(reservereservationidtext),"changed",G_CALLBACK(checkReserveNowButtonActivation),NULL);

		g_signal_connect(G_OBJECT(reservenowbtn),"clicked",G_CALLBACK(sendReserveNow),G_OBJECT(reservenowbtn));

		//SOLAPA 9.2 - UNLOCK CONNECTOR
		chargepointsunlockcombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsunlockcombo"));
		connectorsunlockcombo=GTK_WIDGET(gtk_builder_get_object(builder, "connectorsunlockcombo"));

		unlockconnectorbtn=GTK_WIDGET(gtk_builder_get_object(builder, "unlockconnectorbtn"));

		gtk_widget_set_sensitive (connectorsunlockcombo , FALSE);
		gtk_widget_set_sensitive (unlockconnectorbtn , FALSE);

		g_signal_connect(G_OBJECT(chargepointsunlockcombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(connectorsunlockcombo));
		g_signal_connect(G_OBJECT(chargepointsunlockcombo),"changed",G_CALLBACK(updateConnectorsCombos),G_OBJECT(connectorsunlockcombo));
		g_signal_connect(G_OBJECT(connectorsunlockcombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(unlockconnectorbtn));


		g_signal_connect(G_OBJECT(unlockconnectorbtn),"clicked",G_CALLBACK(sendUnlockConnector),G_OBJECT(reservenowbtn));


		//SOLAPA 10 - RESET
		chargepointsresetcombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsresetcombo"));
		chargepointsresettypecombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsresettypecombo"));
		chargepointsresetbtn=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsresetbtn"));

		gtk_widget_set_sensitive (chargepointsresettypecombo , FALSE);
		gtk_widget_set_sensitive (chargepointsresetbtn , FALSE);

		g_signal_connect(G_OBJECT(chargepointsresetcombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(chargepointsresettypecombo));
		g_signal_connect(G_OBJECT(chargepointsresettypecombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(chargepointsresetbtn));


		g_signal_connect(G_OBJECT(chargepointsresetbtn),"clicked",G_CALLBACK(sendReset),G_OBJECT(reservenowbtn));

		//SOLAPA 11 - TRIGGER MESSAGE
		chargepointstriggercombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointstriggercombo"));
		connectorstriggercombo=GTK_WIDGET(gtk_builder_get_object(builder, "connectorstriggercombo"));
		messagetriggercombo=GTK_WIDGET(gtk_builder_get_object(builder, "messagetriggercombo"));

		messagetriggerbtn=GTK_WIDGET(gtk_builder_get_object(builder, "messagetriggerbtn"));

		gtk_widget_set_sensitive (connectorstriggercombo , FALSE);
		gtk_widget_set_sensitive (messagetriggercombo , FALSE);

		g_signal_connect(G_OBJECT(chargepointstriggercombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(connectorstriggercombo));
		g_signal_connect(G_OBJECT(chargepointstriggercombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(messagetriggercombo));
		g_signal_connect(G_OBJECT(messagetriggercombo),"changed",G_CALLBACK(activateWidget),G_OBJECT(messagetriggerbtn));

		g_signal_connect(G_OBJECT(chargepointstriggercombo),"changed",G_CALLBACK(updateConnectorsCombos),G_OBJECT(connectorstriggercombo));

		g_signal_connect(G_OBJECT(messagetriggerbtn),"clicked",G_CALLBACK(sendTriggerMessage),G_OBJECT(reservenowbtn));

		//SOLAPA 12 - FIRMWARE UPDATE
		chargepointsfirmwarecombo=GTK_WIDGET(gtk_builder_get_object(builder, "chargepointsfirmwarecombo"));
		firmwarelocationtext=GTK_WIDGET(gtk_builder_get_object(builder, "firmwarelocationtext"));
		firmwareretriestext=GTK_WIDGET(gtk_builder_get_object(builder, "firmwareretriestext"));
		firmwareretryintervaltext=GTK_WIDGET(gtk_builder_get_object(builder, "firmwareretryintervaltext"));
		firmwareretrievedatetext=GTK_WIDGET(gtk_builder_get_object(builder, "firmwareretrievedatetext"));

		firmwarebtn=GTK_WIDGET(gtk_builder_get_object(builder, "firmwarebtn"));
		g_signal_connect(G_OBJECT(firmwarebtn),"clicked",G_CALLBACK(sendUpdateFirmware),NULL);




		/////////		LOGS
		LogTextView2=GTK_WIDGET(gtk_builder_get_object(builder, "LogTextView2"));

		////////

	    g_object_unref(builder);


	    setGUIData();

	    gtk_widget_show(window);
	    gtk_main();

	    return 0;
}

void on_window_main_destroy()
{
    gtk_main_quit();
}

