@echo off
REM =============================================================
REM File       : lab.cmd
REM Purpose    : Automation of Windows system administration tasks
REM              (logging, time sync, process analysis, cleanup,
REM              archiving, network diagnostics, remote shutdown,
REM              disk space check, systeminfo, task scheduler).
REM Author     : Student Zinchenko Maksym (Lab Work #2, System Programming)
REM =============================================================
setlocal EnableExtensions EnableDelayedExpansion

REM -------------------------------------------------------------
REM If szSmtpServer is left empty, sending is
REM skipped and a warning is written to the log instead.
REM -------------------------------------------------------------
set "szSmtpServer="
set "nSmtpPort=587"
set "szSmtpUser="
set "szSmtpPass="
set "szSmtpFrom="
set "szSmtpTo="

REM -------------------------------------------------------------
REM Argument count check
REM -------------------------------------------------------------
if "%~6"=="" (
  echo Error: 6 arguments are expected.
  echo Usage: lab.cmd ^<log^> ^<dirToArchive^> ^<processName^> ^<archiveDir^> ^<remoteIP^> ^<maxLogKB^>
  exit /b 1
)

REM -------------------------------------------------------------
REM Named argument variables (Hungarian notation)
REM -------------------------------------------------------------
set "szLogFile=%~1"
set "szSourceDir=%~2"
set "szProcessName=%~3"
set "szArchiveDir=%~4"
set "szRemoteIP=%~5"
set "nMaxLogKB=%~6"
set "szScriptDir=%~dp0"
set "szIpListFile=%szScriptDir%ipon.txt"

call :Step01_InitLog
call :Step02_SyncTime
call :Step03_ListProcesses
call :Step04_KillProcess
call :Step05_CleanTempFiles
call :Step06_CompressAndCopy
call :Step07_CheckYesterdayArchive
call :Step08_PurgeOldArchives
call :Step09_CheckInternet
call :Step10_CheckAndShutdownRemote
call :Step11_ListNetworkHosts
call :Step12_CheckIpList
call :Step13_CheckLogSize
call :Step14_CheckDiskSpace
call :Step15_SaveSysteminfo
call :Step16_ShowSchedulerHint

echo Script finished. See "%szLogFile%".
endlocal
exit /b 0


REM =============================================================
REM Step 1: Check / create the log file
REM =============================================================
:Step01_InitLog
  if not exist "%szLogFile%" (
    type nul > "%szLogFile%"
  )
  call :GetTimeStamp dtNow
  (
    echo [!dtNow!] Log file "%szLogFile%" opened or created.
  )>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Step 2: NTP time synchronization (auto-starts w32time if needed)
REM =============================================================
:Step02_SyncTime
  call :GetTimeStamp dtOld

  REM Make sure the Windows Time service is running; w32tm needs it.
  sc query w32time | find "RUNNING" >nul 2>&1
  if !errorlevel! NEQ 0 (
    net start w32time >> "%szLogFile%" 2>&1
  )

  w32tm /resync >> "%szLogFile%" 2>&1
  if !errorlevel! EQU 0 (
    set "szSyncStatus=success"
  ) else (
    set "szSyncStatus=FAILED (needs administrator rights / w32time service)"
  )
  call :GetTimeStamp dtNew
  (
    echo [!dtNew!] Time synchronization: !szSyncStatus!.
    echo [!dtNew!]   Old time: !dtOld!
    echo [!dtNew!]   New time: !dtNew!
  )>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Step 3: List running processes
REM =============================================================
:Step03_ListProcesses
  call :GetTimeStamp dtNow
  (
    echo [!dtNow!] List of running processes ^(tasklist^):
    tasklist
  )>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Step 4: Terminate the process named in Argument3
REM =============================================================
:Step04_KillProcess
  call :GetTimeStamp dtNow
  tasklist /FI "IMAGENAME eq %szProcessName%" 2>nul | find /I "%szProcessName%" >nul
  if !errorlevel! EQU 0 (
    taskkill /IM "%szProcessName%" /F >> "%szLogFile%" 2>&1
    if !errorlevel! EQU 0 (
      echo [!dtNow!] Process %szProcessName% was found, terminated successfully.>> "%szLogFile%"
    ) else (
      echo [!dtNow!] Process %szProcessName% was found, but termination FAILED.>> "%szLogFile%"
    )
  ) else (
    echo [!dtNow!] Process %szProcessName% was not found among running processes.>> "%szLogFile%"
  )
  exit /b 0


REM =============================================================
REM Step 5: Delete *.tmp and temp*.* files in Argument2 directory
REM =============================================================
:Step05_CleanTempFiles
  set /a nDeleted=0
  if exist "%szSourceDir%\" (
    for %%F in ("%szSourceDir%\*.tmp") do (
      del /Q "%%F" >nul 2>&1
      if !errorlevel! EQU 0 set /a nDeleted+=1
    )
    for %%F in ("%szSourceDir%\temp*.*") do (
      if exist "%%F" (
        del /Q "%%F" >nul 2>&1
        if !errorlevel! EQU 0 set /a nDeleted+=1
      )
    )
  )
  call :GetTimeStamp dtNow
  echo [!dtNow!] Temporary files deleted: !nDeleted! ^(from %szSourceDir%^).>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Step 6: Compress Argument2 directory into a ZIP, copy to Argument4
