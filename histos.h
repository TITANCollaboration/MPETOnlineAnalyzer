#ifndef _HAVE_GLOBALHISTO_
#define _HAVE_GLOBALHISTO_

#include <TH1D.h>
#include <TProfile.h>
#include <TH2D.h>
#include <TH2I.h>

class GlobalHistos
{
	public:
		TH1D *hstHit2, *hstMltHit2, *hstMltHit2Dstr;
		TH2D *hst2DHit2;
		TH2I *hst2DPos;
		TProfile *prfHit2;
		
		GlobalHistos() {
			// Shift the start and stop frequencies by half a step,
			// so that the ROOT histogram bins are centered properly.
			double startF = gStartFreq - gCenterFreq - gFreqStep/2.0;
			double stopF = gEndFreq - gCenterFreq + gFreqStep/2.0;

			prfHit2 = new TProfile("Resonance", "Resonance",gNFreq,
					startF, stopF,gMinTOFRes,gMaxTOFRes);
			prfHit2->SetMarkerStyle(20);
			prfHit2->SetMarkerColor(kRed);
			prfHit2->SetMaximum(gMaxTOFRes);
			prfHit2->SetMinimum(gMinTOFRes);

			hstHit2 = new TH1D("TOFSpectrum", "TOF Histo", gNTOFBins, 0, gMaxTOF);

			hstMltHit2Dstr = new TH1D("NIonsvsFreq", "Num Ions vs Freq", gNFreq,
				startF, stopF);
			hstMltHit2Dstr->SetMarkerStyle(20); // Circles

			hstMltHit2 = new TH1D("Z_Histo", "Z Histo", 10, 0, 10);

			// TOF Matrix
			hst2DHit2 = new TH2D("TOFMatrix", "TOF Matrix", gNFreq,
					startF, stopF, gNTOFBins,gMinTOF, gMaxTOF);

			hst2DPos = new TH2I("MCP_Pos","MCP Position",256,0,255,256,0,255);
			hst2DPos->SetMarkerStyle(6);
		}
		
		~GlobalHistos() {
			if(hstHit2)
				delete hstHit2;//
			if(hstMltHit2)
				delete hstMltHit2;//
			if(hstMltHit2Dstr)
				delete hstMltHit2Dstr;//
			if(hst2DHit2)
				delete hst2DHit2;//
			if(hst2DPos)
				delete hst2DPos;//
			if(prfHit2)
				delete prfHit2;//
		}


		void dummyfill() {
			hstHit2->Reset();//
			hstMltHit2->Reset();//
			hstMltHit2Dstr->Reset();//
			hst2DHit2->Reset();//
			hst2DPos->Reset();//
			prfHit2->Reset();//
		}

		void resizeBins() {
			prfHit2->SetBins(gNFreq, gStartFreq-gCenterFreq, gEndFreq-gCenterFreq);//
			hstHit2->SetBins(gNTOFBins, 0, gMaxTOF);//
			hstMltHit2->SetBins(10, 0, 10);//
			hstMltHit2Dstr->SetBins(gNFreq, gStartFreq-gCenterFreq, gEndFreq-gCenterFreq);//
			hst2DHit2->SetBins(gNFreq, gStartFreq-gFreqStep/2, gEndFreq+gFreqStep/2, gNTOFBins,gMinTOF, gMaxTOF);//
			hst2DPos->SetBins(256,0,255,256,0,255);//
		}

		void reset() {
			hstHit2->Reset();//
			hstMltHit2->Reset();//
			hstMltHit2Dstr->Reset();//
			hst2DHit2->Reset();//
			hst2DPos->Reset();//
			prfHit2->Reset();//
		}
};
#endif
