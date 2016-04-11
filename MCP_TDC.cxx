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
///   state = gOdb->odbReadInt("/runinfo/State");


#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <signal.h>

#include "vt2.h"
#include "MCP_TDC.h"
#include "Globals.h"
//GlobalHistos *gHistos = new GlobalHistos();

//extern TFolder* gManaHistosFolder;
//extern VirtualOdb* gOdb;

#define BUFSIZE 500000


// 1e5 data in milliseconds
// 1e2 data in microseconds
#define CONVERTIONFACTOR 1.e2

static TH1D *hstAbsHstart, *hstAbsHstop, *hstHit1, *hstHit2, *hstMltHit1, *hstMltHit2, *hstMltHit1Dstr, *hstMltHit2Dstr;
static TH2D *hst2DHit1, *hst2DHit2;
static TH2I *hst2DPos;
static TProfile *prfHit1, *prfHit2;
static uint32_t TDCb[BUFSIZE];
static int wp=0, nwp;
static uint32_t CycleNumber=0;
static uint32_t oldCycN=0;
static uint32_t firstCycle=1;
static int H2ave=0,H2total=0;

void HandleMCPTDC(TMidasEvent& event, void* ptr, int nitems)
{
  printf("MPET Bank Found.\n");
  static int i, j;
  uint32_t *data = (uint32_t*) ptr;

  printf("nitems %d\n", nitems);
	printf("wp: %d\n",wp);
  // Check limit
  if ((nitems >= BUFSIZE) || (nitems < 2))
    return;

  // Don't know yet
  if (gOutputFile)
    gOutputFile->cd();

  // Save new segment
  for (i=0 ; i<nitems ; i++) {
    TDCb[wp++] = data[i];
    if (wp >= BUFSIZE)
      {
	printf("buffer overflow at start!\n");
	break;
      }
  }
  nwp = 0;
  // Check if book done
  if (gHistos->prfHit2) {

    // Decode data
    uint32_t lTDC, TDC;
    uint32_t cycleSN=0, cycle1N, cycle2N, cyclePN;
    uint64_t hstart_time64, hit1_time64, hit2_time64, hstop_time64;
    int stopfound = 0, startfound = 0, ncomplete, ccomplete;
    int multiH1, multiH2;
    double CurrentFrequency, freqN;
    float calcTOF;
    
    // Count number of complete event for histo increment
    ccomplete = ncomplete = 0;
    for (i=0; i<wp; i++) {
      uint32_t header = TDCb[i] & 0xF0000000;
      if (header == VT2_HSTART)   startfound = 1;
      if (header == VT2_HSTOP) {
	startfound = 0;
	ncomplete++;
      }
    }
    
  printf(".....................ncomplete:%d wp:%d, nitems:%d, Serial:%d\n",
    ncomplete, wp, nitems, event.GetSerialNumber());
  printf("xxx --- Cycle number: %u\n",CycleNumber);
  if (ncomplete > 0) {
    hstart_time64 = hit1_time64 = hit2_time64 = hstop_time64 = 0;
    multiH1 = multiH2 = 0;
    startfound = 0;
    for (i=0; i<wp; i++) {
      TDC = TDCb[i];
      printf("--------------- data[%i] = 0x%x / 0x%x\n", i, TDCb[i], TDCb[i+1]);
      switch (TDC & 0xF0000000) {
      case VT2_HSTART: 
	startfound = 1;
	cycleSN      = ((TDC & VT2_CYCLE_MASK) >> VT2_CYCLE_SHIFT);
	printf("xxx --- CycleSN: %d --- oldCycN: %d\n",cycleSN,oldCycN);
	if(firstCycle) {
          firstCycle=0;
          CycleNumber=0;
	}
	else if(cycleSN>oldCycN)
	  CycleNumber += (cycleSN-oldCycN);
	else
	  CycleNumber += ((VT2_CYCLE_MASK>>VT2_CYCLE_SHIFT) + 1 + cycleSN-oldCycN);
	oldCycN=cycleSN;
	freqN = (double)((CycleNumber/gNDwellFreq)%gNFreq);
	//CurrentFrequency=gStartFreq + ((CycleNumber/gNDwellFreq)%gNFreq)*gFreqStep;
	CurrentFrequency = gStartFreq + freqN*gFreqStep;
	printf("xxx --- CYCLE: %d --- Frequency: %.3f --- freqN: %d --- fStep: %f\n",CycleNumber,CurrentFrequency, freqN, gFreqStep);
	lTDC = TDCb[++i];
	hstart_time64 =  ((((uint64_t)TDC & 0xFFFF)<<32) + lTDC); 
	printf("HSTART : CycleSN:%d  lTDC:%lld\n", cycleSN, hstart_time64);
	//hstAbsHstart->Fill((float) (hstart_time64/CONVERTIONFACTOR), 1.);
	break;
      case VT2_HIT1: 
	if (startfound) {
	  cycle1N      = ((TDC & VT2_CYCLE_MASK) >> VT2_CYCLE_SHIFT);
	  lTDC = TDCb[++i];
	  hit1_time64 = (((uint64_t)TDC & 0xFFFF)<<32) + lTDC;
	  if(cycle1N != cycleSN) break; // did not have a "start" for this hit
	  multiH1++;
	  printf("HIT1: Cycle1N:%d  lTDC:%lld\n", cycle1N, hit1_time64);
          calcTOF = (float) ((hit1_time64 - hstart_time64)/CONVERTIONFACTOR);
	  //hstHit1->Fill(calcTOF, 1.);
	  if(calcTOF > gMinTOF || calcTOF < gMaxTOF ) {
            //hst2DHit1->Fill(CurrentFrequency,calcTOF,1.);
	    //prfHit1->Fill(CurrentFrequency,calcTOF,1.);
            }
	}
	break;
      case VT2_HIT2: 
	if (startfound) {
	  cycle2N      = ((TDC & VT2_CYCLE_MASK) >> VT2_CYCLE_SHIFT);
	  lTDC = TDCb[++i];
	  if(cycle2N != cycleSN) break; // did not have a "start" for this hit
	  hit2_time64 = (((uint64_t)TDC & 0xFFFF)<<32) + lTDC; 
	  multiH2++;
	  printf("HIT2: Cycle2N:%d  lTDC:%lld\n", cycle2N, hit2_time64);
          calcTOF = (float) ((hit2_time64 - hstart_time64)/CONVERTIONFACTOR);
	  //hstHit2->Fill(calcTOF, 1.);
	  gHistos->hstHit2->Fill(calcTOF, 1.);
          if(calcTOF > gMinTOF || calcTOF < gMaxTOF) {
             //hst2DHit2->Fill(CurrentFrequency,calcTOF,1.);
             gHistos->hst2DHit2->Fill(CurrentFrequency-gCenterFreq,calcTOF,1.);
	     //prfHit2->Fill(CurrentFrequency-gCenterFreq,calcTOF,1.);
	     gHistos->prfHit2->Fill(CurrentFrequency-gCenterFreq,calcTOF,1.);
	     H2ave++;
	     H2total++;
             }
	}
	break;
      case VT2_HSTOP:
	if (startfound) {
	  cyclePN      = ((TDC & VT2_CYCLE_MASK) >> VT2_CYCLE_SHIFT);
	  lTDC = TDCb[++i];
	  nwp = i;
	  hstop_time64 = (((uint64_t)TDC & 0xFFFF)<<32) + (uint64_t)lTDC; 
	  printf("HSTOP: CyclePN:%d lTDC:%lld\n", cyclePN, hstop_time64);
	  //hstAbsHstop->Fill((float) (hstop_time64/CONVERTIONFACTOR), 1.);
	  //hstMltHit1->Fill((float) multiH1, 1.);
	  //hstMltHit2->Fill((float) multiH2, 1.);
	  gHistos->hstMltHit2->Fill((float) multiH2, 1.);
          //hstMltHit1Dstr->Fill(CurrentFrequency, multiH1);
          //hstMltHit2Dstr->Fill(CurrentFrequency-gCenterFreq, multiH2);
          gHistos->hstMltHit2Dstr->Fill(CurrentFrequency-gCenterFreq, multiH2);
	  //if( (CycleNumber % gAnalyzerAveCycles == 0) & !firstCycle) { //update averages
	  //  //odb->...;
	  //  H2ave=0;
	  //}
	  startfound = 0;
	  stopfound = 1;
	  multiH1 = multiH2 = 0;
	  ccomplete++;
	}
	break;
      }
      if (stopfound && (ccomplete >= ncomplete)) { stopfound = 0; printf("escape now\n"); break; }
    }
  }
  printf("Before move New wp:%d  startfound:%d\n", nwp, startfound);
  // Move data from last HSTART if HSTOP not found
  for (j = 0, i = nwp ; i < wp ; i++, j++) {
    TDCb[j] = TDCb[i];
    if (j>=BUFSIZE)
      {
	printf("buffer overflow!\n");
	break;
      }
  }
  wp = j;  // Adjust wp
  printf("After move wp:%d  startfound:%d\n", nwp, startfound);
  
  //mcptdc[0]->Draw();
  }
} 

