
#include "sierrachart.h"
SCDLLName("LogingTool")

SCSFExport scsf_LocalDb(SCStudyInterfaceRef sc) {

  if (sc.SetDefaults) {
    sc.GraphName = "Log Trades to Local DB";
    sc.AutoLoop = 1;
    sc.Subgraph[0].Name = "LOG to LOCAL";
    // During development set this flag to 1, so the DLL can be
    // rebuilt without restarting Sierra Chart. When development is
    // completed, set it to 0 to improve performance.
    sc.FreeDLL = 0;

    return;
  }

  enum { REQUEST_NOT_SENT = 0, REQUEST_SENT, REQUEST_RECEIVED };
  int &RequestState = sc.GetPersistentInt(13);
  if (sc.GetCustomStudyControlBarButtonEnableState(5)) {
    if (RequestState == REQUEST_NOT_SENT) {
      // Make a request for a text file on the server. When the request is complete and
      // all of the data
      // has been downloaded, this study function will be called with the file placed into
      // the sc.HTTPResponse character string array.

      if (!sc.MakeHTTPRequest("http://127.0.0.1:5555/")) {
        sc.AddMessageToLog("Error making HTTP request.", 1);

        // Indicate that the request was not sent.
        // Therefore, it can be made again when sc.Index equals 0.
        RequestState = REQUEST_NOT_SENT;
        sc.SetCustomStudyControlBarButtonEnable(5, 0);
      } else {
        RequestState = REQUEST_SENT;
      }
    }

    if (RequestState == REQUEST_SENT && sc.HTTPResponse != "") {
      RequestState = REQUEST_RECEIVED;

      // Display the response from the Web server in the Message Log
      sc.AddMessageToLog(sc.HTTPResponse, 1);
      sc.SetCustomStudyControlBarButtonEnable(5, 0);
    } else if (RequestState == REQUEST_SENT && sc.HTTPResponse == "") {
      // The request has not completed, therefore there is nothing to do so we will return
      sc.SetCustomStudyControlBarButtonEnable(5, 0);

      return;
    }

    RequestState = REQUEST_NOT_SENT;
  }

  if (sc.GetCustomStudyControlBarButtonEnableState(6)) {
    if (RequestState == REQUEST_NOT_SENT) {
      // Make a request for a text file on the server. When the request is complete and
      // all of the data
      // has been downloaded, this study function will be called with the file placed into
      // the sc.HTTPResponse character string array.

      if (!sc.MakeHTTPRequest("http://localhost:5555/addTrade?made=it&did=this")) {
        sc.AddMessageToLog("Error making HTTP request.", 1);

        // Indicate that the request was not sent.
        // Therefore, it can be made again when sc.Index equals 0.
        RequestState = REQUEST_NOT_SENT;
        sc.SetCustomStudyControlBarButtonEnable(6, 0);
      } else {
        RequestState = REQUEST_SENT;
      }
    }

    if (RequestState == REQUEST_SENT && sc.HTTPResponse != "") {
      RequestState = REQUEST_RECEIVED;

      // Display the response from the Web server in the Message Log
      sc.AddMessageToLog(sc.HTTPResponse, 1);
      sc.SetCustomStudyControlBarButtonEnable(6, 0);
    } else if (RequestState == REQUEST_SENT && sc.HTTPResponse == "") {
      // The request has not completed, therefore there is nothing to do so we will return
      sc.SetCustomStudyControlBarButtonEnable(6, 0);

      return;
    }

    RequestState = REQUEST_NOT_SENT;
  }
}
