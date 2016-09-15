std::map<int,float> FillReport(TString csv);


void Bcm1fEfficiency()
{
   gStyle -> SetLabelSize(0.05,"X");
   gStyle -> SetLabelSize(0.05,"Y");
   gStyle -> SetTitleOffset(1.2,"X");
   gStyle -> SetTitleOffset(1.,"Y");
   gStyle -> SetPadBottomMargin(0.15);
   gStyle -> SetPadLeftMargin(0.12);
   gStyle -> SetTitleSize(0.05,"X");
   gStyle -> SetTitleSize(0.05,"Y");
   
   std::map<int,float> lumiReport = FillReport("../summary/fillReport.csv");
   std::map<int,float>::iterator mapit;
   
   // Check if fill already analysed to save time
   TString dirname = "./dataid1/luminorm/";
   TSystemDirectory dir(dirname,dirname);
   TList *files = dir.GetListOfFiles();
   files -> Sort();
   
   std::vector<int> fill;
   double vFill[1000];
   double intLumi[1000];
   double vFillErr[1000];
   double specRateLumi[1000];
   double specRateLumiErr[1000];
   double specRate[48][1000];
   double specRateErr[48][1000];
   
   TGraphErrors * gr[48];
   TGraphErrors * grLumi;;
   
   std::vector<int> unmaskedChannels;
   
   bool firstFill = true;
   
   if ( files )
   {
      TSystemFile *file;
      TString fname;
      TIter next(files);
      while ((file=(TSystemFile*)next()))
      {
         fname = file->GetName();
         if (!file->IsDirectory() && fname.BeginsWith("analysis_perch_") && fname.EndsWith(".root")) 
         {
            int iFill = TString(fname(15,4)).Atoi();
            // only use fills available in the fill report
            mapit = lumiReport.find(iFill);
            if (mapit == lumiReport.end()) // did not find
               continue;
            fill.push_back(iFill);
            std::cout << "Analysing fill " << fill.back() << " ..." << std::endl;
            vFill[(int)fill.size()-1] = fill.back();
            intLumi[(int)fill.size()-1] = lumiReport[fill.back()];

            TFile f(dirname+fname,"old");
            // Iterate over root objects in file f
            TIter next(f.GetListOfKeys());
            TKey * key;
            while ( (key = (TKey*)next()))
            {
               if ( TString(key->GetClassName()) == "TGraphErrors" )
               {
                   TString gName = TString(key->GetName());
                   if ( gName.BeginsWith("gr_ch_") )
                   {
                      int channel = TString(gName(6,gName.Length()-6)).Atoi();
                      if ( firstFill )
                      {
                         unmaskedChannels.push_back(channel);
                      }
                      TGraphErrors * gr = (TGraphErrors*) f.Get(gName);
                      TF1 * fitf = gr -> GetFunction("pol0");
                      specRate   [channel][(int)fill.size()-1] = fitf -> GetParameter(0);
                      specRateErr[channel][(int)fill.size()-1] = fitf -> GetParameter(1);
                   }
                }
            }
            firstFill = false;

         }
      }

   }

   double ref[48];   
   double referr[48];   
   for ( size_t i = 0 ; i < unmaskedChannels.size() ; ++i )
   {
      int ch = unmaskedChannels[i];
      ref[ch] = specRate[ch][0];
      referr[ch] = specRateErr[ch][0];
   }
   
   int nFills = (int)fill.size();
   
   for ( size_t i = 0 ; i < unmaskedChannels.size() ; ++i )
   {
      int ch = unmaskedChannels[i];
      double b = ref[ch];
      double db = referr[ch];
      for ( int j = 0; j < nFills; ++j )
      {
         double a = specRate[ch][j];
         double da = specRateErr[ch][j];
         double r = a/b;
         double dr = sqrt( ((1./b)*da)*((1./b)*da) + ((a/(b*b))*db)*((a/(b*b))*db));
         specRate[ch][j] = r;
         specRateErr[ch][j] = dr;
      }

   }
   
   TCanvas * c1[48];
//   TLegend * leg = new TLegend(0.6,0.7,0.9,0.9); 
   for ( size_t i = 0 ; i < unmaskedChannels.size() ; ++i )
   {
      int ch = unmaskedChannels[i];
      c1[ch] = new TCanvas(Form("c1_%i",ch),Form("channel %i",ch));
      
      gr[ch] = new TGraphErrors(nFills,intLumi,specRate[ch],vFillErr,specRateErr[ch]);
      gr[ch] -> GetXaxis()-> SetRangeUser(0,(--lumiReport.end())->second*1.1);
      gr[ch] -> GetYaxis()-> SetRangeUser(0,1.1);
      gr[ch] -> GetXaxis()-> SetTitle("integrated luminosity (/pb)");
      gr[ch] -> GetYaxis()-> SetTitle("efficiency");
      gr[ch] -> SetMarkerStyle(20);
      gr[ch] -> SetMarkerSize(1);
      gr[ch] -> SetLineWidth(2);
      gr[ch] -> SetName(Form("gr_%i",ch));
      gr[ch] -> SetTitle(Form("channel %i",ch));
      gr[ch] -> SetMarkerColor(4);
      gr[ch] -> SetLineColor(4);
      gr[ch] -> Draw("ALP");
//       leg->AddEntry(gr[ch]->GetName(),Form("channel %i",ch),"p");
//       leg->Draw();
      c1[ch] -> SaveAs(Form("bcm1f_eff_ch_%i.png",ch));
//       leg->Clear();
   }
   
//    TMultiGraph * mg = new TMultiGraph();
//    mg -> SetMinimum(0);
//    mg -> SetMaximum(1.1);
//    
//    mg -> Add(gr[8]);
//    mg -> Add(gr[32]);
//    mg -> Add(gr[40]);
//    mg -> Add(gr[45]);
//    
//    // change channel here
//    mg -> Draw("ALP");
//    
//    mg -> GetXaxis() -> SetTitle("integrated luminosity (/pb)");
//    mg -> GetYaxis() -> SetTitle("efficiency wrt to VdM");
//    TLegend * leg = new TLegend(0.6,0.7,0.9,0.9); 
//    leg->AddEntry("gr_8","channel 8","p");
//    leg->AddEntry("gr_32","channel 32","p");
//    leg->AddEntry("gr_40","channel 40","p");
//    leg->AddEntry("gr_45","channel 45","p");
//    
//    leg -> Draw();
//    
//    
//    c1 -> SaveAs("Eff_8_32_40_45.png");
}


std::map<int,float> FillReport(TString csv)
{
   std::map<int,float> intLumi;
   int fill;
   float lumi;
   TTree *T = new TTree("ntuple","data from csv file");
   T->ReadFile(csv,"fill/I:lumi/F",',');
   // sort fill
   T->BuildIndex("fill");
   TTreeIndex *idx = (TTreeIndex*)T->GetTreeIndex();
   
   T->SetBranchAddress("lumi",&lumi);
   T->SetBranchAddress("fill",&fill);
   
   float sumLumi = 0;

   for ( int i = 0 ; i < idx->GetN() ; ++i )
   {
      float local = T->LoadTree( idx -> GetIndex()[i] );
      
      T->GetEntry(local);
      intLumi[fill] = sumLumi;
      
      sumLumi += lumi;
   }
   
   return intLumi;
}