void HandleMCPP(TMidasEvent& event, void* ptr, int nitems) {
	printf("MCPP Bank Found.\n");
	
	static int i;
	uint32_t *data = (uint32_t*) ptr;
	
	printf("nitems %d\n", nitems);
	printf("wp: %d\n",wp);
	
	// Check limt
	 if ((nitems >= BUFSIZE) || (nitems < 2))
     		return;
 	
 	// Don't know yet
   	if (gOutputFile)
    		gOutputFile->cd();
 
   	// Save new segment
   	for (i=0 ; i<nitems ; i++) {
    		TDCb[wp++] = data[i];
    		if (wp >= BUFSIZE)
      		{
			printf("buffer overflow at start!\n");
			break;
      		}
   	}
   	nwp = 0;
	
	// Check if booking done
	if (gHistos->prfHit2) {
		// Decode the Data
		//int stopfound = 0, startfound = 0, ncomplete, ccomplete;
		int x=-1, y=-1;
		
		for( i = 1; i < wp; i++ ) {
			x = (TDCb[i] & 0x0000ff00) >> 8;
			y = TDCb[i] & 0x000000ff;
			printf("POS: x = %d, y = %d\n",x,y);
			printf("--------------- data = 0x%x\n",TDCb[i]);
			//hst2DPos->Fill(x,y);
			gHistos->hst2DPos->Fill(x,y);
			x = y = -1;
		}
  	}
}

