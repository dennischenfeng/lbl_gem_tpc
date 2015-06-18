
void testCpp() {

	TCanvas *c1 = new TCanvas("c1","Test", 1000, 10, 900, 550);
	

	// HISTOGRAM

	// TH1F *h1 = new TH1F("h1", "Test", 10, 0, 10);
	
	// h1->Fill(3);
	// h1->Fill(3);
	// h1->Fill(3);
	// h1->Fill(3);
	// h1->Fill(4);
	// h1->Fill(5);
	// h1->Fill(5);
	// h1->Fill(5);
	// h1->Fill(6);
	// h1->Fill(7);
	// h1->Fill(7);
	// h1->Fill(8);
	// h1->Draw();
	// // h1->Fit("pol1", "W");
	// c1->Update();

	// TFitResultPtr fit1 = h1->Fit("pol1","S");
	// // TF1 *fit1 = h1->GetFunction("pol1");
	// // Double_t chi2 = fit1->GetChisquare();
	// // Double_t p0 = fit1->GetParameter(0);
	// // Double_t p1 = fit1->GetParameter(1);
	// // Double_t e0 = fit1->GetParError(0);
	// // Double_t e1 = fit1->GetParError(1);

	// // Double_t chi2 = fit1->GetChisquare();
	// Double_t p0 = fit1->Value(0);
	// Double_t p1 = fit1->Value(1);
	// Double_t e0 = fit1->Error(0);
	// Double_t e1 = fit1->Error(1);
	// // fit1->Print("V");

	// // cout << "chi2 " << chi2 << "\n";
	// cout << "p0 " << p0 << "\n"; 
	// cout << "p1 " << p1 << "\n";
	// cout << "e0 " << e0 << "\n";
	// cout << "e1 " << e1 << "\n";





	// TGRAPH

	const Int_t n = 6;
	Double_t x[n];
	Double_t y[n];
	x[0] = 3; y[0] = 4;
	x[1] = 4; y[1] = 4;
	x[2] = 5; y[2] = 5;
	x[3] = 6; y[3] = 6;
	x[4] = 8; y[4] = 8;
	x[5] = 0; y[5] = 0;

	TGraph *gr = new TGraph(n, x, y);
	// gr->SetLineColor(2);
	// gr->SetLineWidth(4);
	// gr->SetMarkerColor(4);
	// gr->SetMarkerStyle(21);
	// gr->SetTitle("a simple graph");
	// gr->GetXaxis()->SetTitle("X title");
	// gr->GetYaxis()->SetTitle("Y title");
	gr->Draw("p0");

	TFitResultPtr fit = gr->Fit("pol1", "WS");
	fit->Print("V");
	// TCanvas::Update() draws the frame, after which one can change it
	c1->Update();
	// c1->GetFrame()->SetFillColor(21);
	// c1->GetFrame()->SetBorderSize(12);
	// c1->Modified();






	// TGRAPH2D


}