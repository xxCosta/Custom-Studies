//
//
// INSTRUCTIONS:
// O to open orders
// C to cancel current orders
// T to take profit
//
//
//
//
//
//
//
//
//
//
//
//
//
//
#include "sierrachart.h"
#include <vector>
SCDLLName("Entry Boi")


SCSFExport scsf_rectangleBoxEntry(SCStudyInterfaceRef sc)
{

    SCInputRef rr = sc.Input[0];
    rr.Name = "risk:reward ratio";
    rr.SetFloat(1);

	// Section 1 - Set the configuration variables and defaults
	if (sc.SetDefaults)
	{
		sc.GraphName = "Entry Boi - S&D Entry";
        sc.UpdateAlways = 1;
		sc.AutoLoop = 1; 
		sc.HideStudy = 1;
		return;
	}
    sc.MaximumPositionAllowed = 200;
    sc.AllowOnlyOneTradePerBar = 0;

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

            sc.AddMessageToLog(SCString().Format("entry at %.3f\ntp at %.3f\nsl at %.3f", entryPrice,tp,slPrice),0);
            // sc.AddMessageToLog(SCString().Format("tp at%.3f", tp),0);
            // sc.AddMessageToLog(SCString().Format("sl  at %.3f", slPrice),0);
        }

        // ORDER STUFF
        int& entryOrderID = sc.GetPersistentInt(0);
        s_SCNewOrder entryOrder;
        entryOrder.OrderQuantity = 100;
        entryOrder.OrderType = SCT_ORDERTYPE_TRIGGERED_STOP;
        entryOrder.TimeInForce = SCT_TIF_GOOD_TILL_CANCELED;
        entryOrder.AttachedOrderStop1Type = SCT_ORDERTYPE_STOP;
        entryOrder.Stop1Price = slPrice;
        entryOrder.AttachedOrderTarget1Type = SCT_ORDERTYPE_LIMIT;
        entryOrder.Target1Price = tpPrice;

        if(sc.GetCustomStudyControlBarButtonEnableState(1)){
            double triggerPercentage = 0.30; //how deep is it gonna retrace (TODO: set this number to a user input) 
            double safeEntryPercentage = 0.10;
            float triggerPrice = entryPrice - triggerPercentage * (entryPrice - slPrice); //refer to math stuff in your journal for an explantion on this formula               
            float safeEntryPriceAfterTrigHit = entryPrice + safeEntryPercentage * (tpPrice - entryPrice);  
            entryOrder.Price1 = safeEntryPriceAfterTrigHit; 
            entryOrder.Price2 = triggerPrice; 


            if(tpPrice>slPrice){
                sc.SetAttachedOrders(entryOrder);
                
                int orderPlaced = sc.BuyEntry(entryOrder);
                sc.AddMessageToLog(sc.GetTradingErrorTextMessage(orderPlaced),0);
                entryOrderID = orderPlaced;
            }else {
                sc.SetAttachedOrders(entryOrder);

                int orderPlaced = sc.SellEntry(entryOrder);
                entryOrderID = orderPlaced;
            }
        }
        
        //when a button is pressed its just a toggle
        //this ensures that it acts as a click rather than a toggle,
        //allowing me to fire things once through the button
        std::vector<int> vGuiButtons = {1,2};
        for (int x : vGuiButtons){
            if(sc.GetCustomStudyControlBarButtonEnableState(x) == 1){
                sc.SetCustomStudyControlBarButtonEnable(x,0);
            }

        }
    }



    
    
    


    
}
