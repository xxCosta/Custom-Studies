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
    
    // SETTING TRADE MODE
    SCInputRef isTradeModeActive = sc.Input[1];
    isTradeModeActive.Name = "Trade Mode Active?";
    isTradeModeActive.SetYesNo(1);
    bool tradeMode = isTradeModeActive.GetBoolean();

	// Section 1 - Set the configuration variables and defaults
	if (sc.SetDefaults)
	{
		sc.GraphName = "Entry Boi - S&D Entry";
        sc.UpdateAlways = 1;
		sc.AutoLoop = 1; 
		sc.HideStudy = 1;	
		return;
	}

    int chartNum = 2;	
    s_UseTool entryBox;	
    float& slPrice = sc.GetPersistentFloat(0);
    float& entryPrice = sc.GetPersistentFloat(1);
    float& tpPrice = sc.GetPersistentFloat(2);
    
    

    if(sc.GetUserDrawnChartDrawing(chartNum, DRAWING_RECTANGLEHIGHLIGHT, entryBox, -1)){

        if(slPrice != entryBox.BeginValue || entryPrice != entryBox.EndValue ){
            slPrice = entryBox.BeginValue;
            entryPrice = entryBox.EndValue;

            float tp = ((entryPrice - slPrice)*rr.GetFloat()) + entryPrice;
            tpPrice = tp; 

            sc.AddMessageToLog(SCString().Format("entry at %.3f", entryPrice),0);
            sc.AddMessageToLog(SCString().Format("tp at%.3f", tp),0);
            sc.AddMessageToLog(SCString().Format("sl  at %.3f", slPrice),0);
        }

        // ORDER STUFF
        int& entryOrderID = sc.GetPersistentInt(0);
        s_SCNewOrder entryOrder;
        entryOrder.OrderQuantity = 1;
        entryOrder.OrderType = SCT_ORDERTYPE_LIMIT;
        entryOrder.Price1 = entryPrice;
        entryOrder.TimeInForce = SCT_TIF_GOOD_TILL_CANCELED;
        s_SCNewOrder stopOrder;
        s_SCNewOrder tpOrder;

        // RUN DAT CRODIE
        if(tradeMode){
            if(tpPrice>slPrice){
                if(entryOrderID <= 0){
                    int orderPlaced = sc.BuyEntry(entryOrder);
                    entryOrderID = orderPlaced;
                }
            }else {

                if(entryOrderID <= 0){
                    int orderPlaced = sc.SellEntry(entryOrder);
                    entryOrderID = orderPlaced;
                }




            }
        }

    }



    
    
    


    
}
