//
// ROOT analyzer
//
// K.Olchanski
//
// $Id$
//

/// \mainpage
///
/// \section intro_sec Introduction
///
/// This "ROOT analyzer" package is a collection of C++ classes to
/// simplify online and offline analysis of data
/// collected using the MIDAS data acquisition system.
///
/// To permit standalone data analysis in mobile or "home institution"
/// environments, this package does not generally require that MIDAS
/// itself be present or installed.
///
/// It is envisioned that the user will use this package to develop
/// their experiment specific analyzer using the online data
/// connection to a MIDAS experiment. Then they could copy all the code
/// and data (.mid files) to their laptop and continue further analysis
/// without depending on or requiring installation of MIDAS software.
///
/// It is assumed that data will be analyzed using the ROOT
/// toolkit. However, to permit the most wide use of this
/// package, most classes do not use or require ROOT.
///
/// \section features_sec Features
///
/// - C++ classes for reading MIDAS events from .mid files
/// - C++ classes for reading MIDAS events from a running
/// MIDAS experiment via the mserver or directly from the MIDAS
/// shared memory (this requires linking with MIDAS libraries).
/// - C++ classes for accessing ODB data from .mid files
/// - C++ classes for accessing ODB from MIDAS shared memory
/// (this requires linking with MIDAS libraries).
/// - an example C++ analyzer main program
/// - the example analyzer creates a graphical ROOT application permitting full
/// use of ROOT graphics in both online and offline modes.
/// - for viewing "live" histograms using the ROODY graphical histogram viewer,
/// included is the "midasServer" part of the MIDAS analyzer.
///
/// \section links_sec Links to external packages
///
/// - ROOT data analysis toolkit: http://root.cern.ch
/// - MIDAS data acquisition system: http://midas.psi.ch
/// - ROODY graphical histogram viewer: http://daq-plone.triumf.ca/SR/ROODY/
///
/// \section starting_sec Getting started
///
/// - "get" the sources: svn checkout svn://ladd00.triumf.ca/rootana/trunk rootana
/// - cd rootana
/// - make
/// - make dox (generate this documentation); cd html; mozilla index.html
///  

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <signal.h>

#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "TMidasOnline.h"
#include "TMidasEvent.h"
#include "TMidasFile.h"
#include "XmlOdb.h"
#include "midasServer.h"
#include "TROOT.h"


#include <TSystem.h>
#include <TApplication.h>
#include <TTimer.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TFolder.h>
#include "THttpServer.h"

#include "Globals.h"

#include "MCP_TDC.h"

// Global Variables
int  gRunNumber = 0;
bool gIsRunning = false;
bool gIsPedestalsRun = false;
bool gIsOffline = false;
int  gEventCutoff = 0;
// Variables related to the frequency scan

double gCenterFreq=1;
double gFreqWidth=0.1;
double gFreqStep;
int gNFreq;
int gNDwellFreq=1;
double gMinTOF=10; // in microseconds
double gMaxTOF=100; // in microseconds
double gMinTOFRes=10; // in microseconds
double gMaxTOFRes=100; // in microseconds
double gTOFBinSize=0.1; // in microseconds
int gNTOFBins;
int gAnalyzerAveCycles;
double gStartFreq;
double gEndFreq;

//TH1D *hstAbsHstart, *hstAbsHstop, *hstHit1, *hstHit2, *hstMltHit1, *hstMltHit2, *hstMltHit1Dstr, *hstMltHit2Dstr;
//TH2D *hst2DHit1, *hst2DHit2;
//TH2I *hst2DPos;
//TProfile *prfHit1, *prfHit2;
GlobalHistos *gHistos;
THttpServer *serv;

TFile* gOutputFile = NULL;
VirtualOdb* gOdb = NULL;

//TCanvas  *gMainWindow = NULL; 	// the online histogram window

double GetTimeSec()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec + 0.000001*tv.tv_usec;
}

class MyPeriodic : public TTimer
{
public:
  typedef void (*TimerHandler)(void);

  int          fPeriod_msec;
  TimerHandler fHandler;
  double       fLastTime;

  MyPeriodic(int period_msec,TimerHandler handler)
  {
    assert(handler != NULL);
    fPeriod_msec = period_msec;
    fHandler  = handler;
    fLastTime = GetTimeSec();
    Start(period_msec,kTRUE);
  }

  Bool_t Notify()
  {
    double t = GetTimeSec();
    //printf("timer notify, period %f should be %f!\n",t-fLastTime,fPeriod_msec*0.001);

    if (t - fLastTime >= 0.9*fPeriod_msec*0.001)
      {
	//printf("timer: call handler %p\n",fHandler);
	if (fHandler)
	  (*fHandler)();
       fLastTime = t;
      }

    Reset();
    return kTRUE;
  }

