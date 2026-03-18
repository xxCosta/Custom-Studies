#include "sierrachart.h"
SCDLLName("replay-manager")

int replaySpeed = 16000;

SCSFExport scsf_ReplayMaster(SCStudyInterfaceRef sc) {
  if (sc.SetDefaults) {
    sc.GraphName = "Replay Manager - Master";
    sc.GraphRegion = 1;
    sc.AutoLoop = 1;
    sc.UpdateAlways = 1;

    sc.Input[0].Name = "start date";
    sc.Input[0].SetDate(0);

    sc.Input[1].Name = "Goal Time (0-23)";
    sc.Input[1].SetInt(18);
    sc.MaintainAdditionalChartDataArrays = 1;

    return;
  }

  if (sc.IsFullRecalculation || sc.DownloadingHistoricalData)
    return;

  SCDateTime replayStartDate = sc.Input[0].GetDate();
  sc.SetPersistentSCDateTime(1, replayStartDate);
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
  currentTimeS.Name = "Current Time in Seconds";
  int goalTime = 68200; // in seconds

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

SCSFExport scsf_ReplaySlave(SCStudyInterfaceRef sc) {

  if (sc.SetDefaults) {
    sc.GraphName = "Replay Manager - Slave";
    sc.GraphRegion = 1;
    sc.AutoLoop = 1;
    sc.UpdateAlways = 1;

    sc.Input[0].Name = "start date";
    sc.Input[0].SetDate(0);

    sc.Input[1].Name = "Master Chart Number";
    sc.Input[1].SetInt(3);

    sc.Input[2].Name = "Master Chart Study ID";
    sc.Input[2].SetInt(8);

    sc.MaintainAdditionalChartDataArrays = 1;

    return;
  }
  if (sc.IsFullRecalculation || sc.DownloadingHistoricalData)
    return;

  struct mParams {
    int ChartNumber;
    int StudyId;
    SCDateTime Date;
    bool ffActive;
  } m;

  m.ChartNumber = sc.Input[1].GetInt();
  m.StudyId = sc.Input[2].GetInt();
  m.Date = sc.GetPersistentSCDateTimeFromChartStudy(m.ChartNumber, m.StudyId, 1);

  if (sc.GetCustomStudyControlBarButtonEnableState(4)) {
    n_ACSIL::s_ChartReplayParameters rparams;
    rparams.ReplaySpeed = 1;
    rparams.StartDateTime = m.Date;
    rparams.SkipEmptyPeriods = 0;
    rparams.ReplayMode = n_ACSIL::REPLAY_MODE_STANDARD;
    rparams.ClearExistingTradeSimulationDataForSymbolAndTradeAccount = 0;
    sc.StartChartReplayNew(rparams);
    sc.SetCustomStudyControlBarButtonEnable(4, 0);
  }

  int &ffActiveSlave = sc.GetPersistentInt(5);
  int &ffInSessionSlave = sc.GetPersistentInt(6);
  SCSubgraphRef currentTimeS = sc.Subgraph[0];
  currentTimeS.Name = "Current Time in Seconds";
  int goalTime = 68200; // in seconds

  m.ffActive = sc.GetPersistentIntFromChartStudy(m.ChartNumber, m.StudyId, 1);

  currentTimeS[sc.Index] = sc.BaseDateTimeIn[sc.Index].GetTimeInSeconds();
  int ct = currentTimeS[sc.Index];
  float currentReplaySpeed = sc.GetChartReplaySpeed(sc.ChartNumber);
  if (currentReplaySpeed > 1000) {
    sc.AddMessageToLog("turned off master tracking", 0);
    m.ffActive = false;
  }
  if (m.ffActive) {
    sc.ChangeChartReplaySpeed(sc.ChartNumber, replaySpeed);
    ffActiveSlave = true;
  }

  if (ffActiveSlave && ct >= goalTime && !ffInSessionSlave) {
    sc.ChangeChartReplaySpeed(sc.ChartNumber, 1);
    ffActiveSlave = false;
    ffInSessionSlave = true;
  }

  if (ct < goalTime) {
    ffInSessionSlave = false;
  }
}
