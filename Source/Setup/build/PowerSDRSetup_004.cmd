start "C:\Program Files\Mozilla Firefox\firefox.exe" "http://sourceforge.net/account/login.php"
start notepad PowerSDRSetup_005.cmd
echo 1. In Firefox Login at Sourceforge
echo 2. Edit version number at XX and Save
echo 3. Press enter to upload
pause
"C:\Program Files\Mozilla Firefox\firefox.exe" "http://sourceforge.net/project/admin/editreleases.php?package_id=197619&release_id=433331&group_id=172695"
call PowerSDRSetup_005.cmd

