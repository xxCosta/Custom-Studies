//
//
// INSTRUCTIONS:
// B to Buy 
// S to Sell 
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
#include <utility>
SCDLLName("Entry Boi")

void orientateOrder(float& sl, float& tp, float& trig, float& safeEntry, float& entry, SCInputRef rr){
    std::swap(sl,entry);

            float tpBeforeSafeEntry = ((entry - sl)*rr.GetFloat()) + entry;

            double triggerPercentage = 0.30; //how deep is it gonna retrace (TODO: set this number to a user input) 
            double safeEntryPercentage = 0.10;
            trig = entry - triggerPercentage * (entry - sl); //refer to math stuff in your journal for an explantion on this formula               
            safeEntry = entry + safeEntryPercentage * (tpBeforeSafeEntry - entry);

            float tpAfterSafeEntry = ((safeEntry - sl)*rr.GetFloat()) + safeEntry;//i want to tp to be calculated based on the safe entry not the edge of box,i'll have to recalculate
            tp = tpAfterSafeEntry; 
}

SCSFExport scsf_rectangleBoxEntry(SCStudyInterfaceRef sc)
{


    SCInputRef rr = sc.Input[0];
	// Section 1 - Set the configuration variables and defaults
	if (sc.SetDefaults)
	{
		sc.GraphName = "Entry Boi - S&D Entry";
        sc.UpdateAlways = 1;
		sc.AutoLoop = 1; 
		sc.HideStudy = 1;

        rr.Name = "risk:reward ratio";
        rr.SetFloat(1.5f);
		return;
	}
    sc.MaximumPositionAllowed = 200;
    sc.AllowOnlyOneTradePerBar = 0;

    int chartNum = 2;	
    s_UseTool entryBox;	
    float& slPrice = sc.GetPersistentFloat(0);
    float& entryPrice = sc.GetPersistentFloat(1);
    float& tpPrice = sc.GetPersistentFloat(2);
    float& triggerPrice = sc.GetPersistentFloat(3);
    float& safeEntryPriceAfterTrigHit = sc.GetPersistentFloat(4);
    
    if(sc.GetUserDrawnChartDrawing(chartNum, DRAWING_RECTANGLEHIGHLIGHT, entryBox, -1)){

        if(slPrice != entryBox.BeginValue || entryPrice != entryBox.EndValue ){
            slPrice = entryBox.BeginValue;
            entryPrice = entryBox.EndValue;

            float tpBeforeSafeEntry = ((entryPrice - slPrice)*rr.GetFloat()) + entryPrice;

            double triggerPercentage = 0.30; //how deep is it gonna retrace (TODO: set this number to a user input) 
            double safeEntryPercentage = 0.10;
            triggerPrice = entryPrice - triggerPercentage * (entryPrice - slPrice); //refer to math stuff in your journal for an explantion on this formula               
            safeEntryPriceAfterTrigHit = entryPrice + safeEntryPercentage * (tpBeforeSafeEntry - entryPrice);

            float tpAfterSafeEntry = ((safeEntryPriceAfterTrigHit - slPrice)*rr.GetFloat()) + safeEntryPriceAfterTrigHit;//i want to tp to be calculated based on the safe entry not the edge of box,i'll have to recalculate
            tpPrice = tpAfterSafeEntry; 


            sc.AddMessageToLog(SCString().Format("entry at %.3f", entryPrice),0);
            sc.AddMessageToLog(SCString().Format("tp at%.3f", tpAfterSafeEntry),0);
            sc.AddMessageToLog(SCString().Format("tp at%.3f", tpPrice),0);
            sc.AddMessageToLog(SCString().Format("sl  at %.3f", slPrice),0);
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
        entryOrder.Price1 = safeEntryPriceAfterTrigHit; 
        entryOrder.Price2 = triggerPrice; 

        
        //by default, when a button is pressed its just a toggle
        //this ensures that it acts as a click rather than a toggle,
        //allowing me to fire things once through the button
        std::vector<int> vGuiButtons = {1,2};
        for (int x : vGuiButtons){
            if(sc.GetCustomStudyControlBarButtonEnableState(x) == 1){

                sc.SetAttachedOrders(entryOrder);

                if(x==1){
                    if(slPrice>entryPrice){
                        orientateOrder(slPrice,tpPrice,triggerPrice,safeEntryPriceAfterTrigHit,entryPrice,rr);
                        sc.AddMessageToLog("swapped",0);
                        
                        //i have to rebuild the order after this. I know that my core idea is good but the execution is bad 
                        //there is no reason i should have to repeat code like this, its really inefficient to read and write
                        //TODO: rewrite the whole tool in a readable and efficient manner
                        entryOrder.Stop1Price = slPrice;
                        entryOrder.Target1Price = tpPrice;
                        entryOrder.Price1 = safeEntryPriceAfterTrigHit; 
                        entryOrder.Price2 = triggerPrice; 
                    }
                    int orderPlaced = sc.BuyEntry(entryOrder);
                    sc.AddMessageToLog(sc.GetTradingErrorTextMessage(orderPlaced),0);
                    entryOrderID = orderPlaced;
                }else if(x==2) {
                    if(entryPrice>slPrice){
                        orientateOrder(slPrice,tpPrice,triggerPrice,safeEntryPriceAfterTrigHit,entryPrice,rr);
                        sc.AddMessageToLog("swapped",0); 
                        entryOrder.Stop1Price = slPrice;
                        entryOrder.Target1Price = tpPrice;
                        entryOrder.Price1 = safeEntryPriceAfterTrigHit; 
                        entryOrder.Price2 = triggerPrice; 
                    }
                    int orderPlaced = sc.SellEntry(entryOrder);
                    entryOrderID = orderPlaced;
                }
                sc.SetCustomStudyControlBarButtonEnable(x,0);
            }

        }
    }



    
    
    


    
}
