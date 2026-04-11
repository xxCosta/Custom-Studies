#include "sierrachart.h"
SCDLLName("Entry Boi");

struct Order {
  float size;
  double slPrice;
  double entryPrice;
  double tpPrice;
  double triggerPrice;
  double safeEntry;
  int entryOrderID;
  int orderMode;
};

struct Account {
  int size;
  int risk;
};

int sizeOrder(Order *o, SCStudyInterfaceRef sc, Account *a) {

  float tickValue = sc.CurrencyValuePerTick;
  int riskDistance = sc.Subgraph[0][sc.Index];
  float riskValue = tickValue * riskDistance;

  float accRisk = a->size * (a->risk / 100.0f);

  int size =
      (int)(riskValue / accRisk); // gonna need to figure out lot size as well for forex

  return size;
}

s_SCNewOrder buildOrder(Order *o, SCStudyInterfaceRef sc, Account *a) {
  s_SCNewOrder newOrder = s_SCNewOrder();

  double trailTrigPrice;
  double trailOffset;

  trailTrigPrice = fabs(o->tpPrice - o->safeEntry) * 0.8;
  trailOffset = fabs(o->safeEntry - o->slPrice) * 0.8;

  newOrder.OrderType = SCT_ORDERTYPE_TRIGGERED_STOP;
  newOrder.TimeInForce = SCT_TIF_DAY;
  // newOrder.AttachedOrderStop1Type = SCT_ORDERTYPE_STOP;
  // newOrder.Stop1Price = o->slPrice;
  newOrder.AttachedOrderStop1Type = SCT_ORDERTYPE_TRIGGERED_TRAILING_STOP_3_OFFSETS;
  newOrder.Stop1Offset = fabs(o->safeEntry - o->slPrice);
  newOrder.AttachedOrderStop1_TriggeredTrailStopTrailPriceOffset = trailOffset;
  newOrder.AttachedOrderStop1_TriggeredTrailStopTriggerPriceOffset = trailTrigPrice;
  if (o->orderMode == 0) {
    newOrder.AttachedOrderTarget1Type = SCT_ORDERTYPE_LIMIT;
    newOrder.Target1Price = o->tpPrice;
  }
  newOrder.Price1 = o->safeEntry;
  newOrder.Price2 = o->triggerPrice;

  newOrder.OrderQuantity = sizeOrder(o, sc, a);
  return newOrder;
};

void orientateOrder(Order *o, SCInputRef rr, double safeEntryPercentage,
                    SCInputRef iTrigPCT) {
  std::swap(o->slPrice, o->entryPrice);

  double tpBeforeSafeEntry =
      ((o->entryPrice - o->slPrice) * rr.GetFloat()) + o->entryPrice;

  double triggerPercentage = iTrigPCT.GetDouble();
  o->triggerPrice = o->entryPrice - triggerPercentage * (o->entryPrice - o->slPrice);
  o->safeEntry =
      o->entryPrice + safeEntryPercentage * (tpBeforeSafeEntry - o->entryPrice);

  double tpAfterSafeEntry = ((o->safeEntry - o->slPrice) * rr.GetFloat()) + o->safeEntry;
  o->tpPrice = tpAfterSafeEntry;
}

int getPips(double entry, double sl, SCStudyInterfaceRef sc) {

  double tickSize = sc.TickSize;
  double e = entry;
  double s = sl;
  while (tickSize <= 0.9999) {
    e *= 10;
    s *= 10;
    tickSize *= 10;
  }
  return (int)std::round(std::abs(e - s));
}

