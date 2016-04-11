//
// Global variables for the ROOT analyzer
//
// Name: Globals.h
//
// $Id$
//
//#include <TH1D.h>
//#include <TH2I.h>
//#include <TH2D.h>
//#include <TProfile.h>

// Run parameters

extern int  gRunNumber;
extern bool gIsRunning;
extern bool gIsPedestalsRun;
extern bool gIsOffline;

//Variables related to the frequency scan ...

extern double gCenterFreq;
extern double gFreqWidth;
extern double gFreqStep;
extern int gNFreq;
extern int gNDwellFreq;
extern double gMinTOF;
extern double gMaxTOF;
extern double gTOFBinSize;
extern int gNTOFBins;
extern int gAnalyzerAveCycles;
extern double gStartFreq;
extern double gEndFreq;

extern double gMinTOFRes;
extern double gMaxTOFRes;

//extern TH1D *hstAbsHstart, *hstAbsHstop, *hstHit1, *hstHit2, *hstMltHit1, *hstMltHit2, *hstMltHit1Dstr, *hstMltHit2Dstr;
//extern TH2D *hst2DHit1, *hst2DHit2;
//extern TH2I *hst2DPos;
//extern TProfile *prfHit1, *prfHit2;

//... and their ODB paths

#define ODB_CENTERFREQ   "/Experiment/Variables/CenterFreq (Hz)"
#define ODB_FREQWIDTH    "/Experiment/Variables/ScanWidth (Hz)"
//#define ODB_NFREQ        "/Experiment/Variables/NFreq"
#define ODB_NFREQ        "/Equipment/TITAN_ACQ/ppg cycle/begin_ramp/loop count"
#define ODB_STARTFREQ	 "/Experiment/Variables/StartFreq (MHz)"
#define ODB_ENDFREQ	 "/Experiment/Variables/EndFreq (MHz)"

#define ODB_NDWELLFREQ         "/Experiment/Variables/Analyzer/Input/NDwell"
//#define ODB_ANALYZERMAXTOF     "/Experiment/Variables/Analyzer/Input/MaxTOF (us)"
#define ODB_ANALYZERMAXTOF     "/Equipment/TITAN_ACQ/ppg cycle/pul_TDCGate/pulse width (ms)"
#define ODB_ANALYZERMINTOF     "/Experiment/Variables/Analyzer/Input/MinTOF (us)"
#define ODB_ANALYZERMINTOFRES  "/Experiment/Variables/Analyzer/Input/MinTOF (us)"
#define ODB_ANALYZERMAXTOFRES  "/Experiment/Variables/Analyzer/Input/MaxTOF (us)"
#define ODB_ANALYZERTOFBINSIZE "/Experiment/Variables/Analyzer/Input/TOFBinSize (us)"
#define ODB_ANALYZERAVECYCLES  "/Experiment/Variables/Analyzer/Input/AveragingCycles"

#define ODB_ANALYZEROUTAVE       "/Experiment/Variables/Analyzer/Output/IonsAve"
#define ODB_ANALYZEREPICSAVE     "/Equipment/Beamline/Variables/Demand[1]"
#define ODB_ANALYZEROUTTOTAL     "/Experiment/Variables/Analyzer/Output/IonsTotal"
#define ODB_ANALYZEREPICSTOTAL   "/Equipment/Beamline/Variables/Demand[2]"

#define ODB_CENTERFREQUENCY      "/Experiment/Variables/Center Frequency"
#define ODB_FREQUENCYWIDTH      "/Experiment/Variables/Frequency Deviation"


// Output files

extern TFile* gOutputFile;

// ODB access

#include "VirtualOdb.h"

extern VirtualOdb* gOdb;

// end

#include "histos.h"

extern GlobalHistos *gHistos;
