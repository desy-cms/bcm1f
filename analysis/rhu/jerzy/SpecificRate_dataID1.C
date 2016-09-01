



void SpecificFill( int fill, string filename,double results[49][2])
{
    
    static const int nbx = 3564;
    static const int ntrains = 100;  // a large number, larger than usual number of trains
    
    TFile * f = new TFile(filename.c_str(), "read");;
    
    // Filling scheme
    TTree * t_info = (TTree*) f -> Get("beaminfo");
    int bxcfg1[nbx];
    int bxcfg2[nbx];
    t_info->SetBranchAddress("bxconfig1", bxcfg1);
    t_info->SetBranchAddress("bxconfig2", bxcfg2);
    t_info -> GetEntry(0);
    
    std::vector<int> colliding; // array containing all colliding bunch crossings
    std::vector<int> bx; // array with first colliding bunch crossing in the train (better name needed)
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
                    cout << "First bunch in train: " << ii << endl;
                }
            }
        }
    }
    
    cout << "Number of trains = " << bx.size() << endl;
    
    // RHU data
    TTree * t = (TTree*) f -> Get("rhu");
    
    int time;
    float data[171072];
    float bxi1[3564];
    float bxi2[3564];
    float bxlumi[3564];
    
    t->SetBranchAddress("time", &time);
    t->SetBranchAddress("dataid1", data);
    t->SetBranchAddress("bxi1", bxi1);
    t->SetBranchAddress("bxi2", bxi2);
    t->SetBranchAddress("bxlumi",bxlumi);
    
    TH1F * orbit = new TH1F("orbit","",3564,0,3564);
    
    // Create histograms for rates and intensities
    TH1F * h_spec_rate_lumi[ntrains];
    TH1F * h_spec_rate_raw[ntrains];
    TH1F * h_spec_rate_ch[48][ntrains];
    TH1F * h_bxi1[ntrains];
    TH1F * h_bxi2[ntrains];
    TH1F * h_bxlumi[ntrains];
    for ( size_t ii = 0; ii < bx.size(); ++ii )
    {
        h_spec_rate_lumi[ii] = new TH1F(Form("h_spec_rate_lumi_%i",bx[ii]),"",1000,200,1500);
        h_bxi1[ii] = new TH1F(Form("h_bxi1_%i",bx[ii]),"",2000,0,20);
        h_bxi2[ii] = new TH1F(Form("h_bxi2_%i",bx[ii]),"",2000,0,20);
        h_bxlumi[ii]= new TH1F(Form("h_bxlumi_%i",bx[ii]),"",2000,2,7);
        for ( int j =0 ; j < 48 ; ++j )
        {
            h_spec_rate_ch[j][ii] = new TH1F(Form("h_spec_rate_ch_%i_%i",j,bx[ii]),"",1000,0,800);
        }
    }
    
    // Loop over events
    for ( int i = 0; i < t->GetEntries(); ++i )
    {
        t -> GetEntry(i);
        
        for ( size_t j = 0 ;j < bx.size(); ++j)
        {
            float i1 = bxi1[bx[j]]/1.E10;
            float i2 = bxi2[bx[j]]/1.E10;
            float lumi = bxlumi[bx[j]];
            h_bxi1[j] -> Fill(i1);
            h_bxi2[j] -> Fill(i2);
            h_bxlumi[j]->Fill(lumi);
            float rate[48];
            for ( int k = 0; k < 48 ; ++k )
            {
                rate[k] = data[(k*3564)+bx[j]];
                //h_spec_rate_ch[k][j]->Fill(rate[k]/(i1*i2));
                h_spec_rate_ch[k][j]->Fill(rate[k]/lumi);
            }
            float totalRate = rate[40]+rate[41]+rate[44]+rate[45];
            //h_spec_rate_lumi[j]->Fill(totalRate/(i1*i2));
            h_spec_rate_lumi[j]->Fill(totalRate/(lumi));
        }
    }
    
    TFile * fout = new TFile(Form("analysis_perch_%i.root",fill),"recreate");
    
    double gbx[ntrains];
    double gbxe[ntrains];
    double mean[ntrains];
    double rms[ntrains];
    
    
    
    
    
    for ( size_t ii = 0; ii < bx.size(); ++ii )
    {
        TFitResultPtr r = h_spec_rate_lumi[ii] -> Fit("gaus","S");
        h_spec_rate_lumi[ii] -> Write();
        h_bxi1[ii] -> Write();
        h_bxi2[ii] -> Write();
        h_bxlumi[ii]->Write();
        gbx[ii] = double(bx[ii]);
        gbxe[ii] = 0.;
        mean[ii] = r -> Value(1);
        rms[ii]  = r -> Value(2);
        
    }
    
    TGraphErrors * gr_lumi = new TGraphErrors(int(bx.size()),gbx,mean,gbxe,rms);
    
    gr_lumi -> SetMarkerStyle(20);
    gr_lumi -> GetXaxis() -> SetTitle("bx id");
    gr_lumi -> GetYaxis() -> SetTitle("<rate/luminosity>");
    gr_lumi -> GetYaxis() -> SetRangeUser(200,1400);
    gr_lumi -> SetTitle("Four channels");
    //gr_lumi -> Draw("AP");
    TFitResultPtr r = gr_lumi -> Fit("pol0","S");
    TF1 * fitf = gr_lumi -> GetFunction("pol0");
    double value = r -> Value(0);
    double error = r -> Error(0);
    results[0][0]=value;
    results[0][1]=error;
    
    
    TLegend * leg = new TLegend(0.5,0.7,0.9,0.9);
    leg->SetHeader(Form("Fill %i",fill));
    leg->AddEntry(gr_lumi,"data","p");
    leg->AddEntry(fitf,Form("const fit: %2.2f +- %1.2f", value, error),"l");
    //leg -> Draw();
    
    gr_lumi -> Write();
    
    // PER CHANNEL
    TGraphErrors * gr_ch[48];
    for ( int ch = 0 ; ch < 48 ; ++ch )
    {
        if(ch==40 || ch == 41 || ch == 44 || ch == 45 || ch==3 || ch ==8 || ch==12 || ch ==15 || ch==32){
            for ( size_t ii = 0; ii < bx.size(); ++ii )
            {
                TFitResultPtr r = h_spec_rate_ch[ch][ii] -> Fit("gaus","S");
                h_spec_rate_ch[ch][ii] -> Write();
                gbx[ii] = double(bx[ii]);
                gbxe[ii] = 0.;
                mean[ii] = r -> Value(1);
                rms[ii]  = r -> Value(2);
                
            }
            gr_ch[ch] = new TGraphErrors(int(bx.size()),gbx,mean,gbxe,rms);
            gr_ch[ch] -> SetName(Form("gr_ch_%i",ch));
            gr_ch[ch] -> SetMarkerStyle(20);
            gr_ch[ch] -> GetXaxis() -> SetTitle("bx id");
            gr_ch[ch] -> GetYaxis() -> SetTitle("<rate/luminosity>");
            gr_ch[ch] -> GetYaxis() -> SetRangeUser(0,800);
            gr_ch[ch] -> SetTitle(Form("Channel %i",ch));
            
            TFitResultPtr r = gr_ch[ch] -> Fit("pol0","S");
            TF1 * fitf = gr_ch[ch] -> GetFunction("pol0");
            results[ch+1][0]=r->Value(0);
            results[ch+1][1]=r->Error(0);
            gr_ch[ch] -> Write();
        }
    }
    
}