void HandleBOR_MCPTDC(int run, int time)
{
   CycleNumber=0;
   firstCycle=1;
   H2ave=0;
   H2total=0;
  
  printf(" in MCP_TDC BOR... Booking\n");

  if(gManaHistosFolder) {
	  gManaHistosFolder->Add(gHistos->prfHit2);
	  gManaHistosFolder->Add(gHistos->hstHit2);//
	  gManaHistosFolder->Add(gHistos->hstMltHit2);//
	  gManaHistosFolder->Add(gHistos->hstMltHit2Dstr);//
	  gManaHistosFolder->Add(gHistos->hst2DHit2);//
	  gManaHistosFolder->Add(gHistos->hst2DPos);//
	  //

	  //prfHit2 = (TProfile*)gHistos->prfHit2->Clone();
	  //gManaHistosFolder->Add(prfHit2);
	  //gManaHistosFolder->Add(gHistos->prfHit2->Clone());

	  //hstHit2 = (TH1D*)gHistos->hstHit2->Clone();
	  //gManaHistosFolder->Add(hstHit2);
	  //gManaHistosFolder->Add(gHistos->hstHit2->Clone());//

	  //hstMltHit2 = (TH1D*)gHistos->hstMltHit2->Clone();
	  //gManaHistosFolder->Add(hstMltHit2);
	  //gManaHistosFolder->Add(gHistos->hstMltHit2->Clone());//

	  //hstMltHit2Dstr = (TH1D*)gHistos->hstMltHit2Dstr->Clone();
	  //gManaHistosFolder->Add(hstMltHit2Dstr);
	  //gManaHistosFolder->Add(gHistos->hstMltHit2Dstr->Clone());//

	  //hst2DHit2 = (TH2D*)gHistos->hst2DHit2->Clone();
	  //gManaHistosFolder->Add(hst2DHit2);
	  //gManaHistosFolder->Add(gHistos->hst2DHit2->Clone());//

	  //hst2DPos = (TH2I*)gHistos->hst2DPos->Clone();
	  //gManaHistosFolder->Add(hst2DPos);
	  //gManaHistosFolder->Add(gHistos->hst2DPos->Clone());//

	  printf(" in MCP_TDC BOR... Done Booking\n");
  }




  //gStyle->SetPalette(1);
  // Booking 
  //void* p = (TH1D*)gDirectory->Get("AbsHstart");
  //if (p == 0) {
  if(false){
    // Number of TDC Start Gates
    /*hstAbsHstart = new TH1D("AbsHstart", "Num TDC Start Gates", gNTOFBins, 0, gMaxTOF);
    //hstAbsHstart->Fill(0);
    if (gManaHistosFolder) {
      gManaHistosFolder->Add(hstAbsHstart);
    }*/

    // Number of TDC End Gates
    /*hstAbsHstop = new TH1D("AbsHstop", "Num TDC End Gates", gNTOFBins, 0, gMaxTOF);
    //hstAbsHstop->Fill(0);
    if (gManaHistosFolder)
      gManaHistosFolder->Add(hstAbsHstop);
    */

    //
    /*hstHit1 = new TH1D("RelHit1", "RelHit1", gNTOFBins, 0, gMaxTOF);
    //hstHit1->Fill(0);
    if (gManaHistosFolder)
      gManaHistosFolder->Add(hstHit1);
    */
    // TOF Resonance
    //prfHit2 = new TProfile("Resonance", "Resonance",gNFreq,
                //gStartFreq-gCenterFreq, gEndFreq-gCenterFreq,gMinTOF,gMaxTOF);
    //prfHit2->SetMarkerStyle(20);
    //prfHit2->SetMarkerColor(kRed);
    //prfHit2->SetMaximum(gMaxTOF);
    //prfHit2->SetMinimum(gMinTOF);
    //prfHit2->Fill(-10000., -10., 1.);
    //if (gManaHistosFolder)
      //gManaHistosFolder->Add(prfHit2);


    // TOF Spectrum
    //hstHit2 = new TH1D("TOFSpectrum", "TOF Histo", gNTOFBins, 0, gMaxTOF);
    //hstHit2->Fill(0);
    //if (gManaHistosFolder)
      //gManaHistosFolder->Add(hstHit2);

    //
    /*hstMltHit1 = new TH1D("Multi1", "Multiplicity Hit1", 10, 0, 10);
    //hstMltHit1->Fill(0);
    if (gManaHistosFolder)
      gManaHistosFolder->Add(hstMltHit1);
    */

    // Num Ions vs. Frequency
    //hstMltHit2Dstr= new TH1D("NIonsvsFreq", "Num Ions vs Freq", gNFreq,
        //gStartFreq-gCenterFreq, gEndFreq-gCenterFreq);
    //hstMltHit2Dstr->SetMarkerStyle(20); // Circles
    //hstMltHit2Dstr->Sumw2();
    //hstMltHit2Dstr->Fill(-10000.);
    //if (gManaHistosFolder)
        //gManaHistosFolder->Add(hstMltHit2Dstr);
    
    // Z Histo
    //hstMltHit2 = new TH1D("Z_Histo", "Z Histo", 10, 0, 10);
    //hstMltHit2 ->Fill(0);
    //if (gManaHistosFolder)
      //gManaHistosFolder->Add(hstMltHit2);

    // MCP Position data
    //hst2DPos= new TH2I("MCP_Pos","MCP Position",256,0,255,256,0,255);
    //hst2DPos->SetMarkerStyle(6);
    //hst2DPos->Draw("COLZ");
    //hst2DPos->SetDrawOption("hist COLZ");
    //hst2DPos->Fill(-10000., -10., 1.);
    //if (gManaHistosFolder)
        //gManaHistosFolder->Add(hst2DPos);

    /*hst2DHit1 = new TH2D("Hit1_2D", "Hit1 rel 2D histo", gNFreq,
      gStartFreq-gFreqStep/2, gEndFreq+gFreqStep/2, gNTOFBins, 0, gMaxTOF);
    //hst2DHit1->Fill(-10000., -10., 1.);
    if (gManaHistosFolder)
      gManaHistosFolder->Add(hst2DHit1);
    */

    // TOF Matrix
    //hst2DHit2 = new TH2D("TOFMatrix", "TOF Matrix", gNFreq,
      //gStartFreq-gFreqStep/2, gEndFreq+gFreqStep/2, gNTOFBins,gMinTOF, gMaxTOF);
    //hst2DHit2->SetDrawOption("COLZ");
    //hst2DHit2->Fill(-10000., -10., 1.);
    //if (gManaHistosFolder)
      //gManaHistosFolder->Add(hst2DHit2);

    /*prfHit1 = new TProfile("Hit1Pf", "Hit1 rel profile",gNFreq,
		gStartFreq-gFreqStep/2, gEndFreq+gFreqStep/2);
    //prfHit1->Fill(-10000., -10., 1.);
    if (gManaHistosFolder)
      gManaHistosFolder->Add(prfHit1);
    */

    //
    /*hstMltHit1Dstr = new TH1D("Mlt1Distr", "Mlt1Distr", gNFreq,
	gStartFreq-gFreqStep/2, gEndFreq+gFreqStep/2);
    //hstMltHit1Dstr->Fill(-10000.);
    if (gManaHistosFolder)
        gManaHistosFolder->Add(hstMltHit1Dstr);
    */

    printf(" in MCP_TDC BOR... Booking Done ....\n");
  }
  else { // since histos are booked, make sure that the parameters are ok
  }
  /*
  // Reset the histograms to clear data from the previous run
	//hstAbsHstart->Reset();
	//hstAbsHstop->Reset();
	//hstHit1->Reset();
	hstHit2->Reset();//
	//hstMltHit1->Reset();
	hstMltHit2->Reset();//
	//hstMltHit1Dstr->Reset();
	hstMltHit2Dstr->Reset();//
	//hst2DHit1->Reset();
	hst2DHit2->Reset();//
	hst2DPos->Reset();//
	//prfHit1->Reset();
	prfHit2->Reset();//
  */
}

void HandleEOR_MCPTDC(int run, int time)
{
  printf(" in MCP_TDC EOR\n");

}

