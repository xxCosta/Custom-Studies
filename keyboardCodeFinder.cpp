// The top of every source code file must include this line
#include "sierrachart.h"
#include <string>
// For reference, refer to this page:
// https://www.sierrachart.com/index.php?page=doc/AdvancedCustomStudyInterfaceAndLanguage.php

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies. 
SCDLLName("KeyboardCodeFinder")

//This is the basic framework of a study function. Change the name 'TemplateFunction' to what you require.
SCSFExport scsf_KeyboardCodeFinder(SCStudyInterfaceRef sc)
{
	// Section 1 - Set the configuration variables and defaults
	if (sc.SetDefaults)
	{
		sc.GraphName = "Keyboard Code Finder";
        sc.ReceiveKeyboardKeyEvents = 1; // calls this study function everytime a key is pressed
		sc.AutoLoop = 0;  //Automatic looping is enabled. 
		
		sc.Subgraph[0].Name = "Keyboard Code Finder";	
		return;
	}
	
	
	// Section 2 - Do data processing here
    if (sc.KeyboardKeyEventCode != 0 ){ // != 0 to prevent constant logging
        SCString msg;
        int keyboardCode = sc.KeyboardKeyEventCode;
        msg.Format("%d", keyboardCode);
        sc.AddMessageToLog(msg, 1);

        sc.KeyboardKeyEventCode = 0;
    } 
	
}
