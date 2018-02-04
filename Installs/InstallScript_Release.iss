[Files]
Source: ..\Bin\theRushReleaseCrashReport.exe; DestDir: {app}
Source: ..\Bin\crashrpt_lang.ini; DestDir: {app}
Source: ..\Bin\CrashRpt1300.dll; DestDir: {app}
Source: ..\Bin\CrashSender1300.exe; DestDir: {app}
Source: ..\Bin\dbghelp.dll; DestDir: {app}
Source: ..\Bin\libsndfile-1.dll; DestDir: {app}
Source: ..\Bin\msvcp100.dll; DestDir: {app}
Source: ..\Bin\msvcr100.dll; DestDir: {app}
Source: ..\Bin\openal32.dll; DestDir: {app}


[Icons]
Name: {group}\.the rush Alpha; Filename: {app}\theRushReleaseCrashReport.exe; WorkingDir: {app}; IconFilename: {app}\main.exe
Name: {group}\Uninstall .the rush Alpha; Filename: {app}\unins000.exe; WorkingDir: {app}; IconFilename: {app}\main.exe
[UninstallDelete]
Name: Uninstall; Type: filesandordirs
[Setup]
InternalCompressLevel=ultra64
OutputDir=./
AppCopyright=Cedric Guillemet 2007-2012
AppName=the rush - AlphaTest
PrivilegesRequired=none
DefaultDirName={pf}\the rush
AppVerName=0.2 Alpha
DefaultGroupName=the rush
OutputBaseFilename=therush_alphareport_setup
SolidCompression=true
Compression=lzma/ultra64
[Run]

[InstallDelete]

