#include <DataMCPlot.hpp>

#include <TFile.h>
#include <TKey.h>
#include <TObjString.h>
#include <THStack.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TGaxis.h>

#include <boost/algorithm/string/predicate.hpp>

#include <stdexcept>
#include <sstream>


using namespace std;


DataMCPlot::DataMCPlot(string const &srcFileName, string const &dirName /*= ""*/):
    plotResiduals(true)
{
    ReadFile(srcFileName, dirName);
}


shared_ptr<TCanvas> DataMCPlot::Draw()
{
    // Global decoration settings
    gStyle->SetErrorX(0.);
    gStyle->SetHistMinimumZero(true);
    gStyle->SetOptStat(0);
    gStyle->SetStripDecimals(false);
    TGaxis::SetMaxDigits(3);
    
    
    // Setup layout of pads within the canvas
    // Allow space for the residuals plot if requested
    double const bottomSpacing = (plotResiduals) ? 0.2 : 0.;
    
    // Margin for axis labels
    double const margin = 0.1;
    
    // Width of the main pad
    double const mainPadWidth = 0.85;
    
    
    // Create a canvas and pads to draw in
    TCanvas canvas("canvas", "", 1500, 1000 / (1. - bottomSpacing));
    
    TPad mainPad("mainPad", "", 0., bottomSpacing, mainPadWidth + margin, 1.);
    mainPad.SetTicks();
    
    // Adjust margins to host axis labels (otherwise they would be cropped)
    mainPad.SetLeftMargin(margin / mainPad.GetWNDC());
    mainPad.SetRightMargin(margin / mainPad.GetWNDC());
    mainPad.SetBottomMargin(margin / mainPad.GetHNDC());
    mainPad.SetTopMargin(margin / mainPad.GetHNDC());
    
    mainPad.Draw();
    
    
    // Put MC histogramss into a stack
    THStack mcStack("mcStack", title.c_str());
    
    for (auto h = mcHists.crbegin(); h != mcHists.crend(); ++h)
        mcStack.Add(h->get(), "hist");
    
    
    // Create a legend
    TLegend legend(0.86, 0.9 - 0.04 * (mcHists.size() + ((dataHist) ? 1 : 0)), 0.99, 0.9);
    legend.SetFillColor(kWhite);
    legend.SetTextFont(42);
    legend.SetTextSize(0.03);
    
    if (dataHist)
        legend.AddEntry(dataHist.get(), dataHist->GetTitle(), "p");
    
    for (auto const &h: mcHists)
        legend.AddEntry(h.get(), h->GetTitle(), "f");
    
    
    // Draw the MC stack and the data histogram
    mainPad.cd();
    mcStack.Draw();
    
    if (dataHist)
        dataHist->Draw("p0 e1 same");
    
    
    // Draw the legend
    canvas.cd();
    legend.Draw();
    
    
    // Update the maximum
    if (dataHist)
    {
        double const histMax = 1.1 * max(mcStack.GetMaximum(), dataHist->GetMaximum());
        mcStack.SetMaximum(histMax);
        dataHist->SetMaximum(histMax);
    }
    
    
    // Plot residuals histogram if needed
    unique_ptr<TPad> residualsPad;
    unique_ptr<TH1> residualsHist;
    
    if (plotResiduals)
    {
        // Create a histogram with total MC expectation. Use a pointer rather than an object in
        //order to infer in terms of the base class
        auto histIt = mcHists.cbegin();
        unique_ptr<TH1> mcTotalHist(dynamic_cast<TH1 *>((*histIt)->Clone("mcTotalHist")));
        
        for (++histIt; histIt != mcHists.cend(); ++histIt)
            mcTotalHist->Add(histIt->get());
        
        
        // Create a histogram with residuals. Again avoid referring to a concrete histogram class
        residualsHist.reset((dynamic_cast<TH1 *>(dataHist->Clone("residualsHist"))));
        residualsHist->Add(mcTotalHist.get(), -1);
        residualsHist->Divide(mcTotalHist.get());
                
        
        // Create a pad to draw residuals
        residualsPad.reset(new TPad("residualsPad", "", 0., 0., mainPadWidth + margin,
         bottomSpacing + margin));
        
        
        // Adjust the pad's margins so that axis labels are not cropped
        residualsPad->SetLeftMargin(margin / residualsPad->GetWNDC());
        residualsPad->SetRightMargin(margin / residualsPad->GetWNDC());
        residualsPad->SetBottomMargin(margin / residualsPad->GetHNDC());
        residualsPad->SetTopMargin(0.);
        
        
        // Decoration of the pad
        residualsPad->SetTicks();
        residualsPad->SetGrid(0, 1);
        
        residualsPad->SetFillStyle(0);
        //^ Needed not to oscure the lower half of the zero label in the mainPad
        
        
        // Draw the pad
        canvas.cd();
        residualsPad->Draw();
        
        
        // Extract the label of the x axis of the MC stacked plot
        auto const pos1 = title.find_first_of(';');
        auto const pos2 = title.find_first_of(';', pos1 + 1);
        string const xAxisTitle(title.substr(pos1 + 1, pos2 - pos1 - 1));
        
        
        // Reset the title of the residuals histogram. It gets axis titles only
        residualsHist->SetTitle((string(";") + xAxisTitle + ";#frac{Data-MC}{MC}").c_str());
        
        
        // Decoration of the residuals histogram
        residualsHist->SetMinimum(-0.25);
        residualsHist->SetMaximum(0.25);
        
        residualsHist->SetMarkerStyle(20);
        residualsHist->SetLineColor(kBlack);
        
        TAxis *xAxis = residualsHist->GetXaxis();
        TAxis *yAxis = residualsHist->GetYaxis();
        
        // Make axis label and titles of same size as in the main pad (the actual text size for the
        //default font is linked up with the current pad's smallest dimension)
        xAxis->SetTitleSize(mcStack.GetXaxis()->GetTitleSize() *
         mainPad.GetHNDC() / residualsPad->GetHNDC());
        xAxis->SetLabelSize(mcStack.GetXaxis()->GetLabelSize() *
         mainPad.GetHNDC() / residualsPad->GetHNDC());
        yAxis->SetTitleSize(mcStack.GetXaxis()->GetTitleSize() *
         mainPad.GetHNDC() / residualsPad->GetHNDC());
        yAxis->SetLabelSize(mcStack.GetXaxis()->GetLabelSize() *
         mainPad.GetHNDC() / residualsPad->GetHNDC());
        
        yAxis->SetNdivisions(405);
        yAxis->CenterTitle();
        yAxis->SetTitleOffset(0.5);
        xAxis->SetTickLength(xAxis->GetTickLength() * (1. - 2. * margin - bottomSpacing) /
         bottomSpacing);
        
        
        // // Alter the y axis so that its labels do not overlap with the ones of the stacked plot.
        // //In the code "5" is the number of primary divisions
        // double const tickOffset = (residualsHist->GetMaximum() - residualsHist->GetMinimum()) /
        //  5 * 0.6;
        // residualsHist->SetMaximum(residualsHist->GetMaximum() + tickOffset);
        
        
        // Draw the residuals histogram
        residualsPad->cd();
        residualsHist->Draw("p0 e1");
        
        
        // Remove the labels from x axis of the main histogram
        mcStack.GetXaxis()->SetLabelOffset(999.);
    }
    
    
    /**///canvas.Print("figure.png");
    
    // Return a copy of the canvas since all objects drawn in the original canvas will be deleted
    //at the exit of this method or when this object is destroyed
    return shared_ptr<TCanvas>(dynamic_cast<TCanvas *>(canvas.Clone()));
}


