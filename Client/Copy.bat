@echo off
taskkill /F /IM Client.exe >nul 2>&1
taskkill /F /IM Client_Debug.exe >nul 2>&1
xcopy .\..\Engine\Binary\*.dll .\..\Binary\ /D /Y /s
