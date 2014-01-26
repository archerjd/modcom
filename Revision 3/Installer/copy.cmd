set oldpath=D:\modularcombat\Installer\modularcombat_full
set newpath=D:\modularcombat\Installer\update

for /f "tokens=*" %%a in (change_list.txt) do (
	if not exist %newpath%%%~panul mkdir %newpath%%%~pa
    xcopy "%oldpath%\%%a" "%newpath%%%~pa" /S /I
)
mkdir updated
xcopy %newpath% /S /I /C