SCSFExport scsf_rectangleBoxEntry(SCStudyInterfaceRef sc) {

  SCInputRef rr = sc.Input[0];
  SCInputRef inputSafeEntry = sc.Input[1];
  SCInputRef inputTriggerPercentage = sc.Input[2];
  SCInputRef inputOrderMode = sc.Input[3];
  SCInputRef inputAccountSize = sc.Input[4];
  SCInputRef inputAccountRisk = sc.Input[5];

  if (sc.SetDefaults) {
    sc.GraphName = "Entry Boi - S&D Entry";
    sc.AutoLoop = 1;
    sc.UpdateAlways = 1;
    sc.HideStudy = 0;

    sc.GraphRegion = 0;
    sc.DisplayStudyName = 0;
    sc.DisplayStudyInputValues = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;

    sc.CancelAllWorkingOrdersOnExit = 1;
    sc.AllowMultipleEntriesInSameDirection = 1;
    sc.MaximumPositionAllowed = 200;
    sc.AllowOnlyOneTradePerBar = 0;

    sc.Subgraph[0].Name = "risk in pips";

    rr.Name = "Risk:Reward Ratio";
    rr.SetFloat(1.3f);

    inputSafeEntry.Name = "Safe Entry Percentage";
    inputSafeEntry.SetFloat(0.2f);

    inputTriggerPercentage.Name = "trigger Percentage";
    inputTriggerPercentage.SetDouble(0.10);

    inputOrderMode.Name = "order mode";
    inputOrderMode.SetCustomInputStrings("challenge; runners");
    inputOrderMode.SetCustomInputIndex(0);

    inputAccountSize.Name = "Account Size";
    inputAccountSize.SetInt(25000);

    inputAccountRisk.Name = "Account Risk (%)";
    inputAccountRisk.SetInt(2);

    return;
  }

  if (sc.IsFullRecalculation || sc.DownloadingHistoricalData)
    return;

  double safeEntryPercentage = inputSafeEntry.GetFloat();
  int chartNum = sc.ChartNumber;
  s_UseTool entryBox;

  Order *entryOrder = (Order *)sc.GetPersistentPointer(0);
  if (entryOrder == nullptr) {
    entryOrder = new Order;
    sc.SetPersistentPointer(0, entryOrder);
  }

  Account *account = (Account *)sc.GetPersistentPointer(1);
  if (account == nullptr) {
    account = new Account;
    sc.SetPersistentPointer(1, account);
  }
  account->size = inputAccountSize.GetInt();
  account->risk = inputAccountRisk.GetInt();

  if (sc.GetUserDrawnChartDrawing(chartNum, DRAWING_RECTANGLEHIGHLIGHT, entryBox, -1)) {
    if (entryOrder->slPrice != entryBox.BeginValue ||
        entryOrder->entryPrice != entryBox.EndValue) {
      entryOrder->slPrice = entryBox.BeginValue;
      entryOrder->entryPrice = entryBox.EndValue;
      entryOrder->orderMode = inputOrderMode.GetIndex();

      double tpBeforeSafeEntry =
          ((entryOrder->entryPrice - entryOrder->slPrice) * rr.GetFloat()) +
          entryOrder->entryPrice;

      double triggerPercentage = inputTriggerPercentage.GetDouble();
      entryOrder->triggerPrice =
          entryOrder->entryPrice -
          triggerPercentage * (entryOrder->entryPrice - entryOrder->slPrice);
      entryOrder->safeEntry =
          entryOrder->entryPrice +
          safeEntryPercentage * (tpBeforeSafeEntry - entryOrder->entryPrice);

      double tpAfterSafeEntry =
          ((entryOrder->safeEntry - entryOrder->slPrice) * rr.GetFloat()) +
          entryOrder->safeEntry;
      entryOrder->tpPrice = tpAfterSafeEntry;

      // sc.AddMessageToLog(SCString().Format("entry at %.3f", entryOrder->entryPrice),
      // 0); sc.AddMessageToLog(SCString().Format("tp at%.3f", tpAfterSafeEntry), 0);
      // sc.AddMessageToLog(SCString().Format("tp at%.3f", entryOrder->tpPrice), 0);
      // sc.AddMessageToLog(SCString().Format("sl  at %.3f", entryOrder->slPrice), 0);

      s_UseTool showPips;

      showPips.Clear(); // reset tool structure for our next use
      showPips.ChartNumber = sc.ChartNumber;
      showPips.DrawingType = DRAWING_TEXT;
      showPips.LineNumber = 5035;
      showPips.BeginDateTime = entryBox.EndDateTime;
      showPips.BeginValue = (entryBox.BeginValue + entryBox.EndValue) / 2;
      showPips.Region = sc.GraphRegion;
      showPips.Color = RGB(138, 197, 255);
      int pips = getPips(entryOrder->safeEntry, entryOrder->slPrice, sc);
      sc.SetPersistentInt(1, pips);
      showPips.Text.Format("%d", pips);

      sc.UseTool(showPips);
    }
  }
  sc.Subgraph[0][sc.Index] = sc.GetPersistentInt(1);
  std::vector<int> vGuiButtons = {1, 2};
  for (int x : vGuiButtons) {
    if (sc.GetCustomStudyControlBarButtonEnableState(x) == 1) {
      if (x == 1) {

        if (entryOrder->slPrice > entryOrder->entryPrice) {
          orientateOrder(entryOrder, rr, safeEntryPercentage, inputTriggerPercentage);
        }

        s_SCNewOrder newOrder = buildOrder(entryOrder, sc, account);
        sc.SetAttachedOrders(newOrder);
        entryOrder->entryOrderID = sc.BuyEntry(newOrder);

      } else if (x == 2) {
        if (entryOrder->entryPrice > entryOrder->slPrice) {
          orientateOrder(entryOrder, rr, safeEntryPercentage, inputTriggerPercentage);
        }

        s_SCNewOrder newOrder = buildOrder(entryOrder, sc, account);
        sc.SetAttachedOrders(newOrder);
        entryOrder->entryOrderID = sc.SellEntry(newOrder);
      }
      sc.SetCustomStudyControlBarButtonEnable(x, 0);
    }
  }
  s_SCTradeOrder currentOrder;
  if (sc.GetOrderByIndex(0, currentOrder)) {
    s_SCTradeOrder slCurrentOrder;
    sc.GetOrderByOrderID(currentOrder.StopChildInternalOrderID, slCurrentOrder);

    double entry = currentOrder.Price1;
    double stop = slCurrentOrder.Price1;
    double price = sc.GetLastPriceForTrading();
    double pct = 0.10;

    double buffer = fabs(entry - stop) * pct;

    bool isLong = (entry > stop);
    bool isShort = (entry < stop);

    double hardStop;

    if (slCurrentOrder.InternalOrderID == 0 || slCurrentOrder.Price1 == 0)
      return; // this so that the hardstop doesnt get triggered when sl is moved
    // there is a split second where the below stuff gets triggered because the sl
    // is 0.

    if (isLong) {
      hardStop = stop - buffer;
      if (price < hardStop)
        sc.CancelAllOrders();
    } else if (isShort) {
      hardStop = stop + buffer;
      if (price > hardStop) {
        sc.AddMessageToLog("cancelled", 0);
        sc.CancelAllOrders();
      }
    }
  }
}
