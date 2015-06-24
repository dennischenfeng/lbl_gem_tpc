#include "TROOT.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TCanvas.h"
#include "TH2F.h"

#include "helper_functions.cpp"


void convert_raw_to_CRCalc() {
	/*** Creates a CERN ROOT file that contains a TTree that holds all the calculated data needed for analysis. Used when source is: cosmic rays (CR)

	Organized in the following branches: (name of branch, root type descriptor)
	1. h5_file_num, i, UInt_t
	2. SM_event_num, L, Long64_t (NOT same as h5 event num)
	3. num_hits, i, UInt_t
	4. sum_tots, i, UInt_t
	5. mean_x, D, Double_t
	6. mean_y, D, Double_t
	7. mean_z, D, Double_t
	8. line_fit_param0, D, Double_t (x-intercept when z = 0)
	9. line_fit_param1, D, Double_t (dx/dz)
	10. line_fit_param2, D, Double_t (y-intercept when z = 0)
	11. line_fit_param3, D, Double_t (dy/dz)
	12. sum_of_squares, D, Double_t (same as chi square with error = 1 for all pts)
	13. fraction_inside_sphere, D, Double_t (fraction of the hits that are within a sphere of radius 1 centered at the mean; if this is close to 1, it means the hits are like a dense blob instead of a long streak)
	
	Author: Dennis Feng	
	***/
	gROOT->Reset(); 

	// Setting up TTreeReader for input file
	UInt_t h5_file_num_input = 101;    // CHOOSE THIS

	// TFile *in_file = new TFile("/home/pixel/pybar/tags/2.0.2_new/pyBAR-master/pybar/module_202_new/100_module_202_new_stop_mode_ext_trigger_scan_interpreted_raw.root");
	TFile *in_file = new TFile(("/home/pixel/pybar/tags/2.0.2_new/pyBAR-master/pybar/module_202_new/" + to_string(h5_file_num_input) + "_module_202_new_stop_mode_ext_trigger_scan_interpreted_raw.root").c_str());

	TTreeReader *reader = new TTreeReader("Table", in_file);

	TTreeReaderValue<UInt_t> h5_file_num(*reader, "h5_file_num");
	TTreeReaderValue<Long64_t> event_number(*reader, "event_number");
	TTreeReaderValue<UChar_t> tot(*reader, "tot");
	TTreeReaderValue<UChar_t> relative_BCID(*reader, "relative_BCID");
	TTreeReaderValue<Long64_t> SM_event_num(*reader, "SM_event_num");
	TTreeReaderValue<Double_t> x(*reader, "x");
	TTreeReaderValue<Double_t> y(*reader, "y");
	TTreeReaderValue<Double_t> z(*reader, "z");


	// Create CR Calc file and TTree
	// TFile *out_file = new TFile("/home/pixel/pybar/tags/2.0.2_new/pyBAR-master/pybar/module_202_new/100_module_202_new_stop_mode_ext_trigger_scan_interpreted_CRCalc.root","RECREATE");
	TFile *out_file = new TFile(("/home/pixel/pybar/tags/2.0.2_new/pyBAR-master/pybar/module_202_new/" + to_string(h5_file_num_input) + "_module_202_new_stop_mode_ext_trigger_scan_interpreted_CRCalc.root").c_str(), "RECREATE");
	TTree *t = new TTree("Table","CR Calc Data");
	
	UInt_t h5_file_num_CRCalc = h5_file_num_input; // must be different name than the one from raw file
	Long64_t SM_event_num_CRCalc = 0; // must be different name than the one from raw file
	UInt_t num_hits = 0;
	UInt_t sum_tots = 0;
	Double_t mean_x = 0;
	Double_t mean_y = 0;
	Double_t mean_z = 0;
	Double_t line_fit_param0 = 0;
	Double_t line_fit_param1 = 0;
	Double_t line_fit_param2 = 0;
	Double_t line_fit_param3 = 0;
	Double_t sum_of_squares = 0;
	Double_t fraction_inside_sphere = 0;

	t->Branch("h5_file_num", &h5_file_num_CRCalc, "h5_file_num/i");
	t->Branch("SM_event_num", &SM_event_num_CRCalc, "SM_event_num/L");
	t->Branch("num_hits", &num_hits, "num_hits/i");
	t->Branch("sum_tots", &sum_tots, "sum_tots/i");
	t->Branch("mean_x", &mean_x, "mean_x/D");
	t->Branch("mean_y", &mean_y, "mean_y/D");
	t->Branch("mean_z", &mean_z, "mean_z/D");
	t->Branch("line_fit_param0", &line_fit_param0, "line_fit_param0/D");
	t->Branch("line_fit_param1", &line_fit_param1, "line_fit_param1/D");
	t->Branch("line_fit_param2", &line_fit_param2, "line_fit_param2/D");
	t->Branch("line_fit_param3", &line_fit_param3, "line_fit_param3/D");
	t->Branch("sum_of_squares", &sum_of_squares, "sum_of_squares/D");
	t->Branch("fraction_inside_sphere", &fraction_inside_sphere, "fraction_inside_sphere/D");




	// Calculations:
	// Initialize the graph
	TGraph2D *graph = new TGraph2D();

	// Variables used to loop the main loop
	bool endOfReader = false; // if reached end of the reader
	Long64_t smEventNum = 1; // the current SM-event, start at 1
	t->Fill(); // fills SM Event 0 with all zeroes. Assumes there's nothing in this event, which is probably true. Also assumes smEventNum starts at 1.
	
	// Main Loop (loops for every smEventNum)
	while (!endOfReader) {
		// Variables used in this main loop
		int startEntryNum = 0;
		int endEntryNum = 0;
		bool fitFailed = false; // true if the 3D fit failed
		bool lastEvent = false;

		// Declaring some important output values for the current graph and/or line fit
		UInt_t numEntries = 0;
		UInt_t sumTots = 0;
		double meanX = 0;
		double meanY = 0;
		double meanZ = 0;
		double param0 = 0;
		double param1 = 0;
		double param2 = 0;
		double param3 = 0;
		double sumSquares = 0;

		// Print out the smEventNum
		cout << "Calculating data for SM Event Num " << smEventNum << "\n";

		// Get startEntryNum and endEntryNum
		startEntryNum = getEntryNumWithSMEventNum(reader, smEventNum);
		endEntryNum = getEntryNumWithSMEventNum(reader, smEventNum + 1);

		if (startEntryNum == -2) { // can't find the exact smEventNum
			cout << "Error: There should not be any SM event numbers that are missing." << "\n";
		} else if (startEntryNum == -3) { 
			endOfReader = true;
			break;
		} else if (endEntryNum == -3) { // assuming no SM event nums are skipped
			endEntryNum = reader->GetEntries(false);
			lastEvent = true;
		}

		// Fill TGraph with points, get mean XYZ and sumTots
		graph = new TGraph2D(); // create a new TGraph to refresh

		reader->SetEntry(startEntryNum);
		for (int i = 0; i < endEntryNum - startEntryNum; i++) {
			graph->SetPoint(i, (*x - 0.001), (*y + 0.001), (*z - 0.001));
			
			// get mean XYZ and sumTots
			meanX += *x;
			meanY += *y;
			meanZ += *z;
			sumTots += *tot;

			endOfReader = !(reader->Next());
		}
		numEntries = endEntryNum - startEntryNum;
		meanX /= numEntries;
		meanY /= numEntries;
		meanZ /= numEntries;

		// 3D Fit, only accept "good" events, get input
		if (!endOfReader || lastEvent) {
			// Starting the fit. First, get decent starting parameters for the fit - do two 2D fits (one for x vs z, one for y vs z). 
			TGraph *graphZX = new TGraph();
			TGraph *graphZY = new TGraph();
			reader->SetEntry(startEntryNum);
			for (int i = 0; i < endEntryNum - startEntryNum; i++) {
				graphZX->SetPoint(i, (*z - 0.001), (*x + 0.001));
				graphZY->SetPoint(i, (*z - 0.001), (*y + 0.001));
				reader->Next();
			}


			TFitResultPtr fitZX = graphZX->Fit("pol1", "WQS"); // w for ignore error of each pt, q for quiet (suppress results output), s for return a tfitresultptr
			TFitResultPtr fitZY = graphZY->Fit("pol1", "WQS");
			Double_t initParam0 = fitZX->GetParams()[0];
			Double_t initParam1 = fitZX->GetParams()[1];
			Double_t initParam2 = fitZY->GetParams()[0];
			Double_t initParam3 = fitZY->GetParams()[1];

			// 3D FITTING CODE (based on line3Dfit.C), draw graph and line fit
			ROOT::Fit::Fitter  fitter;
		   	SumDistance2 sdist(graph);
#ifdef __CINT__
		   	ROOT::Math::Functor fcn(&sdist,4,"SumDistance2");
#else
		   	ROOT::Math::Functor fcn(sdist,4);
#endif
			// set the function and the initial parameter values
			double pStart[4] = {initParam0, initParam1, initParam2, initParam3};
			fitter.SetFCN(fcn,pStart);
			// set step sizes different than default ones (0.3 times parameter values)
			for (int i = 0; i < 4; ++i) fitter.Config().ParSettings(i).SetStepSize(0.01);

			bool ok = fitter.FitFCN();
			if (!ok) {
				Error("line3Dfit","Line3D Fit failed");
				fitFailed = true;
			} else if (numEntries == 1) {
				cout << "Error solved: The SM Event " << smEventNum << " has only 1 hit, so the line fit failed, and it was taken care of accordingly." << "\n";
				fitFailed = true;
			} else if (numEntries <= 0) {
				cout << "Error: The SM Event " << smEventNum << " causes numEntries <= 0, which should never be the case." << "\n";
			} else {
				const ROOT::Fit::FitResult & result = fitter.Result();
				const double * fitParams = result.GetParams();

				// Access fit params and minfcnvalue
				param0 = fitParams[0];
				param1 = fitParams[1];
				param2 = fitParams[2];
				param3 = fitParams[3];
				sumSquares = result.MinFcnValue();

				// std::cout << "Theta : " << TMath::ATan(sqrt(pow(fitParams[1], 2) + pow(fitParams[3], 2))) << std::endl;
			}

			// Variables for determining whether the current event is a "good" event
			// calculating fraction of hits inside the sphere of radius 1 (mm)
			double radius = 1; // length in mm 
			double fractionInsideSphere = 0;
			reader->SetEntry(startEntryNum);
			for (int i = 0; i < endEntryNum - startEntryNum; i++) {
				double distanceFromMeanXYZ = sqrt(pow(graph->GetX()[i] - meanX, 2) + pow(graph->GetY()[i] - meanY, 2) + pow(graph->GetZ()[i] - meanZ, 2));
				if (distanceFromMeanXYZ <= 2) {
					fractionInsideSphere += 1;
				}
				reader->Next();
			}
			fractionInsideSphere /= endEntryNum - startEntryNum;


			// Fill the TTree
			if (fitFailed) { // if fit failed, these variables are 0
				line_fit_param0 = 0;
				line_fit_param1 = 0;
				line_fit_param2 = 0;
				line_fit_param3 = 0;
				sum_of_squares = 0;
			}

			h5_file_num_CRCalc = h5_file_num_input;
			SM_event_num_CRCalc = smEventNum;
			num_hits = numEntries;
			sum_tots = sumTots;
			mean_x = meanX;
			mean_y = meanY;
			mean_z = meanZ;
			line_fit_param0 = param0;
			line_fit_param1 = param1;
			line_fit_param2 = param2;
			line_fit_param3 = param3;
			sum_of_squares = sumSquares;
			fraction_inside_sphere = fractionInsideSphere;

			t->Fill();
		}
		smEventNum++;
	}




	out_file->Write();
    out_file->Close();
}
