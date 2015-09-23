#pragma once

#include <string>


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
    DataMCPlot(std::string const &srcFileName, std::string const &directory = "");
};
