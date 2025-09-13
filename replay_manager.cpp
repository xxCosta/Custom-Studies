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
        sc.ReceiveKeyboardKeyEvents = 1; 
        sc.SupportKeyboardModifierStates = 1;
        return;
    }

    int keyboardCode = sc.KeyboardKeyEventCode;

    // D key logic: start/pause/resume replay
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
    if (keyboardCode == 69 && !fastForwardActive)
    {
        SCDateTime currentReplayTime = sc.LatestDateTimeForLastBar;

        // Calculate hours until next session (e.g., 20:00)
        int hour = currentReplayTime.GetHour();
        int hoursTillNextSession = 20 - hour;
        if (hoursTillNextSession <= 0)
            hoursTillNextSession += 24;

        SCDateTime nextSessionTime = currentReplayTime + SCDateTime::HOURS(hoursTillNextSession);

        // Calculate difference in seconds
        double secondsToNextSession = (nextSessionTime - currentReplayTime).GetAsDouble() * 24.0 * 60.0 * 60.0;

        // Determine fast speed to reach next session in ~2 seconds
        fastSpeed = (int)(secondsToNextSession / 2.0);
        if (fastSpeed < 100) fastSpeed = 100;

        // Start fast forward
        sc.ResumeChartReplay(chartNum);
        sc.ChangeChartReplaySpeed(chartNum, fastSpeed);

        // Mark as active and store start time
        fastForwardActive = true;
        fastForwardStartTime = std::chrono::steady_clock::now();
        commandRun = false;
    }

    // === AutoLoop section to run 2-second action ===
    if (fastForwardActive)
    {
        auto now = std::chrono::steady_clock::now();
        double elapsedSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - fastForwardStartTime).count() / 1000.0;

        if (elapsedSeconds < 2.0)
        {
            // During the 2 seconds, you can do something repeatedly
            sc.AddMessageToLog("Fast forwarding...", 0);
        }
        else if (!commandRun)
        {
            // After 2 seconds, reset speed to normal
            sc.ChangeChartReplaySpeed(chartNum, 4);
            fastForwardActive = false;
            commandRun = true;
            sc.AddMessageToLog("2 seconds elapsed, replay speed reset.", 0);
        }
    }

    // F key logic: toggle replay speed
    if (keyboardCode == 70)
    {
        speed = (speed == 4000) ? 4 : 4000;
        sc.ChangeChartReplaySpeed(chartNum, speed);
    }
}
