void CheckRates()
{
   static const int nbx = 3564;
   static const int ntrains = 100;  // a large number, larger than usual number of trains
   
   int time;
   float dataid1[171072];
   float bxi1[3564];
   float bxi2[3564];
   float bxlumi[3564];
   
   
   TFile * f  = new TFile("/tmp/roberval/rhuv4/rhu_4990.root","OLD");
   
   // Filling scheme
   TTree * t_info = (TTree*) f -> Get("beaminfo");
   int bxcfg1[nbx];
   int bxcfg2[nbx];
   t_info->SetBranchAddress("bxconfig1", bxcfg1);
   t_info->SetBranchAddress("bxconfig2", bxcfg2);
   t_info -> GetEntry(0);
   
   std::vector<int> colliding; // vector containing all colliding bunch crossings
   std::vector<int> bx; // vector with first colliding bunch crossing in the train (better name needed)
   
   // Identify first bunch in each train
   // Use of first bunches suppresses albedo + non linear effects (e.g. signal pile-up, polarisation)
   for ( size_t ii = 0; ii < nbx ; ++ii)
   {
       if ( bxcfg1[ii] == 1 && bxcfg2[ii] == 1 )  // colliding bunches
       {
           colliding.push_back(ii);
           if ( ii > 0 && ii < nbx-1 )
           {
               if ( bxcfg1[ii-1] == 0 )
               {
                   bx.push_back(ii);
//                    cout << "First bunch in train: " << ii << endl;
               }
           }
       }
   }
   
   TTree * t = (TTree*) f -> Get("rhu");
   
   t->SetBranchAddress("time", &time);
   t->SetBranchAddress("dataid1", dataid1);
   t->SetBranchAddress("bxi1", bxi1);
   t->SetBranchAddress("bxi2", bxi2);
   t->SetBranchAddress("bxlumi",bxlumi);
   
   
   int n = t->GetEntries();
   std::cout << "Number of entries = " << n << std::endl;
   
   double utctime[10000];
   double rates[48][10000];
   double rates_spec[48][10000];
   
   
   // Loop over events
   bool first = false;
   for ( int i = 0; i < n; ++i )
   {
      t -> GetEntry(i);
      utctime[i] = double(time);
      
      int minTrain = 6;  // avoid period in orbit during which polarisation builds up
      for ( int j = minTrain ;j < (int)bx.size(); ++j)
      {
         double i1 = bxi1[bx[j]]/1.E11;
         double i2 = bxi2[bx[j]]/1.E11;
         double lumi = bxlumi[bx[j]];
         double specNorm = i1*i2;
         double rate[48];
         for ( int k = 0; k < 48 ; ++k )
         {
            rate[k] = dataid1[(k*3564)+bx[j]];
            rates[k][i]  += rate[k];
            rates_spec[k][i] += rate[k]/specNorm;
         }
      }
      // average rate per bunch crossing - reduce statistical fluctuations + systematics of bunch intensities
      for ( int k = 0; k < 48 ; ++k )
      {
         rates[k][i]  /=(bx.size()-minTrain);
         rates_spec[k][i]  /=(bx.size()-minTrain);
      }
   }
   
   TGraph * grates[48];
   TMultiGraph * mgrates[6];
   TCanvas * crates[6];
   TLegend * leg[6];
   for ( int i = 0; i < 6; ++i )
   {
      crates[i] = new TCanvas(Form("crates_%i",i),"");
      crates[i] -> SetRightMargin(0.3);
      mgrates[i] = new TMultiGraph(Form("mgrates_%i",i),"");
      leg[i] = new TLegend(0.72,0.4,0.95,0.9,NULL,"brNDC");
      TLegendEntry *entry;
      for ( int j = 0 ; j < 8 ; ++j )
      {
         int ich = i*8+j;
         grates[ich] = new TGraph(n,utctime,rates_spec[ich]);
         grates[ich] -> SetName(Form("grates_%i",ich));
         grates[ich] -> SetMarkerStyle(20);
         grates[ich] -> SetMarkerSize(0.8);
         grates[ich] -> SetMarkerColor(j+1);
         mgrates[i] -> Add(grates[ich]);
         entry = leg[i]->AddEntry(Form("grates_%i",ich),Form("channel %i",ich),"p");
         entry -> SetMarkerStyle(20);
         entry -> SetMarkerSize(1);
         entry -> SetMarkerColor(j+1);
      }
      mgrates[i] -> SetMinimum(0);
      mgrates[i] -> Draw("AP");
      mgrates[i] -> GetXaxis() -> SetNdivisions(504);
      leg[i] -> Draw();
   }
   
   
   
}
