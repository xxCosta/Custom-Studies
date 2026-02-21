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


struct Order {
  float slPrice;
  float entryPrice;
  float tpPrice;
  float triggerPrice;
  float safeEntryPriceAfterTrigHit;
  int entryOrderID;
};


s_SCNewOrder buildOrder(Order* o ){

  s_SCNewOrder newOrder = s_SCNewOrder();

  newOrder.OrderQuantity = 100;
  newOrder.OrderType = SCT_ORDERTYPE_TRIGGERED_STOP;
  newOrder.TimeInForce = SCT_TIF_DAY;
  newOrder.AttachedOrderStop1Type = SCT_ORDERTYPE_STOP;
  newOrder.Stop1Price = o->slPrice;
  newOrder.AttachedOrderTarget1Type = SCT_ORDERTYPE_LIMIT;
  newOrder.Target1Price = o->tpPrice;
  newOrder.Price1 = o->safeEntryPriceAfterTrigHit; 
  newOrder.Price2 = o->triggerPrice; 

  return newOrder;

};

void orientateOrder(Order* o, SCInputRef rr){
  std::swap(o->slPrice,o->entryPrice);

  float tpBeforeSafeEntry = ((o->entryPrice - o->slPrice)*rr.GetFloat()) + o->entryPrice;

  double triggerPercentage = 0.30; //how deep is it gonna retrace (TODO: set this number to a user input) 
  double safeEntryPercentage = 0.10;
  o->triggerPrice = o->entryPrice - triggerPercentage * (o->entryPrice - o->slPrice); //refer to math stuff in your journal for an explantion on this formula               
  o->safeEntryPriceAfterTrigHit = o->entryPrice + safeEntryPercentage * (tpBeforeSafeEntry - o->entryPrice);

  float tpAfterSafeEntry = ((o->safeEntryPriceAfterTrigHit - o->slPrice)*rr.GetFloat()) + o->safeEntryPriceAfterTrigHit;//i want to tp to be calculated based on the safe o->entryPrice not the edge of box,i'll have to recalculate
  o->tpPrice = tpAfterSafeEntry; 
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

  sc.CancelAllWorkingOrdersOnExit = 1;
  sc.AllowMultipleEntriesInSameDirection = 1;
  sc.MaximumPositionAllowed = 200;
  sc.AllowOnlyOneTradePerBar = 0;

  int chartNum = sc.ChartNumber;	
  s_UseTool entryBox;	

  Order* entryOrder = (Order*)sc.GetPersistentPointer(0);
  if(entryOrder == nullptr){

    entryOrder = new Order;
    sc.SetPersistentPointer(0, entryOrder);
  }
  
  //i need to wrap this in an if statement so that it only runs on open orders
  //also needs to be independent per chart so the multiple orders across...
  //different charts dont affect eachother.  
    //
    // double highHardStop = std::fmax(entryOrder->slPrice, entryOrder->safeEntryPriceAfterTrigHit);
    // double lowHardStop = std::fmin(entryOrder->slPrice, entryOrder->safeEntryPriceAfterTrigHit);
    // double currentPrice = sc.GetLastPriceForTrading();
    // double pctHardStop = 0.10;
    // double hardStop = lowHardStop - pctHardStop*(highHardStop-lowHardStop);
    // if(currentPrice < hardStop){
    //   sc.CancelAllOrders();
    // }
    //



  if(sc.GetUserDrawnChartDrawing(chartNum, DRAWING_RECTANGLEHIGHLIGHT, entryBox, -1)){
    if(entryOrder->slPrice != entryBox.BeginValue || entryOrder->entryPrice != entryBox.EndValue ){
      entryOrder->slPrice = entryBox.BeginValue;
      entryOrder->entryPrice = entryBox.EndValue;

      float tpBeforeSafeEntry = ((entryOrder->entryPrice - entryOrder->slPrice)*rr.GetFloat()) + entryOrder->entryPrice;

      double triggerPercentage = 0.30; //how deep is it gonna retrace (TODO: set this number to a user input) 
      double safeEntryPercentage = 0.18;
      entryOrder->triggerPrice = entryOrder->entryPrice - triggerPercentage * (entryOrder->entryPrice - entryOrder->slPrice); //refer to math stuff in your journal for an explantion on this formula               
      entryOrder->safeEntryPriceAfterTrigHit = entryOrder->entryPrice + safeEntryPercentage * (tpBeforeSafeEntry - entryOrder->entryPrice);

      float tpAfterSafeEntry = ((entryOrder->safeEntryPriceAfterTrigHit - entryOrder->slPrice)*rr.GetFloat()) + entryOrder->safeEntryPriceAfterTrigHit;//i want to tp to be calculated based on the safe entry not the edge of box,i'll have to recalculate
      entryOrder->tpPrice = tpAfterSafeEntry; 


      sc.AddMessageToLog(SCString().Format("entry at %.3f", entryOrder->entryPrice),0);
      sc.AddMessageToLog(SCString().Format("tp at%.3f", tpAfterSafeEntry),0);
      sc.AddMessageToLog(SCString().Format("tp at%.3f", entryOrder->tpPrice),0);
      sc.AddMessageToLog(SCString().Format("sl  at %.3f", entryOrder->slPrice),0);
    }

    //by default, when a button is pressed its just a toggle
    //this ensures that it acts as a click rather than a toggle,
    //allowing me to fire things once through the button
    std::vector<int> vGuiButtons = {1,2};
    for (int x : vGuiButtons){
      if(sc.GetCustomStudyControlBarButtonEnableState(x) == 1){
        if(x==1){

          if(entryOrder->slPrice>entryOrder->entryPrice){
            orientateOrder(entryOrder, rr);
            sc.AddMessageToLog("swapped",0); 
          }
          
          s_SCNewOrder newOrder = buildOrder(entryOrder);
          sc.SetAttachedOrders(newOrder);
          entryOrder->entryOrderID= sc.BuyEntry(newOrder);

        }else if(x==2) {
          if(entryOrder->entryPrice>entryOrder->slPrice){
            orientateOrder(entryOrder, rr);
            sc.AddMessageToLog("swapped",0); 
          }

          s_SCNewOrder newOrder = buildOrder(entryOrder);
          sc.SetAttachedOrders(newOrder);
          entryOrder->entryOrderID = sc.SellEntry(newOrder);
        }
        sc.SetCustomStudyControlBarButtonEnable(x,0);
      }

    }

    s_SCTradeOrder currentOrder;
    if(sc.GetOrderByIndex(0,currentOrder)){
      s_SCTradeOrder slCurrentOrder;

      sc.GetOrderByOrderID(currentOrder.StopChildInternalOrderID, slCurrentOrder);
      double highHardStop = std::fmax(currentOrder.Price1, slCurrentOrder.Price1);
      double lowHardStop = std::fmin(currentOrder.Price1, slCurrentOrder.Price1);
      double currentPrice = sc.GetLastPriceForTrading();
      double pctHardStop = 0.10;
      double hardStop = lowHardStop - pctHardStop*(highHardStop-lowHardStop);
      if(currentPrice < hardStop){
        sc.CancelAllOrders();
      }

    }






  }










}
