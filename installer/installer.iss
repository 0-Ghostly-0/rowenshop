; Rowen Fragment — Windows installer (Inno Setup 6).
; The AppId GUID must NEVER change between versions (it is distinct from the
; Vocal Polish GUID so the two products install and uninstall independently).

#ifndef AppVersion
  #define AppVersion "0.5.0"
#endif

#define AppName "Rowen Fragment"
#define Publisher "Rowen"
#define URL "https://rowen.work"

[Setup]
AppId={{3F9D2B6C-8A41-4E5F-B27D-91C4E7A0D512}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=no
PrivilegesRequired=admin
OutputBaseFilename=Rowen Fragment Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}

[Files]
Source: "..\build\RowenFragment_artefacts\Release\VST3\Rowen Fragment.vst3\*"; \
    DestDir: "{commoncf64}\VST3\Rowen Fragment.vst3"; \
    Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
SetupWindowTitle=%1 Setup
WelcomeLabel1=Welcome to the {#AppName} installer
WelcomeLabel2=This will install {#AppName} on your computer.%n%nAfter installing, open your music software and rescan plugins - {#AppName} will appear as a VST3 effect.%n%nClose your music software (DAW) before continuing.