  ~MyPeriodic()
  {
    TurnOff();
  }
};


void endRun(int, int, int);

void startRun(int transition,int run,int time)
{
  if(gIsRunning) {
	  endRun(0, run - 1, 0);
  }
  gIsRunning = true;
  gRunNumber = run;
  //  gIsPedestalsRun = gOdb->odbReadBool("/experiment/edit on start/Pedestals run");
  // printf("Begin run: %d, pedestal run: %d\n", gRunNumber, gIsPedestalsRun);
  
  gCenterFreq = gOdb->odbReadDouble(ODB_CENTERFREQ);
  gFreqWidth  = gOdb->odbReadDouble(ODB_FREQWIDTH);
  gNFreq      = gOdb->odbReadInt(ODB_NFREQ,0,1);
  //gFreqStep   = 2.0*gFreqWidth/(gNFreq-1);;

  gNDwellFreq = gOdb->odbReadInt(ODB_NDWELLFREQ,0,1);
  
  gMinTOF     = gOdb->odbReadDouble(ODB_ANALYZERMINTOF,0,0.);
  //gMaxTOF     = gOdb->odbReadDouble(ODB_ANALYZERMAXTOF,0,200.);
  gMaxTOF     = gOdb->odbReadDouble(ODB_ANALYZERMAXTOF,0,0.500) * 1000.;

  gMinTOFRes     = gOdb->odbReadDouble(ODB_ANALYZERMINTOFRES,0,0.);
  gMaxTOFRes     = gOdb->odbReadDouble(ODB_ANALYZERMAXTOFRES,0,200.);

  gTOFBinSize = gOdb->odbReadDouble(ODB_ANALYZERTOFBINSIZE,0,1.0);
  gNTOFBins   = (int)((gMaxTOF-gMinTOF)/gTOFBinSize);
  gAnalyzerAveCycles = gOdb->odbReadInt(ODB_ANALYZERAVECYCLES,0,100);
  
  gStartFreq = gOdb->odbReadDouble(ODB_STARTFREQ)*1e6;
  gEndFreq = gOdb->odbReadDouble(ODB_ENDFREQ)*1e6;
  
  gCenterFreq = (gStartFreq + gEndFreq)/2.0;
  gFreqWidth = (gEndFreq - gStartFreq)/2.0;
  //gCenterFreq = gOdb->odbReadDouble(ODB_CENTERFREQUENCY);
  //gFreqWidth = gOdb->odbReadDouble(ODB_FREQUENCYWIDTH);
  //gStartFreq = gCenterFreq - gFreqWidth;
  //gEndFreq = gCenterFreq + gFreqWidth;
  gFreqStep   = 2.0*gFreqWidth/((double)(gNFreq-1));
  
  // ugly defaults:
  if( gMaxTOF == 0.0 ) {
  	gMinTOF = 10.0;
  	gMaxTOF = 200.0;
  	gTOFBinSize = 0.2;
  	gNTOFBins   = (int)((gMaxTOF-gMinTOF)/gTOFBinSize);
  }
  
  printf("Center Freq: %lg, Frequency Scan Width: %lg, Step: %lg\n"
    "NFreq: %d, NDwell: %d\n"
    "Min TOF: %lg, Max TOF: %lg, NBins: %d\n",
    gCenterFreq,gFreqWidth,gFreqStep,gNFreq,gNDwellFreq,gMinTOF,gMaxTOF,gNTOFBins);

  if(gHistos) {
	  delete gHistos;
	  gHistos = 0;
  }

  if(!gHistos) {
	  gHistos = new GlobalHistos();
	  delete serv;
	  serv = new THttpServer("http:8084");
	  //printf("X max: %f\n", gHistos->prfHit2->GetXaxis()->GetXmax());
	  //printf("gFreqWidth: %f\n", gFreqWidth);
	  //serv->Register("/", gHistos->prfHit2); // Resonance
	  //serv->Register("/", gHistos->hstHit2); // TOF Histogram
	  //serv->Register("/", gHistos->hstMltHit2Dstr); // N ions vs Freq
	  //serv->Register("/", gHistos->hstMltHit2 ); // Z Histo
	  //serv->Register("/", gHistos->hst2DHit2); // TOF Matrix
	  //serv->Register("/", gHistos->hst2DPos); // MCP Position 
	  
	  //serv->SetItemField("/", "_monitoring", "2000");
	  //serv->SetItemField("/", "_layout", "tabs");
	  //serv->SetItemField("/", "_drawitem", "[Resonance,TOFSpectrum,NIonsvsFreq,TOFMatrix,MCP_Pos]");
  }

    
  if(gOutputFile!=NULL)
  {
    gOutputFile->Write();
    gOutputFile->Close();
    delete gOutputFile;
    gOutputFile=NULL;
  }  

  char filename[1024];
  sprintf(filename, "output%05d.root", run);
  //gOutputFile = new TFile(filename,"RECREATE"); 
  //serv->Unregister(gOutputFile);
  //gOutputFile->Add(gManaHistosFolder);

  //gHistos->reset();
  //gHistos->resizeBins();
  HandleBOR_MCPTDC(run, time);
}

