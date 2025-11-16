#include "sierrachart.h"
#include <string>
#include <chrono>

SCDLLName("replay-manager")

SCSFExport scsf_ToggleJump(SCStudyInterfaceRef sc)
{
    // int chartNum = sc.ChartNumber;
    int chartNum = 2;
    static int speed = 4;
    static bool fastForwardActive = false;
    static int replaySpeedPerSecond = 0;
    static int fastSpeed = 0;

    if (sc.SetDefaults)
    {
        sc.GraphName = "Replay Manager";
        sc.AutoLoop = 0;
        sc.UpdateAlways = 1;
        sc.ReceiveKeyboardKeyEvents = 1; 
        sc.SupportKeyboardModifierStates = 1;
        return;
    }

    int keyboardCode = sc.KeyboardKeyEventCode;

    // D key logic: init/start/pause/resume replay
    if (keyboardCode == 68)
    { 
        int replayStatus = sc.GetReplayStatusFromChart(chartNum);

        if (replayStatus == 0) 
        {
            SCDateTime replayDate;
            replayDate.SetDateTimeYMDHMS(2023, 10, 30, 0, 0, 0);

            n_ACSIL::s_ChartReplayParameters replayParams;
            replayParams.ChartNumber = chartNum;
            replayParams.ReplaySpeed = 4;
            replayParams.StartDateTime = replayDate;

            sc.StartChartReplayNew(replayParams);
            sc.ResumeChartReplay(chartNum);
        }
        else if (replayStatus == 2)
        {
            sc.ResumeChartReplay(chartNum);
        }
        else if (replayStatus == 1)
        {
            sc.PauseChartReplay(chartNum);
        }
    }

    // === E key: fast forward to next session in ~2 seconds ===
     
    static SCDateTime repeatUntil2SecondsLater;
    
    if (keyboardCode == 69) //&& !fastForwardActive)
    {
        SCDateTime latestBarTime = sc.LatestDateTimeForLastBar;
        sc.AddMessageToLog(sc.DateTimeToString(latestBarTime, FLAG_DT_HOUR), 0);
        int currentTimeInHour = latestBarTime.GetHour();
        int goalTime = 20; 
        int hoursTilNextSesh;
        if(currentTimeInHour >= goalTime){
            hoursTilNextSesh = (goalTime - currentTimeInHour) + 24;
        } else {
            hoursTilNextSesh = goalTime - currentTimeInHour;    
        }
        sc.AddMessageToLog(SCString().Format("%d hrs till next session",hoursTilNextSesh),0);
        int secondsTilNextSesh = hoursTilNextSesh * 3600;
        replaySpeedPerSecond = secondsTilNextSesh/2;
        // dont delete the log, it's a good example of a one line log for numbers
        sc.AddMessageToLog(SCString().Format("%d",replaySpeedPerSecond),0);

        repeatUntil2SecondsLater = sc.CurrentSystemDateTime.AddSeconds(2);

    }

    if(sc.CurrentSystemDateTime < repeatUntil2SecondsLater){
        if(fastForwardActive == false){
            fastForwardActive = true;
            sc.ChangeChartReplaySpeed(chartNum, replaySpeedPerSecond); 
            // sc.AddMessageToLog("started fast forward",0);
        };
    }

    if(sc.CurrentSystemDateTime == repeatUntil2SecondsLater){
        if(fastForwardActive == true) {
            fastForwardActive = false;
            sc.ChangeChartReplaySpeed(chartNum, 4); 
            // sc.AddMessageToLog("done fast forawrd",0);
        }
    }

    // F key logic: toggle replay speed
    if (keyboardCode == 70)
    {
        speed = (speed == 4000) ? 4 : 4000;
        sc.ChangeChartReplaySpeed(chartNum, speed);
    }
}
