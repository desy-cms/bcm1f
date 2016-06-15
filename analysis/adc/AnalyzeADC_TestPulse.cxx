/*
Original author: Olena Karacheban, DESY-Zeuthen, CMS (BRIL): olena.karacheban@desy.de
 
 Script to analyse raw ADC data files and produce output files with following histograms: baseline, baseline sigma, amplitude spectrum and time over threshold.
 Arrival time analysis is not finished yet.
 
 Script will be used to prodese plots for my PhD theses (defence is planed for the end of 2016 .)
 
 Currently raw files for analysis have to be in the folder:
 /localdata/OutputADC/FilesToAnalyzeNow/
 and analyzed files will be written to:
 /localdata/OutputADC/Analysis/
 Change these pathes to your prefered folders. Also variables as threshold and minimum signal length you can set to preferable:
 int Threshold = 3; // in ADC counts
 int minsigwidth = 4;  // in samples
 
 To run the code, produce executable file. If you will copy it to brilvme4 machne, where raw ADC data is stored, you can run it as ./executable_file_name.
 
 g++ -o findTestPulse `root-config --cflags --glibs` AnalyzeADC_TestPulse.cxx -lboost_system -lboost_filesystem -lboost_regex

 Note: just copy executable file to brilvme4, no sourse codes should be there, as spase is limited.
       
       *** Modified my Roberval Walsh
       
 */

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TMath.h>
#include <TMarker.h>
#include <TGraph.h>
#include <TFitResult.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TArrow.h>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <time.h>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <TROOT.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>


using namespace std;
using namespace boost::filesystem;


//int Analyse_multiADC() {

int NEntries = 0;
int N=0;
unsigned char data[0x200000];
unsigned int header[4];
unsigned int Eheader[4];
unsigned int board = 0;
unsigned int channel=0;
unsigned int Event=0;

int plotdata[0x200000];
int timeax[0x200000];
int N_BL = 20;

float Summ_BL = 0;
float Aver_BL = 0;
float Summ_BL_sigma = 0;
float BL_sigma = 0;

float sigmax = 300.;
float sigsize =0;
float Sig_Amp = 0;
int sigmaxtime = 0;
int arrivaltime = 0;

int Threshold = 3;
int minsigwidth = 4;
int width = 0;
int countlong = 0;
int CTD_width = 0;
int CTD_range_max = 0;
int CTD_range_min = 0;
float CTD_threshold = 0;
float CTD_arrivaltime =0;
int CTD_time_max = 0;
int CTD_time_min = 0;
int NPeacksFound = 0;
int N_0f_processed_files = 0;
int skipEvent = 0;

