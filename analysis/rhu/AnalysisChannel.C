void AnalysisChannel()
{
   // list of fills
   int fill = 5020;
   int timei = 1465926300;
   
   cout << "AnalysisChannel: Fill " << fill << " ..." << endl;

//   static const int ntrains = 11;
//   static const int ntrains = 21;
//   static const int ntrains = 25;
//   static const int ntrains = 26;
   static const int ntrains = 29;
   
   // 1465
//   int bx[ntrains] = {2,280,330,439,1224,1333,1782,2118,2227,3009,3118};
//   int bx[ntrains] = {169,218,327,436,545,654,894,1003,1112,1221,1330,1788,1897,2006,2115,2224,2682,2791,2900,3009,3118};
//   int bx[ntrains] = {61,109,218,327,436,545,785,894,1003,1112,1221,1330,1439,1679,1788,1897,2006,2115,2224,2573,2682,2791,2900,3009,3118};
//   int bx[ntrains] = {61,109,218,327,436,545,785,894,1003,1112,1221,1330,1439,1679,1788,1897,2006,2115,2224,2333,2573,2682,2791,2900,3009,3118};
   int bx[ntrains] = {62,111,220,329,438,547,656,786,895,1004,1113,1222,1331,1440,1549,1680,1789,1898,2007,2116,2225,2334,2443,2573,2682,2791,2900,3009,3118};
   TFile * f;
   if ( timei < 0 )
      f = new TFile(Form("/tmp/roberval/rhu/rhu_%i.root",fill), "read");
   else
      f = new TFile(Form("/tmp/roberval/rhu/rhu_%i_%i.root",fill,timei), "read");
   
   TTree * t = (TTree*) f -> Get("rhu");
   
   int time;
   float data[171072];
   float bxi1[3564];
   float bxi2[3564];
   
   t->SetBranchAddress("time", &time);
   t->SetBranchAddress("data", data);
   t->SetBranchAddress("bxi1", bxi1);
   t->SetBranchAddress("bxi2", bxi2);
   
   TH1F * orbit = new TH1F("orbit","",3564,0,3564);
   
//   TH1F * h_rate = new TH1F("h_rate","",5000,0,5000);
   
   
   TH1F * h_spec_rate_lumi[ntrains];
   TH1F * h_spec_rate_ch[48][ntrains];
   
   for ( int i = 0; i < ntrains; ++i )
   {
      h_spec_rate_lumi[i] = new TH1F(Form("h_spec_rate_lumi_%i",bx[i]),"",300,20,50);
      for ( int j =0 ; j < 48 ; ++j )
      {
         h_spec_rate_ch[j][i] = new TH1F(Form("h_spec_rate_ch_%i_%i",j,bx[i]),"",500,0,50);
      }
   }
   for ( int i = 0; i < t->GetEntries(); ++i )
   {
      t -> GetEntry(i);
      
      for ( int j = 0 ;j < ntrains; ++j)
      {
         float i1 = bxi1[bx[j]]/1.E10;
         float i2 = bxi2[bx[j]]/1.E10;
         float rate[48];
         for ( int k = 0; k < 48 ; ++k )
         {
            rate[k] = data[(k*3564)+bx[j]];
            h_spec_rate_ch[k][j]->Fill(rate[k]/(i1*i2));
         }
         float totalRate = rate[40]+rate[41]+rate[44]+rate[45];
         h_spec_rate_lumi[j]->Fill(totalRate/(i1*i2));
      }
   }
   
   TFile * fout;
   
   if ( timei < 0 )
      fout = new TFile(Form("analysis_perch_%i.root",fill),"recreate");
   else
      fout = new TFile(Form("analysis_perch_%i_%i.root",fill,timei),"recreate");
   
   double gbx[ntrains];
   double gbxe[ntrains];
   double mean[ntrains];
   double rms[ntrains];
   for ( int i = 0; i < ntrains; ++i )
   {
      TFitResultPtr r = h_spec_rate_lumi[i] -> Fit("gaus","S");
      h_spec_rate_lumi[i] -> Write();
      gbx[i] = double(bx[i]);
      gbxe[i] = 0.;
      mean[i] = r -> Value(1);
      rms[i]  = r -> Value(2);
   }
   
   TGraphErrors * gr_lumi = new TGraphErrors(ntrains,gbx,mean,gbxe,rms);
   
   gr_lumi -> SetMarkerStyle(20);
   gr_lumi -> GetXaxis() -> SetTitle("bx id");
   gr_lumi -> GetYaxis() -> SetTitle("<rate/(i1*i2)> (1E20)");
   gr_lumi -> GetYaxis() -> SetRangeUser(0,50);
   gr_lumi -> SetTitle("");
   gr_lumi -> Draw("AP");
   TFitResultPtr r = gr_lumi -> Fit("pol0","S");
   TF1 * fitf = gr_lumi -> GetFunction("pol0");
   double value = r -> Value(0);
   double error = r -> Error(0);
   
   TLegend * leg = new TLegend(0.5,0.7,0.9,0.9);
   leg->SetHeader(Form("Fill %i",fill));
   leg->AddEntry(gr_lumi,"data","p");
   leg->AddEntry(fitf,Form("const fit: %2.2f +- %1.2f", value, error),"l");

   leg -> Draw();
   
   gr_lumi -> Write();
   
   
   // PER CHANNEL
   TGraphErrors * gr_ch[48];
   for ( int ch = 0 ; ch < 48 ; ++ch )
   {
      for ( int i = 0; i < ntrains; ++i )
      {
         TFitResultPtr r = h_spec_rate_ch[ch][i] -> Fit("gaus","S");
         h_spec_rate_ch[ch][i] -> Write();
         gbx[i] = double(bx[i]);
         gbxe[i] = 0.;
         mean[i] = r -> Value(1);
         rms[i]  = r -> Value(2);
      }
      gr_ch[ch] = new TGraphErrors(ntrains,gbx,mean,gbxe,rms);
      gr_ch[ch] -> SetName(Form("gr_ch_%i",ch));
      gr_ch[ch] -> SetMarkerStyle(20);
      gr_ch[ch] -> GetXaxis() -> SetTitle("bx id");
      gr_ch[ch] -> GetYaxis() -> SetTitle("<rate/(i1*i2)> (1E20)");
      gr_ch[ch] -> GetYaxis() -> SetRangeUser(0,20);
      gr_ch[ch] -> SetTitle(Form("Channel %i",ch));
      TFitResultPtr r = gr_ch[ch] -> Fit("pol0","S");
      TF1 * fitf = gr_ch[ch] -> GetFunction("pol0");
      
      gr_ch[ch] -> Write();
      
   }
   
}