REM =============================================================
:Step06_CompressAndCopy
  call :GetTimeStamp dtNow
  for /f %%T in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMdd-HHmmss"') do set "szStamp=%%T"
  set "szZipName=%szStamp%.zip"

  if not exist "%szArchiveDir%\" mkdir "%szArchiveDir%" >nul 2>&1

  powershell -NoProfile -Command ^
    "Compress-Archive -Path '%szSourceDir%\*' -DestinationPath '%szScriptDir%%szZipName%' -Force" ^
    >> "%szLogFile%" 2>&1

  if exist "%szScriptDir%%szZipName%" (
    copy /Y "%szScriptDir%%szZipName%" "%szArchiveDir%\%szZipName%" >nul 2>&1
    echo [!dtNow!] Archive %szZipName% created and copied to %szArchiveDir%.>> "%szLogFile%"
  ) else (
    echo [!dtNow!] Failed to create archive %szZipName%.>> "%szLogFile%"
  )
  exit /b 0


REM =============================================================
REM Step 7: Check whether yesterday's archive exists
REM =============================================================
:Step07_CheckYesterdayArchive
  call :GetTimeStamp dtNow
  for /f %%D in ('powershell -NoProfile -Command "(Get-Date).AddDays(-1).ToString('yyyyMMdd')"') do set "szYesterday=%%D"

  set "boolFound=0"
  if exist "%szArchiveDir%\" (
    for %%F in ("%szArchiveDir%\%szYesterday%-*.zip") do set "boolFound=1"
  )

  if "!boolFound!"=="1" (
    echo [!dtNow!] Archive for the previous day ^(!szYesterday!^) was found.>> "%szLogFile%"
  ) else (
    echo [!dtNow!] Archive for the previous day ^(!szYesterday!^) is MISSING.>> "%szLogFile%"
    call :SendMail "Lab2: missing archive" "No archive for !szYesterday! found in %szArchiveDir%."
  )
  exit /b 0


REM =============================================================
REM Step 8: Delete archives older than 30 days in Argument4
REM =============================================================
:Step08_PurgeOldArchives
  call :GetTimeStamp dtNow
  set /a nOldFound=0
  if exist "%szArchiveDir%\" (
    for /f %%C in ('forfiles /P "%szArchiveDir%" /M *.zip /D -30 2^>nul ^| find /c /v ""') do set "nOldFound=%%C"
    forfiles /P "%szArchiveDir%" /D -30 /M *.zip /C "cmd /c del @file" >nul 2>&1
  )
  echo [!dtNow!] Old archives found and deleted ^(older than 30 days^): !nOldFound!.>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Step 9: Check Internet connectivity
REM =============================================================
:Step09_CheckInternet
  call :GetTimeStamp dtNow
  ping -n 2 8.8.8.8 >nul 2>&1
  if !errorlevel! EQU 0 (
    echo [!dtNow!] Internet connection: available ^(ping 8.8.8.8 succeeded^).>> "%szLogFile%"
  ) else (
    echo [!dtNow!] Internet connection: NOT available.>> "%szLogFile%"
  )
  exit /b 0


REM =============================================================
REM Step 10: Check host at Argument5 and remote shutdown
REM =============================================================
:Step10_CheckAndShutdownRemote
  call :GetTimeStamp dtNow
  ping -n 2 %szRemoteIP% >nul 2>&1
  if !errorlevel! EQU 0 (
    echo [!dtNow!] Host %szRemoteIP% is reachable on the network.>> "%szLogFile%"
    shutdown /m \\%szRemoteIP% /s /t 0 >> "%szLogFile%" 2>&1
    if !errorlevel! EQU 0 (
      echo [!dtNow!] Shutdown command sent successfully.>> "%szLogFile%"
    ) else (
      echo [!dtNow!] Failed to send shutdown command ^(permissions/firewall/RPC^).>> "%szLogFile%"
    )
  ) else (
    echo [!dtNow!] Host %szRemoteIP% is NOT reachable.>> "%szLogFile%"
  )
  exit /b 0


REM =============================================================
REM Step 11: List computers on the local network
REM =============================================================
:Step11_ListNetworkHosts
  call :GetTimeStamp dtNow
  (
    echo [!dtNow!] List of network hosts ^(net view^):
    net view 2>&1
    echo [!dtNow!] ARP table ^(arp -a^):
    arp -a
  )>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Step 12: Check every IP address listed in ipon.txt
