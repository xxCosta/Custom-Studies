// The top of every source code file must include this line
#include "sierrachart.h"

// For reference, refer to this page:
// https://www.sierrachart.com/index.php?page=doc/AdvancedCustomStudyInterfaceAndLanguage.php

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies. 
SCDLLName("Entry Boi")

//This is the basic framework of a study function. Change the name 'TemplateFunction' to what you require.
SCSFExport scsf_rectangleBoxEntry(SCStudyInterfaceRef sc)
{

    SCInputRef rr = sc.Input[0];
    rr.Name = "risk:reward ratio";
    rr.SetFloat(1);

	// Section 1 - Set the configuration variables and defaults
	if (sc.SetDefaults)
	{
		sc.GraphName = "Entry Boi - S&D Entry";
        sc.UpdateAlways = 0;
		sc.AutoLoop = 1; 
		sc.HideStudy = 1;	
		return;
	}

    int chartNum = 2;	
    s_UseTool entryBox;	
    float& slPriceBottomOfBox = sc.GetPersistentFloat(0);
    float& entryPriceTopOfBox = sc.GetPersistentFloat(1);
    float& tpPrice = sc.GetPersistentFloat(2);
    
    

    if(sc.GetUserDrawnChartDrawing(chartNum, DRAWING_RECTANGLEHIGHLIGHT, entryBox, -1)){

        if(slPriceBottomOfBox != entryBox.BeginValue || entryPriceTopOfBox != entryBox.EndValue ){
            slPriceBottomOfBox = entryBox.BeginValue;
            entryPriceTopOfBox = entryBox.EndValue;

            float tp = ((entryPriceTopOfBox - slPriceBottomOfBox)*rr.GetFloat()) + entryPriceTopOfBox;

            sc.AddMessageToLog(SCString().Format("entry at %.3f", entryPriceTopOfBox),0);
            sc.AddMessageToLog(SCString().Format("tp at%.3f", tp),0);
            sc.AddMessageToLog(SCString().Format("sl  at %.3f", slPriceBottomOfBox),0);
        }

    }


    
}
