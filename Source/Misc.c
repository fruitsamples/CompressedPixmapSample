/****************************//*      MISC ROUTINES       *//* By Brian Greenstone      *//****************************//***************//* EXTERNALS   *//***************/#include <Events.h>#include <Dialogs.h>#include <Processes.h>#include <NumberFormatting.h>#include <Timer.h>#include <math.h>#include <QD3D.h>#include <QD3DErrors.h>#include "myglobals.h"#include "misc.h"extern	EventRecord	gTheEvent;/****************************//*    CONSTANTS             *//****************************/#define		ERROR_ALERT_ID		401/**********************//*     VARIABLES      *//**********************//****************** DO SYSTEM ERROR ***************/void ShowSystemErr(long err){Str255		numStr;	NumToString(err, numStr);	DoAlert (numStr);	CleanQuit();}/*********************** DO ALERT *******************/void DoAlert(Str255 s){	ParamText(s,NIL_STRING,NIL_STRING,NIL_STRING);	NoteAlert(ERROR_ALERT_ID,nil);}		/*********************** DO FATAL ALERT *******************/void DoFatalAlert(Str255 s){	ParamText(s,NIL_STRING,NIL_STRING,NIL_STRING);	NoteAlert(ERROR_ALERT_ID,nil);	CleanQuit();}/************ CLEAN QUIT ***************/void CleanQuit(void){	ExitToShell();}/******************* FLOAT TO STRING *******************/void FloatToString(float num, Str255 string){Str255	sf;long	i,f;	i = num;						// get integer part			f = (fabs(num)-fabs((float)i)) * 10000;		// reduce num to fraction only & move decimal --> 5 places		if ((i==0) && (num < 0))		// special case if (-), but integer is 0	{		string[0] = 2;		string[1] = '-';		string[2] = '0';	}	else		NumToString(i,string);		// make integer into string			NumToString(f,sf);				// make fraction into string		string[++string[0]] = '.';		// add "." into string		if (f >= 1)	{		if (f < 1000)			string[++string[0]] = '0';	// add 1000's zero		if (f < 100)			string[++string[0]] = '0';	// add 100's zero		if (f < 10)			string[++string[0]] = '0';	// add 10's zero	}		for (i = 0; i < sf[0]; i++)	{		string[++string[0]] = sf[i+1];	// copy fraction into string	}}