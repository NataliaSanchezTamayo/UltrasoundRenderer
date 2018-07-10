MODIFIED TO SAVE BITMAP IN DESKTOP.



This sample demonstrates how to use scan-converter plug-in and access
each B mode ultrasound output frame in real-time during scanning.
For demonstration purposes ultrasound image is displayed not using Usgfw2 SDK
renderer, but custom output method. Usgfw2 SDK renderer output is redirected to
hidden window.

Used objects and interfaces:
Usgfw2Class, IProbe, IUsgDataView, IUsgMixerControl, IUsgGain, IUsgScanConverterPlugin.

!!! IMPORTANT !!!
* In order to modify this sample you will need knowledge of COM and DirecShow.
* For sample compilation you will need to install Microsoft Platform SDK
  (Windows SDK), compile DirectShow base classes.
* Also you will need to adjust project's include and library paths related to
  DirectShow and USGFWSDK.
* USGFWSDK include files define ultrasound-related structures and interfaces.
* For operation must be installed Microsoft Visual C++ 2017 redistributable:
  https://www.visualstudio.com/downloads/
  https://go.microsoft.com/fwlink/?LinkId=746572

Additional include directories in plug-in's project settings:
.\;"$(USGFWSDK)\include";"$(DXSDK_INCLUDE)";"$(PSDK_INCLUDE)"; "$(DS_BASE_INCLUDE)";

Additional library directories in plug-in's project settings:
"$(DXSDK_LIB)\x86";"$(PSDK_LIB)"; "$(DSHOWBASECLASSES)\VC80\Release";

Additional dependencies in plug-in's project settings:
strmbase.lib vfw32.lib quartz.lib winmm.lib dxtrans.lib comctl32.lib odbc32.lib odbccp32.lib comsuppw.lib psapi.lib user32.lib Setupapi.lib

Here 
USGFWSDK - Ultrasonography for Windows SDK;
DXSDK - DirectX SDK;
PSDK - Platform SDK (Windows SDK);
DS_BASE, DSHOWBASE - DirectShow base classes from Platform SDK;
strmbase.lib - compiled DirectShow base classes;

