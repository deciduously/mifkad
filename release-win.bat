:: NOTE - must be run within VS2015 x64 Native Tools Command Prompt

:: Suppress command output
ECHO OFF
:: Run production builds  -see package.json for details
yarn prod
:: Create release target
mkdir release
:: Copy executable
copy target\release\mifkad.exe release
:: Copy pre-built frontend and static assets
copy mifkad-assets release
:: Clean up database in case it accidentally existed when releasing
rmdir mifkad-assets\db
:: Copy remaining odds and ends
copy LICENSE release
copy README.md release
copy mifkad-launcher.bat release