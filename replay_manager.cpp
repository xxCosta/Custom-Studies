// The top of every source code file must include this line
#include "sierrachart.h"
#include <string>
// For reference, refer to this page:
// https://www.sierrachart.com/index.php?page=doc/AdvancedCustomStudyInterfaceAndLanguage.php

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies. 
SCDLLName("replay-manager")

//This is the basic framework of a study function. Change the name 'TemplateFunction' to what you require.
SCSFExport scsf_ToggleJump(SCStudyInterfaceRef sc)
{
    SCString msg;
    SCDateTime replayDate;
    int chartNum = sc.ChartNumber;
    static int speed = 4;
	
    if (sc.SetDefaults)
	{
		sc.GraphName = "replay manager";		
		sc.AutoLoop = 0;  //Automatic looping is enabled. 
        sc.ReceiveKeyboardKeyEvents = 1; // calls this study function everytime a key is pressed
        sc.SupportKeyboardModifierStates = 1; 
 return;
	}
    
    int keyboardCode= sc.KeyboardKeyEventCode ;
    // msg.Format("%d", keyboardCode);
    // sc.AddMessageToLog(msg, 1);
    

    if (keyboardCode == 68 && sc.IsKeyPressed_Shift == 1){
        sc.AddMessageToLog("hello",1);
    }

    if (keyboardCode == 68){ 
        //	CLICK THE D KEY TO 
        //	INITIALIZE or START/STOP TOGGLE
        int replayStatus = sc.GetReplayStatusFromChart(chartNum);
        if (replayStatus == 0){ 
	        replayDate.SetDateTimeYMDHMS(2023,10,30,0,0,0);
	        sc.StartChartReplay(chartNum, 4, replayDate);
	        sc.ResumeChartReplay(chartNum); 
        }
        if (replayStatus == 2){
            sc.ResumeChartReplay(chartNum);
        }
        if (replayStatus == 1){
            sc.PauseChartReplay(chartNum);
        }
    }

    if (keyboardCode == 69){
        //  CLICK THE E KEY TO
        //  FAST FORWARD TO NEXT SESSION
        SCDateTime currentReplayTime = sc.LatestDateTimeForLastBar;
        int hour = currentReplayTime.GetHour();
        int hoursTillNextSession = 20 - hour;
        currentReplayTime += SCDateTime::HOURS(hoursTillNextSession);

        //STOPED HERE. NEXT STEP IS TO MAKE A TIMER REFER TO CHATGPT IF YOU FORGET
        
        sc.StartChartReplay(chartNum,3,currentReplayTime);
        SCString debug;                  
        debug.Format("%d",hour);
        sc.AddMessageToLog(debug,1);
    }

    if (keyboardCode == 70) {
        //  CLICK THE F KEY TO
        //  TOGGLE FAST FORWARD
        if (speed == 4000){
            speed = 4;
        }else{
            speed = 4000;
        }
        sc.ChangeChartReplaySpeed(chartNum,speed);
    }
        //SCString debug;                  
        //debug.Format("%d",replayDate);
        //sc.AddMessageToLog(debug,1);
}


SCSFExport scsf_KeyboardCodeFinder(SCStudyInterfaceRef sc)
{
	if (sc.SetDefaults)
	{
		sc.GraphName = "Keyboard Code Finder";
        sc.ReceiveKeyboardKeyEvents = 1; // calls this study function everytime a key is pressed
		sc.AutoLoop = 0;  //Automatic looping is enabled. 
		
		sc.Subgraph[0].Name = "Keyboard Code Finder";	
		return;
	}
	
    if (sc.KeyboardKeyEventCode != 0 ){ // != 0 to prevent constant logging
        SCString msg;
        int keyboardCode = sc.KeyboardKeyEventCode;
        msg.Format("%d", keyboardCode);
        sc.AddMessageToLog(msg, 1);

        sc.KeyboardKeyEventCode = 0;
    } 
	
}
