//
// ROOT analyzer
//
// MCP TDC handling
//
// $Id$
//

/// \mainpage
///
/// \section intro_sec Introduction
///
///
/// \section features_sec Features
///
///

#ifndef _MCP_TDC_H_
#define _MCP_TDC_H_

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <signal.h>

//#include "TMidasOnline.h"
#include "TMidasEvent.h"
#include "TMidasFile.h"
#include "XmlOdb.h"
#include "midasServer.h"

#include <TSystem.h>
#include <TApplication.h>
#include <TTimer.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TFolder.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>

#include "Globals.h"

void HandleMCPTDC(TMidasEvent& event, void* ptr, int wsize);
void HandleMCPP(TMidasEvent& event, void* ptr, int wsize); 
void HandleBOR_MCPTDC(int run, int time);
void HandleEOR_MCPTDC(int run, int time);

#endif
