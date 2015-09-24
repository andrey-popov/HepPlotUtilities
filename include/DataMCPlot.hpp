#pragma once

#include <TH1.h>
#include <TCanvas.h>

#include <memory>
#include <string>
#include <list>


/**
 * \class DataMCPlot
 * \brief Creates a plot with a comparison of data and MC using provided histograms
 */
class DataMCPlot
{
public:
    /**
     * \brief Constructor
     * 
     * The arguments are the name of the ROOT file with histograms to be plotted and the name of the
     * directory in the file that contains the histograms.
     */
    DataMCPlot(std::string const &srcFileName, std::string const &dirName = "");
    
    /// Copy constructor is deleted
    DataMCPlot(DataMCPlot const &) = delete;
    
    /// Assignment operator is deleted
    DataMCPlot &operator=(DataMCPlot const &) = delete;
    
public:
    /// Draw the plot
    std::shared_ptr<TCanvas> Draw();
    
private:
    /// Reads histograms from a ROOT file
    void ReadFile(std::string const &srcFileName, std::string const &dirName);
    
private:
    /**
     * \brief Title of the plot
     * 
     * Follows the usual ROOT format, with axis titles included after semicolons
     */
    std::string title;
    
    /// Histogram with data points
    std::unique_ptr<TH1> dataHist;
    
    /// MC histograms
    std::list<std::unique_ptr<TH1>> mcHists;
    
    /// Indicates if the data/MC residuals should be plotted
    bool plotResiduals;
};
