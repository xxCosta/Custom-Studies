#include "sierrachart.h"
#include <algorithm>
#include <vector>

SCDLLName("replay-manager")

    SCSFExport scsf_ToggleJump(SCStudyInterfaceRef sc) {
  if (sc.IsFullRecalculation || sc.DownloadingHistoricalData)
    return;
  if (sc.SetDefaults) {
    sc.GraphName = "Replay Manager";
    sc.AutoLoop = 1;
    sc.UpdateAlways = 1;

    sc.MaintainAdditionalChartDataArrays = 1;

    return;
  }

  int &ffActive = sc.GetPersistentInt(1);
  int &ffInSession = sc.GetPersistentInt(2);

  SCSubgraphRef currentTimeS = sc.Subgraph[0];
  currentTimeS.Name = "Current Time in Seconds";
  int goalTime = 68200; // in seconds

  currentTimeS[sc.Index] = sc.BaseDateTimeIn[sc.Index].GetTimeInSeconds();
  int ct = currentTimeS[sc.Index];
  if (sc.GetCustomStudyControlBarButtonEnableState(3)) {
    sc.ChangeChartReplaySpeed(sc.ChartNumber, 17000);
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