void endRun(int transition,int run,int time)
{

  HandleEOR_MCPTDC(run, time);

  gIsRunning = false;
  gRunNumber = run;

  if (gManaHistosFolder)
    gManaHistosFolder->Clear();
  
  if (gOutputFile)
    {
      gOutputFile->Write();
      gOutputFile->Close();		//close the histogram file
      delete gOutputFile;
      gOutputFile = NULL;
    }

  printf("End of run %d\n",run);
}

#include <TH1D.h>
TH1D* samplePlot[10];

void HandleSample(int ichan, void* ptr, int wsize)
{
  uint16_t *samples = (uint16_t*) ptr;
  int numSamples = wsize;
  
  printf("numSamples:%d Data:%x\n", numSamples, samples[0]);

  if (numSamples > 512)
    return;

  char name[256];
  sprintf(name, "channel%d", ichan);
  printf("Name : %s\n", name);

  if (gOutputFile)
    gOutputFile->cd();

  
  samplePlot[0] = (TH1D*)gDirectory->Get(name);
  if (samplePlot[0] == 0)
    {
      for (int i=0;i<10;i++) {	
	sprintf(name, "channel%d", i);
	printf("Create [%s]\n", name);
	samplePlot[i] = new TH1D(name, name, 1000, 0, 2000);
	//samplePlot->SetMinimum(0);
	if (gManaHistosFolder)
	  gManaHistosFolder->Add(samplePlot[i]);
      }
    }

  for (int i=0;i<numSamples;i++)
    samplePlot[i]->Fill(samples[i], 1.);


  /*
  for(int ti=0; ti<numSamples; ti++)
    samplePlot->SetBinContent(ti, samples[ti]);
  */
}

void HandleMidasEvent(TMidasEvent& event)
{
  int eventId = event.GetEventId();

  if ((eventId == 1))
    {
      void *ptr;
      int size = event.LocateBank(event.GetData(), "MPET", &ptr);
      if (ptr) {
	HandleMCPTDC(event, ptr, size);
      }
      void *ptr2;
      size = event.LocateBank(event.GetData(),"MCPP", &ptr2);
      if (ptr2) {
      	//printf("MPETMCPP Bank Found.\n");
	HandleMCPP(event, ptr2, size);
      }
    }
  else if ((eventId == 0xB))
    {
      // ignore this event type
    }
  else if ((eventId == 0xD))
    {
      // ignore this event type
    }
  else
    {
      // unknown event type
      event.SetBankList();
      event.Print();
    }
}

void eventHandler(const void*pheader,const void*pdata,int size)
{
  TMidasEvent event;
  //memcpy(event.GetEventHeader(), pheader, sizeof(EventHeader_t));
  memcpy(event.GetEventHeader(), pheader, sizeof(TMidas_EVENT_HEADER));
  event.SetData(size, (char*)pdata);
  event.SetBankList();
  HandleMidasEvent(event);
}

int ProcessMidasFile(TApplication*app,const char*fname)
{
  TMidasFile f;
  bool tryOpen = f.Open(fname);

  if (!tryOpen)
    {
      printf("Cannot open input file \"%s\"\n",fname);
      return -1;
    }

  int i=0;
  while (1)
    {
      TMidasEvent event;
      if (!f.Read(&event))
	break;

      int eventId = event.GetEventId();
      //printf("Have an event of type %d\n",eventId);

      if ((eventId & 0xFFFF) == 0x8000)
	{
	  // begin run
	  //event.SetBankList();
	  //event.Print();

	  //char buf[256];
	  //memset(buf,0,sizeof(buf));
	  //memcpy(buf,event.GetData(),255);
	  //printf("buf is [%s]\n",buf);

	  //
	  // Load ODB contents from the ODB XML file
	  //
	  if (gOdb)
	    delete gOdb;
	  gOdb = new XmlOdb(event.GetData(),event.GetDataSize());

	  startRun(0,event.GetSerialNumber(),0);
	}
      else if ((eventId & 0xFFFF) == 0x8001)
	{
	  // end run
	  //event.SetBankList();
	  //event.Print();
	}
      else
	{
	  event.SetBankList();
	  //event.Print();
	  HandleMidasEvent(event);
	}
	
      if((i%500)==0)
	{
	  //resetClock2time();
	  printf("Processing event %d\n",i);
	  //SISperiodic();
	  //StepThroughSISBuffer();
	}
      
      i++;
      if ((gEventCutoff!=0)&&(i>=gEventCutoff))
	{
	  printf("Reached event %d, exiting loop.\n",i);
	  break;
	}
    }
  
  f.Close();

  endRun(0,gRunNumber,0);

  // start the ROOT GUI event loop
  //  app->Run(kTRUE);

  return 0;
}