void DataMCPlot::ReadFile(std::string const &srcFileName, std::string const &dirName)
{
    // Try to open the source file
    unique_ptr<TFile> srcFile(TFile::Open(srcFileName.c_str()));
    
    if (not srcFile or srcFile->IsZombie())
    {
        ostringstream ost;
        ost << "Source file \"" << srcFileName << "\" is corrupted or is not a valid ROOT file.";
        throw runtime_error(ost.str());
    }
    
    
    // Open the desired directory in the source file
    unique_ptr<TDirectory> curDirectory(srcFile->GetDirectory(dirName.c_str()));
    
    if (not curDirectory)
    {
        ostringstream ost;
        ost << "Source file \"" << srcFileName << "\" does not contain a directory \"" <<
         dirName << "\".";
        throw runtime_error(ost.str());
    }
    
    
    // Read histogram title
    unique_ptr<TObjString> titleStored(dynamic_cast<TObjString *>(curDirectory->Get("title")));
    
    if (titleStored)
        title = titleStored->GetString().Data();
    
    
    // Read data histogram
    dataHist.reset(dynamic_cast<TH1 *>(curDirectory->Get("data")));
    
    if (not dataHist)
    {
        ostringstream ost;
        ost << "Failed to find data histogram in file \"" << srcFileName << "\", directory \"" <<
         dirName << "\".";
        throw runtime_error(ost.str());
    }
    
    
    // Read histograms with simulation
    TIter keyIter(curDirectory->GetListOfKeys());
    
    while (true)
    {
        TKey *key = dynamic_cast<TKey *>(keyIter.Next());
        
        if (not key)  // the end of the list has been reached
            break;
        
        
        // Consider only one-dimensional histograms
        string const className(key->GetClassName());
        
        if (className != "TH1D" and className != "TH1F" and className != "TH1I" and
         className != "TH1S" and className != "TH1C")
            continue;
        
        
        // Skip the data histogram and histograms with systematics
        string const keyName(key->GetName());
        
        if (keyName == "data" or keyName == "syst_up" or keyName == "syst_down")
            continue;
        
        
        // Read the histogram associated with the current key
        mcHists.emplace_back(dynamic_cast<TH1 *>(curDirectory->Get(keyName.c_str())));
    }
    
    
    if (mcHists.size() == 0)
    {
        ostringstream ost;
        ost << "Failed to find any MC histograms in file \"" << srcFileName << "\", directory \"" <<
         dirName << "\".";
        throw runtime_error(ost.str());
    }
    
    
    // Remove association of histograms with the source file so that the histogram are not deleted
    //when the file is closed
    dataHist->SetDirectory(nullptr);
    
    for (auto &h: mcHists)
        h->SetDirectory(nullptr);
}
