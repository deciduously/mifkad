:: Suppress command output
ECHO OFF
:: Launch server
start mifkad.exe | wac.exe
:: Launch client
start chrome http://127.0.0.1:8080