#ifdef HAVE_MIDAS

void MidasPollHandler()
{
  if (!(TMidasOnline::instance()->poll(0)))
    gSystem->ExitLoop();
}

int ProcessMidasOnline(TApplication*app)
{
   TMidasOnline *midas = TMidasOnline::instance();

   //int err = midas->connect(NULL,"mpet","rootana");
   int err = midas->connect("localhost","mpet","rootana");
   assert(err == 0);

   gOdb = midas;

   midas->setTransitionHandlers(startRun,endRun,NULL,NULL);
   midas->registerTransitions();

   /* reqister event requests */

   midas->setEventHandler(eventHandler);
   midas->eventRequest("SYSTEM",-1,-1,(1<<1));

   /* fill present run parameters */

   gRunNumber = gOdb->odbReadInt("/runinfo/Run number");

   if ((gOdb->odbReadInt("/runinfo/State") == 3))
     startRun(0,gRunNumber,0);

   printf("Startup: run %d, is running: %d, is pedestals run: %d\n",gRunNumber,gIsRunning,gIsPedestalsRun);
   
   MyPeriodic tm(100,MidasPollHandler);
   //MyPeriodic th(1000,SISperiodic);
   //MyPeriodic tn(1000,StepThroughSISBuffer);
   //MyPeriodic to(1000,Scalerperiodic);

   /*---- start main loop ----*/

   //loop_online();
   app->Run(kTRUE);

   /* disconnect from experiment */
   midas->disconnect();

   return 0;
}

#endif

static bool gEnableShowMem = false;

int ShowMem(const char* label)
{
  if (!gEnableShowMem)
    return 0;

  FILE* fp = fopen("/proc/self/statm","r");
  if (!fp)
    return 0;

  int mem = 0;
  fscanf(fp,"%d",&mem);
  fclose(fp);

  if (label)
    printf("memory at %s is %d\n", label, mem);

  return mem;
}

void help()
{
  printf("\nUsage:\n");
  printf("\n./analyzer.exe [-h] [-eMaxEvents] [-pTcpPort] [-m] [-g] [-bMinTOF] [-cMaxTOF] [file1 file2 ...]\n");
  printf("\n");
  printf("\t-h: Print this help message\n");
  printf("\t-p: Start the midas histogram server on specified tcp port (for use with roody)\n");
  printf("\t-D: Start the analyzer in daemon mode. Only used for online analysis.");
  printf("\t-e: Number of events to read from input data files\n");
  printf("\t-m: Enable memory leak debugging\n");
  printf("\t-g: Enable graphics display when processing data files\n");
  printf("\t-b: Ignore TOF events under specified boundary (in microseconds)\n");
  printf("\t-c: Ignore TOF events beyond specified boundary (in microseconds)\n");
  printf("\n");
  printf("Example1: analyze online data: ./analyzer.exe -p9090\n");
  printf("Example2: analyze existing data: ./analyzer.exe /data/alpha/current/run00500.mid\n");
  exit(1);
}

// Main function call

