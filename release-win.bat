:: NOTE - this will not build the app.  Please do so separately before running this batchfile

:: Suppress command output
ECHO OFF

:: Create release target
mkdir release
:: Copy executable
copy target\release\mifkad.exe release
:: Copy pre-built frontend and static assets
xcopy /s /i mifkad-assets release\mifkad-assets
:: Clean up database in case it accidentally existed when releasing
rmdir /s /q release\mifkad-assets\db
:: Copy remaining odds and ends
copy LICENSE release\LICENSE
copy README.md release\README.md
copy mifkad-launcher.bat release\mifkad-launcher.bat