void SpecificRate(){
    const char * dirname = "/nfs/dust/cms/user/walsh/bril/bcm1f/rhu/v3/";
    //TGraphErrors * FinalGr = new TGraphErrors();
    string filename,fillnumber;
    TSystemDirectory dir(dirname, dirname);
    TList *files = dir.GetListOfFiles();
    TFile *LUMI= new TFile("lumi.root");
    TTree *tr = new TTree();
    tr= (TTree*)LUMI->Get("lumi");
    int fill;
    int size=0;
    double luminosity;
    double results[49][2];
    double x [49][100]; // Larger size than number of fills
    double y [49][100];
    double yerr [49][100];
    double xerr [49][100];
    tr->SetBranchAddress("lumi",&luminosity);
    tr->SetBranchAddress("fill",&fill);
    double NORM [49];
    
    
    tr->GetEntry(0);
    
    
    if (files) {
        TSystemFile *file;
        TString fname;
        TIter next(files);
        while ((file=(TSystemFile*)next())) {
            fname = file->GetName();
            if (!file->IsDirectory() && fname.BeginsWith(Form("rhu_%i_",fill)) && fname.EndsWith(".root")) {
                filename  = string(dirname)+string(fname.Data());
                SpecificFill(fill,filename,results);
                for(int K=0;K<49;++K){
                    if(K==0 || K==41 || K==42 || K==45 || K==46 || K==4 || K==9 || K==13 || K==16 || K==33){
                        NORM[K]=results[K][0];
                        x[K][0]=luminosity;
                        y[K][0]=results[K][0]/NORM[K];
                        yerr[K][0]=results[K][1]/NORM[K];
                        xerr[K][0]=0.;
                    }
                }
                
                
            }
        }
    }
    
    for(size_t ki=1;ki<tr->GetEntries();++ki){
        
        tr->GetEntry(ki);
        
        
        if (files) {
            TSystemFile *file;
            TString fname;
            TIter next(files);
            while ((file=(TSystemFile*)next())) {
                fname = file->GetName();
                if (!file->IsDirectory() && fname.BeginsWith(Form("rhu_%i_",fill)) && fname.EndsWith(".root") && fill!=5005 && fill!=5020) {
                    filename  = string(dirname)+string(fname.Data());
                    SpecificFill(fill,filename,results);
                    size++;
                    for(int K=0;K<49;++K){
                        if(K==0 || K==41 || K==42 || K==45 || K==46 || K==4 || K==9 || K==13 || K==16 || K==33){
                            x[K][size]=luminosity;
                            y[K][size]=results[K][0]/NORM[K];
                            yerr[K][size]=results[K][1]/NORM[K];
                            xerr[K][size]=0.;
                        }
                    }
                    /*
                     x[size]=luminosity;
                     y[size]=results[0][0]/NORM;
                     yerr[size]=results[0][1]/NORM;
                     xerr[size]=0.;
                     */
                    
                    
                }
            }
        }
    }
    
    TGraphErrors * FinalGr = new TGraphErrors(size+1,x[0],y[0],xerr[0],yerr[0]);
    TFile* RES= new TFile("results.root","RECREATE");
    FinalGr -> SetMarkerStyle(20);
    FinalGr -> GetXaxis() -> SetTitle("Integrated Luminosity");
    FinalGr -> GetYaxis() -> SetTitle("Relative Efficency");
    FinalGr -> GetYaxis() -> SetRangeUser(0,1.2);
    FinalGr -> SetTitle("Efficency downgrade");
    //TFitResultPtr r = FinalGr -> Fit("pol1");
    //TF1 * fitf = FinalGr -> GetFunction("pol1");
    FinalGr -> Draw("AP");
    FinalGr -> Write();
    
    TGraphErrors * ChannelGraphs [49];
    
    for ( int ch = 1 ; ch < 49 ; ++ch )
    {
        if(ch==41 || ch == 42 || ch == 45 || ch == 46 || ch==4 || ch ==9 || ch==13 || ch ==16 || ch==33){
            ChannelGraphs[ch] = new TGraphErrors(size+1,x[ch],y[ch],xerr[ch],yerr[ch]);
            ChannelGraphs[ch] -> SetName(Form("Ch_Number_%i",ch-1));
            ChannelGraphs[ch] -> SetMarkerStyle(20);
            ChannelGraphs[ch] -> GetXaxis() -> SetTitle("Integrated Luminosity");
            ChannelGraphs[ch] -> GetYaxis() -> SetTitle("Relative Efficency");
            ChannelGraphs[ch] -> GetYaxis() -> SetRangeUser(0,1.2);
            ChannelGraphs[ch] -> SetTitle(Form("Channel_%i_Efficency_Downgrade",ch-1));
            //TFitResultPtr r = ChannelGraphs[ch] -> Fit("pol1");
            //TF1 * fitf = ChannelGraphs[ch] -> GetFunction("pol1");
            //ChannelGraphs[ch] -> Draw("AP");
            ChannelGraphs[ch] -> Write();
        }
    }
}