int main(int argc, char* argv[])
{
    std::string output_folder; std::string input_folder;
    
    const boost::regex my_filter( "^data_201.*completed.root$" );
    
    std::vector< std::string > all_matching_files;
    
    if ( argc < 2 )
    {
        printf("\nUsage: /findPeacks_TestPulse_withCTD_Th3_forWeb <work folder>\n\n"); return 0;
    }
    
    output_folder = argv[1]; input_folder = argv[1];
    
    output_folder += "histograms/";
    
    boost::filesystem::create_directories(output_folder);
    
    boost::filesystem::directory_iterator end_itr;
    
    for( boost::filesystem::directory_iterator j( input_folder ); j != end_itr; ++j )
    {
        if( !boost::filesystem::is_regular_file( j->status() ) ) continue;
        
        boost::smatch what;
        
        if( !boost::regex_match( j->leaf(), what, my_filter ) ) continue;
        
        string dataFile= input_folder + j->leaf();
        
        std::cout << "Found a completed ROOT file for processing: " << dataFile << std::endl;
        
        gStyle->SetOptStat(1110);
        //gStyle->SetOptStat(0000);
        
        //reading raw ROOT file from ADC
        TChain * ADC_data =new TChain("T");
        
        
        ADC_data -> Add(dataFile.c_str());
        cout <<"Start to analyse file: " << dataFile<<endl;
        NEntries = ADC_data -> GetEntries();
        cout << "NEntries = "<< NEntries<< endl;
        
        if(NEntries<1) {
            cerr << "No Entries found" << endl;
            return -1;
        }
        
        //create new ROOT file, tree and branches
        string outputfilename;
        string outputfilename_txt;
        
        string filename1 =  j->leaf();
        filename1.erase(filename1.size()-15);
        
        cout<< "file nane here is "<< filename1<< endl;
        
        outputfilename = output_folder + "Analysed_TP_"+ j->leaf();
        
        outputfilename_txt = output_folder + "TestPulseSpectrum.txt";
        
        cout <<"outputfilename =" << outputfilename<<endl;
        cout <<"outputfilename_txt =" << outputfilename_txt<<endl;
        
        TFile* f_out = TFile::Open(outputfilename.c_str(),"RECREATE");
        
        //branches of raw ROOT file from ADC
        ADC_data -> SetBranchAddress("time_sec",&header[0]);
        ADC_data -> SetBranchAddress("time_usec",&header[1]);
        ADC_data -> SetBranchAddress("ACQ_Status",&header[2]);
        ADC_data -> SetBranchAddress("VME_Status",&header[3]);
        ADC_data -> SetBranchAddress("board",&board);
        ADC_data -> SetBranchAddress("channel",&channel);
        ADC_data -> SetBranchAddress("Event",&Event);
        ADC_data -> SetBranchAddress("EventHeader1",&Eheader[0]);
        ADC_data -> SetBranchAddress("EventHeader2",&Eheader[1]);
        ADC_data -> SetBranchAddress("EventHeader3",&Eheader[2]);
        ADC_data -> SetBranchAddress("EventHeader4",&Eheader[3]);
        ADC_data -> SetBranchAddress("N",&N);
        ADC_data -> SetBranchAddress("data",data);
        
        // new histos
        TH1F * h_BL[48];
        TH1F * h_BL_sigma[48];
        TH1F * h_Sig_Amp[48];
        TH1F * h_Sig_length[48];
        TH1F * h_Arrival_time[48];
        
        for (int b = 0; b < 6 ; ++b) {
            for (int ch = 0; ch < 8 ; ++ch) {
                h_BL[ch+8*b]=new TH1F(Form("BL_b%i_ch%i",b,ch),"Base Line",256,0,256);
                h_BL[ch+8*b]->GetXaxis()->SetTitle("Baseline (ADC counts)");
                h_BL[ch+8*b]->GetYaxis()->SetTitle("Number of events");
                h_BL[ch+8*b]->GetYaxis()->SetTitleOffset(1.4);
                h_BL[ch+8*b]->SetLineWidth(3);
                
                
                h_BL_sigma[ch+8*b]=new TH1F(Form("BL_sigma_b%i_ch%i",b,ch),"Base Line Sigma",100,0,10);
                h_BL_sigma[ch+8*b]->GetXaxis()->SetTitle("Baseline sigma");
                h_BL_sigma[ch+8*b]->GetYaxis()->SetTitle("Number of events");
                h_BL_sigma[ch+8*b]->GetYaxis()->SetTitleOffset(1.4);
                h_BL_sigma[ch+8*b]->SetLineWidth(3);
                
                h_Sig_Amp[ch+8*b]=new TH1F(Form("Sig_Amp_b%i_ch%i",b,ch),"Test Pulse Amplitude",256,0,1024);
                h_Sig_Amp[ch+8*b]->GetXaxis()->SetTitle("Test pulse amplitude (mV)");
                h_Sig_Amp[ch+8*b]->GetYaxis()->SetTitle("Number of events");
                h_Sig_Amp[ch+8*b]->GetYaxis()->SetTitleOffset(1.45);
                h_Sig_Amp[ch+8*b]->SetLineWidth(3);
                h_Sig_Amp[ch+8*b]->GetXaxis()->SetRangeUser(0.,350.);
                
                h_Sig_length[ch+8*b]=new TH1F(Form("Sig_length_b%i_ch%i",b,ch),"Sig_length",100,0,200);
                h_Sig_length[ch+8*b]->GetXaxis()->SetTitle("Signal length (ns)");
                h_Sig_length[ch+8*b]->GetYaxis()->SetTitle("Number of events");
                
                h_Arrival_time[ch+8*b]=new TH1F(Form("Arrival_time_b%i_ch%i",b,ch),"h_Arrival_time",45000,0,90000);
                h_Arrival_time[ch+8*b]->GetXaxis()->SetTitle("Time (ns)");
                h_Arrival_time[ch+8*b]->GetYaxis()->SetTitle("Number of events");
                
            }
        }
        
        TH1F * h_Arrival_time_48 = new TH1F ("h_Arrival_time_48", "Arrival_time BCM1F: 48 channels", 45000,0,90000);
        h_Arrival_time_48->GetXaxis()->SetTitle("Time (ns)");
        h_Arrival_time_48->GetYaxis()->SetTitle("Number of events");
        TH1F * h_Amp_Spectrum_48 = new TH1F ("h_Amp_Spectrum_48", "Amplitude spectrum: 48 channels", 256,0,1024);
        h_Amp_Spectrum_48->GetXaxis()->SetTitle("Signal amplitude (mV)");
        h_Amp_Spectrum_48->GetYaxis()->SetTitle("Number of events");
        
        TCanvas *canvas = new TCanvas("canvas","Draw histo",800,600);
        
        for (unsigned int i=0; i<NEntries; i++) {
            // for (unsigned int i=0; i<1000; i++) {
            ADC_data -> GetEntry(i);
            
            skipEvent = 0;
            
            if(N>0){
                
                for (int ss = 0; ss < N_BL ; ++ss) {
                    Summ_BL+=(int)data[ss];
                }
                Aver_BL=Summ_BL/N_BL;
                
                for (int hh = 0; hh < N_BL ; ++hh) {
                    Summ_BL_sigma+=(Aver_BL-((int)data[hh]))*(Aver_BL-((int)data[hh]));
                }
                BL_sigma=sqrt(Summ_BL_sigma/N_BL);
                
                h_BL[channel+8*board]->Fill(Aver_BL);
                h_BL_sigma[channel+8*board]->Fill(BL_sigma);
                
                //for (int nn = 0; nn < N ; ++nn) {
                for (int nn = 3060; nn < 3090 ; ++nn) {  // ONLY TP
                    //if ( ((nn >3050) && (nn <3200)) ) continue; // exlude TP
                    
                    if ( (int)data[nn] < Aver_BL-Threshold ) {
                        width = 1;
                        // cout << "Candidate: ..."<< endl;
                        while ( ((int)data[nn+width] < Aver_BL-Threshold) && (width<minsigwidth) && ((nn+width)<N)){
                            width++;
                        }
                        
                        if ( width >= minsigwidth ) {
                            width = 0;
                            sigsize = 0.;
                            sigmax = 256;
                            Sig_Amp = 0;
                            CTD_width =0;
                            CTD_threshold =0;
                            
                            NPeacksFound++;
                            
                            while ( ((int)data[nn+width] < Aver_BL-Threshold) && ((nn+width)<N)){
                                
                                sigsize += Aver_BL - (int)data[nn+width];
                                
                                width++;
                                
                                if (width >60)  {
                                    
                                    //cout <<"Too long signal, will set nn to end of orbit!!! " << endl;
                                    nn = N;
                                    countlong++;
                                    skipEvent = 1;
                                    //cout << "i "<<i <<  " its own loop , skipEv ="<< skipEvent<< endl;
                                }
                                
                                if ( (int)data[nn+width] < sigmax ){
                                    sigmax = (int)data[nn+width];
                                    sigmaxtime=nn+width;
                                    arrivaltime = nn;
                                    
                                    
                                } // end of if ( (int)data[nn+width] < sigmax ){
                                
                            } // end of while ...data[nn+width]
                            
                            Sig_Amp=Aver_BL-sigmax;
                            CTD_threshold = Aver_BL - Sig_Amp *0.5;
                            
                            while ( ((int)data[nn+CTD_width] < Aver_BL-Threshold)&& ((int)data[nn+CTD_width] > sigmax) && ((nn+CTD_width)<N)){
                                
                                
                                if ( ((int)data[nn+CTD_width] < CTD_threshold) && ((int)data[nn+CTD_width-1] > CTD_threshold)){
                                    CTD_range_max = (int)data[nn+CTD_width];
                                    CTD_range_min = (int)data[nn+CTD_width-1];
                                    CTD_time_max = nn+CTD_width;
                                    CTD_time_min = nn+CTD_width-1;
                                    
                                    CTD_arrivaltime = (CTD_time_min +((CTD_threshold-CTD_range_min)/(CTD_range_max-CTD_range_min))* (CTD_time_max-CTD_time_min));
                                    
                                } // end of if
                                CTD_width++;
                            } // end of while ...data[nn+CTD_width]
                            
                            if (skipEvent == 1) continue; 
                            
                            h_Sig_Amp[channel+8*board]->Fill(Sig_Amp*4);    
                            h_Sig_length[channel+8*board]->Fill(width*2); 
                            
                            
                            if (CTD_arrivaltime !=0)              
                                h_Arrival_time[channel+8*board]->Fill(CTD_arrivaltime*2); 
                        }
                        nn += width;
                        
                    } // if ( (int)data[nn] < Aver_BL-Threshold ) {
                    
                } //end of nn loop
            } //end of if(N>0)
            
            
            Aver_BL = 0;
            Summ_BL = 0;
            Summ_BL_sigma = 0;
            BL_sigma = 0;
            Sig_Amp = 0;
            
        } //end of i loop
        
        for (int b = 0; b < 48 ; ++b) { 
            h_Arrival_time_48->Add(h_Arrival_time[b], 1);
            h_Amp_Spectrum_48->Add(h_Sig_Amp[b], 1); 
        }
        
        
        f_out->WriteTObject(h_Arrival_time_48);
        f_out->WriteTObject(h_Amp_Spectrum_48);
        f_out->Write();
        // f_out->Close();
        
        NPeacksFound = 0;
        
        N_0f_processed_files++;
        cout << "Reached end of analysis for current file successfully... " << endl; 
        cout << " N_0f_processed_files =" << N_0f_processed_files<< endl; 
        
        h_Amp_Spectrum_48->Delete();
        h_Arrival_time_48->Delete();
        
        //TCanvas canvas; 
        //TCanvas *canvas = new TCanvas("canvas","Draw histo",800,600);
        std::fstream text_out;
        text_out.open(outputfilename_txt.c_str(),std::ios::out | std::ios::app);
        char tmpstr[1000];
        
        for (int b = 0; b < 6 ; ++b) {
            for (int ch = 0; ch < 8 ; ++ch) {
                //if ( (ch+8*b == 3) ||  (ch+8*b == 8) ||  (ch+8*b == 12) ||  (ch+8*b == 15) ||  (ch+8*b == 32) ||  (ch+8*b == 40) ||  (ch+8*b == 41) ||  (ch+8*b == 44) || (ch+8*b == 45) ) {
                if   (ch+8*b < 50) {
                    sprintf(tmpstr, "%i    %10.4f    %10.4f    \n", ch+8*b , h_Sig_Amp[ch+8*b]-> GetMean(), h_Sig_Amp[ch+8*b]-> GetMeanError());
                    text_out << tmpstr;
                    //h_Sig_Amp[ch+8*b]->Draw();
                    
                    std::string formFormat = output_folder; formFormat += "TestPulseSpectrum_%s_ch%i.png"; 
                    
                    //canvas->SaveAs(Form(formFormat.c_str(), filename1.c_str(), ch+8*b));
                }
                
                h_BL[ch+8*b]->Delete(); 
                h_BL_sigma[ch+8*b]->Delete(); 
                h_Sig_Amp[ch+8*b]->Delete();
                h_Sig_length[ch+8*b]->Delete();
                h_Arrival_time[ch+8*b]->Delete();
            }
        }
        text_out.close();
        f_out->Close();
        //break; // Only one completed file has to be processes at once
    } // end of j loop  -- files loop 
    
    return 0;
}


