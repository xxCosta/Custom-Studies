#include "sierrachart.h"
#include <vector>




SCDLLName("replay-manager")

SCSFExport scsf_ToggleJump(SCStudyInterfaceRef sc)
{
    static std::vector<int> chartNums = {2,3,5,6,8}; 
    // int chartNum2 = sc.ChartNumber;
    int chartNum2 = 2;
    int chartNum8 = 8;
    static int speed = 4;
    static bool fastForwardActive = false;
    static int replaySpeedPerSecond;
    static int fastSpeed = 0;

    if (sc.SetDefaults)
    {
        sc.GraphName = "Replay Manager";
        sc.AutoLoop = 1;
        sc.UpdateAlways = 1;
        sc.ReceiveKeyboardKeyEvents = 1; 
        sc.SupportKeyboardModifierStates = 1;
        return;
    }

    int keyboardCode = sc.KeyboardKeyEventCode;

    // D key logic: init/start/pause/resume replay
    if (keyboardCode == 68)
    {
        for(int chartNum : chartNums){
            int replayStatus = sc.GetReplayStatusFromChart(chartNum);

            if (replayStatus == 0) 
            {
                SCDateTime replayDate;
                // reset when starting a new backtest
                // replayDate.SetDateTimeYMDHMS(2024, 01, 15, 0, 0, 0);
                
                replayDate.SetDateTimeYMDHMS(2024, 02, 11, 0, 0, 0);

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
    }

    // === E key: fast forward to next session in ~2 seconds ===
     
    static SCDateTime repeatUntil2SecondsLater;
    static int futureBarIndex;
    static int lastVisibleBarIndex;
    if (keyboardCode == 69) //&& !fastForwardActive)
    {
        SCDateTime latestBarTime = sc.LatestDateTimeForLastBar;
        // sc.AddMessageToLog(sc.DateTimeToString(latestBarTime, FLAG_DT_HOUR), 0);
        int currentTimeInHour = latestBarTime.GetHour();
        int goalTime = 20; 
        int hoursTilNextSesh;
        if(currentTimeInHour >= goalTime){
            hoursTilNextSesh = (goalTime - currentTimeInHour) + 24;
        } else {
            hoursTilNextSesh = goalTime - currentTimeInHour;    
        }
        //sc.AddMessageToLog(SCString().Format("%d hrs till next session",hoursTilNextSesh),0);
        int secondsTilNextSesh = hoursTilNextSesh * 3600;
        replaySpeedPerSecond = secondsTilNextSesh/2;
        // dont delete the log, it's a good example of a one line log for numbers
        //sc.AddMessageToLog(SCString().Format("%d",replaySpeedPerSecond),0);
        
        lastVisibleBarIndex = sc.IndexOfLastVisibleBar;
        sc.AddMessageToLog(SCString().Format("%d",lastVisibleBarIndex),0);

        futureBarIndex = lastVisibleBarIndex + hoursTilNextSesh;
        sc.AddMessageToLog(SCString().Format("%d", futureBarIndex),0);
        fastForwardActive = true;
        for(int chartNum : chartNums){
            sc.ChangeChartReplaySpeed(chartNum, replaySpeedPerSecond); 
        }
    }


    if(fastForwardActive) {
        if(sc.IndexOfLastVisibleBar >= futureBarIndex){
            for(int chartNum : chartNums){
                sc.ChangeChartReplaySpeed(chartNum, 4); 
            }
        // sc.AddMessageToLog("done fast forawrd",0);
        fastForwardActive = false;
        }
    }

    // F key logic: toggle replay speed
    if (keyboardCode == 70)
    {
        speed = (speed == 4000) ? 4 : 4000;
        for(int chartNum : chartNums){

            sc.ChangeChartReplaySpeed(chartNum, speed);
        }
    }
}
