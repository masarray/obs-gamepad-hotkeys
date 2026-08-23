#include "build.generated.iss"

#ifndef MyAppVersion
  #error MyAppVersion is not defined. Run scripts/build-installer.ps1.
#endif
#ifndef PluginDll
  #error PluginDll is not defined. Run scripts/build-installer.ps1.
#endif
#ifndef PluginDataDir
  #error PluginDataDir is not defined. Run scripts/build-installer.ps1.
#endif
#ifndef InstallerOutputDir
  #error InstallerOutputDir is not defined. Run scripts/build-installer.ps1.
#endif
#ifndef WizardLargeImage
  #error WizardLargeImage is not defined. Run scripts/build-installer.ps1.
#endif
#ifndef WizardSmallImage
  #error WizardSmallImage is not defined. Run scripts/build-installer.ps1.
#endif

#define MyAppName "OBS Gamepad Hotkeys"
#define MyPluginName "obs-gamepad-hotkeys"
#define MyPublisher "Mas Ari / Open Source Contributors"
#define MyProjectUrl "https://github.com/masarray/obs-gamepad-hotkeys"

[Setup]
AppId={{8EDE3BA7-760E-47D1-9C15-1C2C811856DF}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyPublisher}
AppPublisherURL={#MyProjectUrl}
AppSupportURL={#MyProjectUrl}/issues
AppUpdatesURL={#MyProjectUrl}/releases
AppComments=Native gamepad control for OBS Studio — no JoyToKey or keyboard emulation required.
VersionInfoDescription=OBS Gamepad Hotkeys Smart Installer
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}
DefaultDirName={commonappdata}\obs-studio\plugins\{#MyPluginName}
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
WizardStyle=modern dynamic windows11 includetitlebar
WizardResizable=no
WizardSizePercent=120,120
WizardKeepAspectRatio=yes
WizardImageFile={#WizardLargeImage}
WizardImageFileDynamicDark={#WizardLargeImage}
WizardSmallImageFile={#WizardSmallImage}
WizardSmallImageFileDynamicDark={#WizardSmallImage}
Compression=lzma2/ultra64
SolidCompression=yes
OutputDir={#InstallerOutputDir}
OutputBaseFilename=OBS-Gamepad-Hotkeys-Setup-v{#MyAppVersion}
UninstallDisplayName={#MyAppName}
Uninstallable=ShouldCreateUninstaller
UninstallFilesDir={commonappdata}\obs-studio\plugins\{#MyPluginName}\uninstall
CreateUninstallRegKey=ShouldCreateUninstaller
CloseApplications=no
RestartApplications=no
RestartIfNeededByRun=no
SetupLogging=yes
#ifdef EnableInnoSigning
SignTool=obs-sign
SignedUninstaller=yes
#else
SignedUninstaller=no
#endif
DisableWelcomePage=no
DisableReadyPage=no
DisableReadyMemo=no
AllowNoIcons=yes

[Files]
Source: "{#PluginDll}"; DestDir: "{code:GetPluginBinDir}"; DestName: "{#MyPluginName}.dll"; Flags: ignoreversion
Source: "{#PluginDataDir}\*"; DestDir: "{code:GetPluginDataDir}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Run]
Filename: "{code:GetObsExePath}"; Parameters: "{code:GetObsLaunchParameters}"; Description: "Launch OBS Studio and open Tools > Gamepad Hotkeys"; Flags: postinstall nowait skipifsilent runasoriginaluser; Check: CanLaunchObs

[Code]
const
  InstallModeStandard = 0;
  InstallModePortable = 1;

var
  InstallModePage: TInputOptionWizardPage;
  PortableRootPage: TInputDirWizardPage;
  DetectedObsRoot: string;
  PortableRoot: string;
  SelectedInstallMode: Integer;
  AutoDetectedStandard: Boolean;
  CommandLinePortableMode: Boolean;

function IsObsRoot(const Root: string): Boolean;
begin
  Result := FileExists(AddBackslash(Root) + 'bin\64bit\obs64.exe');
end;

function IsPortableObsRoot(const Root: string): Boolean;
begin
  Result := IsObsRoot(Root) and
    (FileExists(AddBackslash(Root) + 'portable_mode.txt') or
     FileExists(AddBackslash(Root) + 'portable_mode'));
end;

function TryStandardObsRoot(const Candidate: string): Boolean;
var
  CleanCandidate: string;
begin
  CleanCandidate := RemoveBackslashUnlessRoot(Candidate);
  Result := IsObsRoot(CleanCandidate) and not IsPortableObsRoot(CleanCandidate);
  if Result then
    DetectedObsRoot := CleanCandidate;
end;

function FindObsFromUninstallKey(const RootKey: Integer; const BaseKey: string): Boolean;
var
  Keys: TArrayOfString;
  I: Integer;
  DisplayName: string;
  InstallLocation: string;
  KeyPath: string;
begin
  Result := False;
  if not RegGetSubkeyNames(RootKey, BaseKey, Keys) then
    Exit;

  for I := 0 to GetArrayLength(Keys) - 1 do begin
    KeyPath := BaseKey + '\\' + Keys[I];
    DisplayName := '';
    if RegQueryStringValue(RootKey, KeyPath, 'DisplayName', DisplayName) then begin
      if Pos('OBS Studio', DisplayName) > 0 then begin
        InstallLocation := '';
        if RegQueryStringValue(RootKey, KeyPath, 'InstallLocation', InstallLocation) then begin
          if TryStandardObsRoot(InstallLocation) then begin
            Result := True;
            Exit;
          end;
        end;
      end;
    end;
  end;
end;

function DetectStandardObs: Boolean;
var
  UninstallKey: string;
begin
  Result := False;

  if TryStandardObsRoot(ExpandConstant('{commonpf}\obs-studio')) then begin
    Result := True;
    Exit;
  end;

  if TryStandardObsRoot(ExpandConstant('{localappdata}\Programs\obs-studio')) then begin
    Result := True;
    Exit;
  end;

  UninstallKey := 'Software\Microsoft\Windows\CurrentVersion\Uninstall';
  if FindObsFromUninstallKey(HKLM64, UninstallKey) or
     FindObsFromUninstallKey(HKLM32, UninstallKey) or
     FindObsFromUninstallKey(HKCU, UninstallKey) then begin
    Result := True;
    Exit;
  end;
end;

function IsObsRunning: Boolean;
var
  ResultCode: Integer;
  PowerShellExe: string;
  Params: string;
begin
  PowerShellExe := ExpandConstant('{sys}\WindowsPowerShell\v1.0\powershell.exe');
  Params := '-NoProfile -NonInteractive -ExecutionPolicy Bypass -WindowStyle Hidden -Command "if (Get-Process obs64 -ErrorAction SilentlyContinue) { exit 10 } else { exit 0 }"';
  Result := Exec(PowerShellExe, Params, '', SW_HIDE, ewWaitUntilTerminated, ResultCode) and (ResultCode = 10);
end;

function NormalizePortableRoot(const InputPath: string): string;
var
  P: string;
begin
  P := RemoveBackslashUnlessRoot(InputPath);

  if FileExists(AddBackslash(P) + 'obs64.exe') then begin
    if CompareText(ExtractFileName(P), '64bit') = 0 then
      P := ExtractFileDir(ExtractFileDir(P));
  end;

  Result := RemoveBackslashUnlessRoot(P);
end;

function GetPortableRoot: string;
begin
  if (PortableRootPage <> nil) and (PortableRootPage.Values[0] <> '') then
    PortableRoot := NormalizePortableRoot(PortableRootPage.Values[0]);
  Result := PortableRoot;
end;

function GetPluginBinDir(Param: string): string;
begin
  if SelectedInstallMode = InstallModePortable then
    Result := AddBackslash(GetPortableRoot) + 'obs-plugins\64bit'
  else
    Result := ExpandConstant('{commonappdata}\obs-studio\plugins\{#MyPluginName}\bin\64bit');
end;

function GetPluginDataDir(Param: string): string;
begin
  if SelectedInstallMode = InstallModePortable then
    Result := AddBackslash(GetPortableRoot) + 'data\obs-plugins\{#MyPluginName}'
  else
    Result := ExpandConstant('{commonappdata}\obs-studio\plugins\{#MyPluginName}\data');
end;

function GetObsExePath(Param: string): string;
begin
  if SelectedInstallMode = InstallModePortable then
    Result := AddBackslash(GetPortableRoot) + 'bin\64bit\obs64.exe'
  else if DetectedObsRoot <> '' then
    Result := AddBackslash(DetectedObsRoot) + 'bin\64bit\obs64.exe'
  else
    Result := '';
end;

function GetObsLaunchParameters(Param: string): string;
begin
  if SelectedInstallMode = InstallModePortable then
    Result := '--portable'
  else
    Result := '';
end;

function CanLaunchObs: Boolean;
begin
  Result := FileExists(GetObsExePath(''));
end;

function ShouldCreateUninstaller: Boolean;
begin
  Result := SelectedInstallMode = InstallModeStandard;
end;

procedure InitializeWizard;
var
  CommandLineRoot: string;
  ModeDescription: string;
begin
  WizardForm.Caption := 'OBS Gamepad Hotkeys Setup';
  WizardForm.WelcomeLabel1.Caption := 'OBS Gamepad Hotkeys';
  WizardForm.WelcomeLabel2.Caption :=
    'Native gamepad control for OBS Studio.' + #13#10 + #13#10 +
    'Map controller buttons directly to recording, scenes, audio and other OBS actions — without JoyToKey or keyboard emulation.' + #13#10 + #13#10 +
    'Setup supports both standard OBS Studio and self-contained OBS Portable installations.';

  SelectedInstallMode := InstallModeStandard;
  PortableRoot := '';
  CommandLinePortableMode := False;

  CommandLineRoot := ExpandConstant('{param:OBSROOT|}');
  if CommandLineRoot <> '' then begin
    PortableRoot := NormalizePortableRoot(CommandLineRoot);
    if IsObsRoot(PortableRoot) then begin
      SelectedInstallMode := InstallModePortable;
      AutoDetectedStandard := False;
      CommandLinePortableMode := True;
      Exit;
    end;
  end;

  AutoDetectedStandard := DetectStandardObs;
  if AutoDetectedStandard then
    ModeDescription := 'A standard OBS Studio installation was detected. Choose it, or select a Portable/custom OBS folder.'
  else
    ModeDescription := 'Choose where OBS Gamepad Hotkeys should be installed.';

  InstallModePage := CreateInputOptionPage(
    wpWelcome,
    'Choose OBS Studio installation',
    'Standard and Portable OBS are supported.',
    ModeDescription,
    True,
    False);

  if AutoDetectedStandard then
    InstallModePage.Add('Standard OBS Studio (detected: ' + DetectedObsRoot + ')')
  else
    InstallModePage.Add('Standard OBS Studio (system-wide plugin)');
  InstallModePage.Add('OBS Studio Portable / custom OBS folder');
  InstallModePage.SelectedValueIndex := InstallModeStandard;

  PortableRootPage := CreateInputDirPage(
    InstallModePage.ID,
    'OBS Studio Portable folder',
    'Select the OBS Studio root folder.',
    'Choose the folder that contains bin, data, and obs-plugins. Setup verifies bin\64bit\obs64.exe before installing.',
    False,
    '');
  PortableRootPage.Add('');
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := False;
  if (PortableRootPage <> nil) and (PageID = PortableRootPage.ID) then
    Result := InstallModePage.SelectedValueIndex <> InstallModePortable;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  Root: string;
begin
  Result := True;

  if (InstallModePage <> nil) and (CurPageID = InstallModePage.ID) then
    SelectedInstallMode := InstallModePage.SelectedValueIndex;

  if (PortableRootPage <> nil) and (CurPageID = PortableRootPage.ID) then begin
    Root := NormalizePortableRoot(PortableRootPage.Values[0]);
    if not IsObsRoot(Root) then begin
      MsgBox(
        'This folder does not look like an OBS Studio installation.' + #13#10 + #13#10 +
        'Select the OBS root folder that contains bin\64bit\obs64.exe.',
        mbError,
        MB_OK);
      Result := False;
      Exit;
    end;
    PortableRoot := Root;
    SelectedInstallMode := InstallModePortable;
  end;
end;

procedure CurPageChanged(CurPageID: Integer);
var
  InstallType: string;
  ObsLocation: string;
  PluginLocation: string;
begin
  if CurPageID = wpReady then begin
    if SelectedInstallMode = InstallModePortable then
      InstallType := 'OBS Studio Portable / custom root'
    else
      InstallType := 'Standard OBS Studio';

    if SelectedInstallMode = InstallModePortable then
      ObsLocation := GetPortableRoot
    else if DetectedObsRoot <> '' then
      ObsLocation := DetectedObsRoot
    else
      ObsLocation := 'System-wide OBS plugin location';

    PluginLocation := GetPluginBinDir('');

    WizardForm.ReadyLabel.Caption :=
      'OBS Gamepad Hotkeys is ready to install. Review the selected OBS target, then click Install.';
    WizardForm.ReadyMemo.Lines.Clear;
    WizardForm.ReadyMemo.Lines.Add('OBS GAMEPAD HOTKEYS  •  v{#MyAppVersion}');
    WizardForm.ReadyMemo.Lines.Add('');
    WizardForm.ReadyMemo.Lines.Add('OBS mode:      ' + InstallType);
    WizardForm.ReadyMemo.Lines.Add('OBS location:  ' + ObsLocation);
    WizardForm.ReadyMemo.Lines.Add('Plugin target: ' + PluginLocation);
    WizardForm.ReadyMemo.Lines.Add('');
    WizardForm.ReadyMemo.Lines.Add('Default controls');
    WizardForm.ReadyMemo.Lines.Add('  B       Pause / Resume Recording');
    WizardForm.ReadyMemo.Lines.Add('  START   Start / Stop Recording');
    WizardForm.ReadyMemo.Lines.Add('');
    WizardForm.ReadyMemo.Lines.Add('Native controller input • No JoyToKey • No keyboard emulation');
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): string;
begin
  Result := '';
  NeedsRestart := False;

  if IsObsRunning then begin
    Result :=
      'OBS Studio is currently open.' + #13#10 + #13#10 +
      'Close OBS Studio so Gamepad Hotkeys can be updated safely, then click Install again.';
    Exit;
  end;
end;
