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
    
    /// Destructor
    ~DataMCPlot();
    
public:
    /// Draw the figure
    TCanvas &Draw();
    
private:
    /// Reads histograms from a ROOT file
    void ReadFile(std::string const &srcFileName, std::string const &dirName);
    
    /**
     * \brief Creates an object of type T passing arguments Args to its constructor and saves a
     * pointer to it to the ownedObjects container
     * 
     * This method is used to create ROOT objects that will be drawn in the canvas.
     */
    template<typename T, typename... Args>
    T *NewOwnedObject(Args... args);
    
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
    
    /// Canvas to host the figure
    std::unique_ptr<TCanvas> canvas;
    
    /**
     * \brief List of owned ROOT objects to be deleted by the destructor
     * 
     * It seems ROOT offers no way to make a deep copy of a canvas (TCanvas::Clone still preserves
     * some links to objects included in the original canvas). And the objects drawn in the canvas
     * must not be deleted since the canvas does not own them or keeps a copy. This list keeps track
     * of all drawn objects.
     */
    std::list<TObject *> ownedObjects;
};


template<typename T, typename... Args>
T *DataMCPlot::NewOwnedObject(Args... args)
{
    T *obj = new T(args...);
    ownedObjects.emplace_back(obj);
    return obj;
}
