#include "sierrachart.h"
SCDLLName("replay-manager")

int replaySpeed = 16000;

SCSFExport scsf_ReplayMaster(SCStudyInterfaceRef sc) {
  if (sc.SetDefaults) {
    sc.GraphName = "Replay Manager - Master";
    sc.AutoLoop = 1;
    sc.UpdateAlways = 1;

    sc.DisplayStudyInputValues = 0;
    sc.DisplayStudyName = 0;
    sc.GraphRegion = 0;
    sc.GlobalDisplayStudySubgraphsNameAndValue = 0;

    sc.Input[0].SetStudyID(8);

    sc.Input[2].Name = "start date";
    sc.Input[2].SetDate(0);

    sc.Input[1].Name = "Goal Time (0-23)";
    sc.Input[1].SetInt(18);
    sc.MaintainAdditionalChartDataArrays = 1;

    return;
  }

  if (sc.IsFullRecalculation || sc.DownloadingHistoricalData)
    return;

  SCDateTime replayStartDate = sc.Input[2].GetDate();
  n_ACSIL::s_ChartReplayParameters rparams;
  rparams.ReplaySpeed = 1;
  rparams.StartDateTime = replayStartDate;
  rparams.SkipEmptyPeriods = 0;
  rparams.ReplayMode = n_ACSIL::REPLAY_MODE_STANDARD;
  rparams.ClearExistingTradeSimulationDataForSymbolAndTradeAccount = 0;

  if (sc.GetCustomStudyControlBarButtonEnableState(4)) {
    sc.StartChartReplayNew(rparams);
    sc.SetCustomStudyControlBarButtonEnable(4, 0);
  }

  int &ffActive = sc.GetPersistentInt(1);
  int &ffInSession = sc.GetPersistentInt(2);
  SCSubgraphRef currentTimeS = sc.Subgraph[0];
  currentTimeS.DrawStyle = DRAWSTYLE_HIDDEN;
  currentTimeS.Name = "Current Time in Seconds";
  int goalTime = 67000; // in seconds

  currentTimeS[sc.Index] = sc.BaseDateTimeIn[sc.Index].GetTimeInSeconds();
  int ct = currentTimeS[sc.Index];
  if (sc.GetCustomStudyControlBarButtonEnableState(3)) {
    sc.ChangeChartReplaySpeed(sc.ChartNumber, replaySpeed);
    ffActive = true;
  }

  sc.SetCustomStudyControlBarButtonEnable(3, 0);

  if (ffActive && ct >= goalTime && !ffInSession) {
    sc.ChangeChartReplaySpeed(sc.ChartNumber, 1);
    ffActive = false;
    ffInSession = true;
  }

  if (ct < goalTime) {
    ffInSession = false;
  }
}
