
// AC64.h: Hauptheaderdatei f�r die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'stdafx.h' vor dieser Datei f�r PCH einschlie�en"
#endif

#include "resource.h"		// Hauptsymbole


// CAC64App:
// Siehe AC64.cpp f�r die Implementierung dieser Klasse
//

class CAC64App : public CWinApp
{
public:
	CAC64App();

// �berschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CAC64App theApp;