REM =============================================================
:Step12_CheckIpList
  call :GetTimeStamp dtNow
  if not exist "%szIpListFile%" (
    echo [!dtNow!] ipon.txt not found ^(%szIpListFile%^) - step skipped.>> "%szLogFile%"
    exit /b 0
  )
  set "boolAllUp=1"
  for /f "usebackq tokens=* delims=" %%A in ("%szIpListFile%") do (
    if not "%%A"=="" (
      ping -n 1 %%A >nul 2>&1
      if !errorlevel! EQU 0 (
        echo [!dtNow!]   %%A - reachable.>> "%szLogFile%"
      ) else (
        echo [!dtNow!]   %%A - NOT REACHABLE.>> "%szLogFile%"
        set "boolAllUp=0"
      )
    )
  )
  if "!boolAllUp!"=="0" (
    call :SendMail "Lab2: unreachable host" "One or more IPs from ipon.txt are unreachable. See %szLogFile% for details."
  )
  exit /b 0


REM =============================================================
REM Step 13: Check current log file size
REM =============================================================
:Step13_CheckLogSize
  call :GetTimeStamp dtNow
  for /f %%S in ('powershell -NoProfile -Command "[math]::Round((Get-Item '%szLogFile%').Length/1KB)"') do set "nLogSizeKB=%%S"
  echo [!dtNow!] Current log file size: !nLogSizeKB! KB ^(limit %nMaxLogKB% KB^).>> "%szLogFile%"
  if !nLogSizeKB! GTR %nMaxLogKB% (
    echo [!dtNow!] Log file size exceeded the limit!>> "%szLogFile%"
    call :SendMail "Lab2: log size exceeded" "Size !nLogSizeKB! KB exceeded the limit of %nMaxLogKB% KB."
  )
  exit /b 0


REM =============================================================
REM Step 14: Free / used space on all disks
REM =============================================================
:Step14_CheckDiskSpace
  call :GetTimeStamp dtNow
  (
    echo [!dtNow!] Disk space status:
    powershell -NoProfile -Command ^
      "Get-PSDrive -PSProvider FileSystem | Select-Object Name,@{N='UsedGB';E={[math]::Round($_.Used/1GB,2)}},@{N='FreeGB';E={[math]::Round($_.Free/1GB,2)}} | Format-Table -AutoSize"
  )>> "%szLogFile%" 2>&1
  exit /b 0


REM =============================================================
REM Step 15: Save systeminfo output to a file
REM =============================================================
:Step15_SaveSysteminfo
  call :GetTimeStamp dtNow
  for /f %%T in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMdd-HHmmss"') do set "szSysStamp=%%T"
  set "szSysFile=%szScriptDir%systeminfo-%szSysStamp%.txt"
  systeminfo > "%szSysFile%" 2>&1
  echo [!dtNow!] systeminfo output saved to %szSysFile%.>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Step 16: Task Scheduler hint for recurring auto-start
REM =============================================================
:Step16_ShowSchedulerHint
  call :GetTimeStamp dtNow
  (
    echo [!dtNow!] For recurring auto-start, use Task Scheduler, e.g.:
    echo [!dtNow!]   schtasks /create /tn "LR2_Script" /tr "\"%szScriptDir%lab.cmd\" lab.log d:\baza xx.exe d:\archive 192.168.1.2 500" /sc daily /st 09:00 /rl highest
  )>> "%szLogFile%"
  exit /b 0


REM =============================================================
REM Helper: send an email via Send-MailMessage
REM Params: %1 - subject, %2 - body
REM =============================================================
:SendMail
  if "%szSmtpServer%"=="" (
    call :GetTimeStamp dtNow
    echo [!dtNow!] SMTP is not configured - email "%~1" NOT sent ^(see settings block at the top of the script^).>> "%szLogFile%"
    exit /b 0
  )
  powershell -NoProfile -Command ^
    "$pass = ConvertTo-SecureString '%szSmtpPass%' -AsPlainText -Force; ^
     $cred = New-Object System.Management.Automation.PSCredential('%szSmtpUser%', $pass); ^
     Send-MailMessage -SmtpServer '%szSmtpServer%' -Port %nSmtpPort% -UseSsl -Credential $cred -From '%szSmtpFrom%' -To '%szSmtpTo%' -Subject '%~1' -Body '%~2'" ^
    >> "%szLogFile%" 2>&1
  exit /b 0


REM =============================================================
REM Helper: returns the current date/time as a string
REM Param: %1 - variable name to store the result in
REM =============================================================
:GetTimeStamp
  for /f %%T in ('powershell -NoProfile -Command "Get-Date -Format \"yyyy-MM-dd HH:mm:ss\""') do set "%~1=%%T"
  exit /b 0

REM =============================================================
REM End of file lab.cmd
REM =============================================================
