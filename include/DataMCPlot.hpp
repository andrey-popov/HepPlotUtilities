#pragma once

#include <TH1.h>
#include <TCanvas.h>

#include <memory>
#include <string>
#include <list>
#include <utility>


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
    /// Returns the title of the plot
    std::string const &GetTitle() const;
    
    /**
     * \brief Returns histogram with the given name
     * 
     * The method loops over all histograms to find the requested one and thus can be slow. Returns
     * an empty pointer if the requested histogram is not found.
     */
    std::shared_ptr<TH1> GetHist(std::string const &name) const;
    
    /**
     * \brief Rescales all MC histograms so that the total expectation equals normalization of data
     * 
     * The method must be called before the figure is drawn. If the argument is true, the histograms
     * are assumed to represent event density, and the normalization is calculated taking bin widths
     * into account. If the argument is false, the normalization is simply calculated as a sum of
     * content of all bins. In both cases under- and overflow bins are taken into account.
     */
    void NormalizeMCToData(bool isDensity);
    
    /**
     * \brief Enables or disables plotting of the residuals
     * 
     * The method must be called before the figure is drawn. The last two arguments define the
     * range for the residuals.
     */
    void RequestResiduals(bool plotResiduals, double min = -0.25, double max = 0.28);
    
    /// Draw the figure
    TCanvas &Draw();
    
    /**
     * \brief Draws CMS label in the upper left part of the figure
     * 
     * The figure must have been drawn before calling this method. The additional text provided as
     * the argument is written after the CMS label using a different font. Typical examples are
     * "Simulation", "Preliminary".
     */
    void AddCMSLabel(std::string const &additionalText = "");
    
    /**
     * \brief Draws a label with energy in the upper right part of the figure
     * 
     * The figure must have been drawn before calling this method. The text can be arbitrary, but
     * typical format is "20 fb^{-1} (8 TeV)".
     */
    void AddEnergyLabel(std::string const &text);
    
    /**
     * \brief Returns pointer to the legend
     * 
     * The pointer is null before the figure is drawn.
     */
    std::unique_ptr<TLegend> const &GetLegend();
    
    /**
     * \brief Returns pointer to the main pad, in which data and MC histograms are drawn
     * 
     * The pointer is null before the figure is drawn.
     */
    std::unique_ptr<TPad> const &GetMainPad();
    
    /**
     * \brief Prints the canvas to a file
     * 
     * This method is preferred to calling Print for the returned canvas since in that case the
     * legend will not be saved.
     */
    void Print(std::string const &fileName);
    
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
    std::shared_ptr<TH1> dataHist;
    
    /// MC histograms
    std::list<std::shared_ptr<TH1>> mcHists;
    
    /// Indicates if the data/MC residuals should be plotted
    bool plotResiduals;
    
    /**
     * \brief Range for residuals
     * 
     * First value is the minimum, second one is the maximum.
     */
    std::pair<double, double> residualsRange;
    
    /// Canvas to host the figure
    std::unique_ptr<TCanvas> canvas;
    
    /// Pad that hosts the main graph
    std::unique_ptr<TPad> mainPad;
    
    /// Legend
    std::unique_ptr<TLegend> legend;
    
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
