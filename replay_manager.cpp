#include "sierrachart.h"
#include <string>
#include <chrono>

SCDLLName("replay-manager")

SCSFExport scsf_ToggleJump(SCStudyInterfaceRef sc)
{
    int chartNum = sc.ChartNumber;
    static int speed = 4;

    static bool fastForwardActive = false;
    static int fastSpeed = 0;
    static std::chrono::steady_clock::time_point fastForwardStartTime;
    static bool commandRun = false;

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
        // i'll need to get the current bar time first
        SCDateTime latestBarTime = sc.LatestDateTimeForLastBar;
        SCString stringConvertedLatestBarTime = sc.DateTimeToString(latestBarTime, FLAG_DT_HOUR); 
        sc.AddMessageToLog(stringConvertedLatestBarTime, 0);
        //next i'll need to find out how much time till the next session
        int currentTimeInHour = latestBarTime.GetHour();
        int goalTime = 20; 
        //c_str makes it so sierra chart can read the string, just using std 
        //will no work because sierra chart expects a const char
        SCString stringConvertedCurrentTimeInHours = std::to_string(currentTimeInHour).c_str();
        int hoursTilNextSesh = (goalTime - currentTimeInHour) + 24;
        int secondsTilNextSesh = hoursTilNextSesh * 3600; 
    

        repeatUntil2SecondsLater = sc.CurrentSystemDateTime.AddSeconds(2);

        //              SETTING UP REPLAY
        sc.AddMessageToLog(sc.TimeToString(sc.CurrentSystemDateTime), 0);
        sc.AddMessageToLog(sc.TimeToString(repeatUntil2SecondsLater), 0);
        
    }
    if(sc.CurrentSystemDateTime <= repeatUntil2SecondsLater){

        sc.AddMessageToLog("hello",0);
    }

    // F key logic: toggle replay speed
    if (keyboardCode == 70)
    {
        speed = (speed == 4000) ? 4 : 4000;
        sc.ChangeChartReplaySpeed(chartNum, speed);
    }
}
