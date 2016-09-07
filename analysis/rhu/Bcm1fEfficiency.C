std::map<int,float> FillReport(TString csv);


void Bcm1fEfficiency()
{
   std::map<int,float> lumiReport = FillReport("fillReport.csv");
   std::map<int,float>::iterator mapit;
   
   // Check if fill already analysed to save time
   TString dirname = "./dataid1/n1n2norm/";
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
   
   if ( files )
   {
      TSystemFile *file;
      TString fname;
      TIter next(files);
      while ((file=(TSystemFile*)next()))
      {
         fname = file->GetName();
         std::cout << fname << std::endl;
         if (!file->IsDirectory() && fname.BeginsWith("analysis_perch_") && fname.EndsWith(".root")) 
         {
            int iFill = TString(fname(15,4)).Atoi();
            std::cout << iFill << std::endl;
            // only use fills available in the fill report
            mapit = lumiReport.find(iFill);
            if (mapit == lumiReport.end()) // did not find
               continue;
            fill.push_back(iFill);
//            std::cout << "Analysing fill " << fill.back() << " ..." << std::endl;
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
                      unmaskedChannels.push_back(channel);
                      TGraphErrors * gr = (TGraphErrors*) f.Get(gName);
                      TF1 * fitf = gr -> GetFunction("pol0");
                      specRate   [channel][(int)fill.size()-1] = fitf -> GetParameter(0);
                      specRateErr[channel][(int)fill.size()-1] = fitf -> GetParameter(1);
                   }
                }
            }
         }
      }

   }
   
   int nFills = (int)fill.size();
   for ( int j = 0; j < nFills; ++j )
   {
      std::cout << "oioi " << fill[j] << std::endl;
   }
   
   for ( size_t i = 0 ; i < unmaskedChannels.size() ; ++i )
   {
      int ch = unmaskedChannels[i];
      gr[ch] = new TGraphErrors(nFills,intLumi,specRate[ch],vFillErr,specRateErr[ch]);
   }
   
   gr[45] -> Draw("ALP");
   
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
//      std::cout <<  i << "  " << local << "  " << fill << "  " << lumi << "  " << sumLumi << std::endl;
      
      intLumi[fill] = sumLumi;
      
      sumLumi += lumi;
   }
   
   return intLumi;
}