int main(int argc, char *argv[])
{
   setbuf(stdout,NULL);
   setbuf(stderr,NULL);
 
   signal(SIGILL,  SIG_DFL);
   signal(SIGBUS,  SIG_DFL);
   signal(SIGSEGV, SIG_DFL);
 
   std::vector<std::string> args;
   for (int i=0; i<argc; i++)
     {
       args.push_back(argv[i]);
       if (strcmp(argv[i],"-h")==0)
	 help();
     }

   //gROOT->MakeBatch();
   TApplication *app = new TApplication("rootana", &argc, argv);
   //THttpServer serv("http:8084");
   serv = new THttpServer("http:8084");

   //startRun(0, 10, 0);
   //endRun(0, 10, 0);
   //HandleBOR_MCPTDC(0, 0);
   //HandleEOR_MCPTDC(0, 0);

   //serv.Register("/", prfHit2); // Resonance
   //serv.Register("/", hstHit2); // TOF Histogram
   //serv.Register("/", hstMltHit2Dstr); // N ions vs Freq
   //serv.Register("/", hst2DHit2); // TOF Matrix
   //serv.Register("/", hst2DPos); // MCP Position
   //serv.Register("/", gHistos->prfHit2); // Resonance

   //serv.SetItemField("/", "_monitoring", "2000");
   //serv.SetItemField("/", "_layout", "tabs");
   //serv.SetItemField("/", "_drawitem", "[Resonance,TOFSpectrum,NIonsvsFreq,TOFMatrix,MCP_Pos]");
   
   //if(gROOT->IsBatch()) {
   //	printf("Cannot run in batch mode\n");
   //   return 1;
   //}

   bool forceEnableGraphics = false;
   int  tcpPort = 0;
   bool daemonMode = false;

   for (unsigned int i=1; i<args.size(); i++) // loop over the commandline options
     {
       const char* arg = args[i].c_str();
       //printf("argv[%d] is %s\n",i,arg);
	   
       if (strncmp(arg,"-e",2)==0)  // Event cutoff flag (only applicable in offline mode)
	 gEventCutoff = atoi(arg+2);
       else if (strncmp(arg,"-m",2)==0) // Enable memory debugging
	 gEnableShowMem = true;
       else if (strncmp(arg,"-p",2)==0) // Set the histogram server port
	 tcpPort = atoi(arg+2);
       else if (strcmp(arg,"-g")==0)  
	 forceEnableGraphics = true;
       else if (strncmp(arg,"-b",2)==0) {
         gMinTOF = atof(arg+2);
       }
       else if (strncmp(arg,"-c",2)==0) {
         gMaxTOF = atof(arg+2);
       }
       else if (strncmp(arg,"-D",2)==0) {
         daemonMode = true;
       }
       else if (strcmp(arg,"-h")==0)  
	 help(); // does not return
       else if (arg[0] == '-')
	 help(); // does not return
    }
   
   if( daemonMode && tcpPort == 0 ) {
   	printf("Error: Daemon Mode started not in online mode. Please specify the port. Exit\n");
	exit(-1);
   }
   
   //if (tcpPort)
     //StartMidasServer(tcpPort);
	 
   gIsOffline = false;

   for (unsigned int i=1; i<args.size(); i++)
     {
       const char* arg = args[i].c_str();

       if (arg[0] != '-')  
	 {  
	   gIsOffline = true;
	   //gEnableGraphics = false;
	   //gEnableGraphics |= forceEnableGraphics;
	   ProcessMidasFile(app,arg);
	   //std::cout << "In here" << std::endl;
	 }
     }

   // if we processed some data files,
   // do not go into online mode.
   if (gIsOffline)
     return 0;
	   
   gIsOffline = false;
   //gEnableGraphics = true;
//#ifdef HAVE_MIDAS
//   pid_t pid, sid;
//    if( daemonMode ) {
//    	//std::cout << "ppid: " << getppid() << std::endl;
//    	if(getppid()==1) {
// 		printf("Another instance is already a daemon. Exit.\n");
// 	}
//    	pid = fork();
// 	printf("pid is: %d\n",pid);
// 	if( pid < 0 ) exit(-1);
// 	else if ( pid > 0 ) exit(0);
// 	sid = setsid();
// 	if(sid<0) exit(-1);
// 	
// 	pid = fork();
// 	printf("pid is: %d\n",pid);
// 	if( pid < 0 ) exit(-1);
// 	else if ( pid > 0 ) exit(0);
// 	
// 	umask(666);
// 	printf("Becoming a daemon.\n");
// 	close(STDIN_FILENO);
// 	close(STDOUT_FILENO);
// 	close(STDERR_FILENO);
// 	if (tcpPort)
//      		StartMidasServer(tcpPort);
//    	ProcessMidasOnline(app);
//    } else {
//    	if (tcpPort)
//      		StartMidasServer(tcpPort);
//    	ProcessMidasOnline(app);
//    }

   if( daemonMode ) {
	   printf("Becoming a daemon\n");
	   daemon(1,0);
	   if (tcpPort)
		   StartMidasServer(tcpPort);
	   ProcessMidasOnline(app);
   } else {
	   if (tcpPort)
		   StartMidasServer(tcpPort);
	   ProcessMidasOnline(app);
   }
//#endif
   
   return 0;
}

//end
