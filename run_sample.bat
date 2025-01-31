@echo off
:: Prompt for input
echo Enter the sample id tag:
set /p sample_id=

:: Run the Vulkan Samples executable with the input sample id tag
vulkan_samples.exe sample %sample_id%

:: Pause to view output
pause
