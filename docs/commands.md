[Back to main README](../README.md)

### Useful Linux Commands <a name="useful-linux-commands"></a>

#### Accessing the R36S Terminal Using a Keyboard

>**Security Notice**  
The R36S ships with default credentials username `ark` password `ark`. If connecting R36S to WiFi, **strongly recommended** changing password and reflashing ArkOS (dont use OS on included SD Card).

##### Requirements

- USB keyboard  
- USB-A to USB-C adapter **or** a USB-C keyboard

##### Accessing R36S Terminal Steps

| Step / Command | Description | Notes |
|---------------|-------------|-------|
| Press `Start` | Open the main menu | From the main menu, select `Quit` |
| Press `A` | Exit EmulationStation | Confirm by selecting `Quit` |
| Flashing `|` cursor | Indicates TTY input mode | Device is ready to receive keyboard input |
| `Ctrl + Shift + F2` | Switch to TTY terminal | Keyboard must be connected to the R36S |
| Username | `ark` | Default system username |
| Password | `ark` | Default password (change recommended) |
| `passwd` | Change user password | Follow prompts to set a new password |
| `Ctrl + Shift + F1` | Return to graphical interface | Key combination depending on ArkOS version |


##### Post-Login Commands

```bash
# To change default password
passwd

# Verify current user
whoami

# Display system information
uname -a
```  

#### Linux Terminal Commands

| Command | Description | Example |
|---------|-------------|---------|
| `cd <dir>` | Change directory | `cd /home/ark` |
| `ls` | List files | `ls` `ls -l` `ls -lh` (detailed with sizes) |
| `ls -la` | List all files including hidden | `ls -la` |
| `pwd` | Print working directory | `pwd` |
| `cp src dest` | Copy file | `cp game.c game_backup.c` |
| `cp -r src dest` | Copy directory recursively | `cp -r assets/ backup/` |
| `mv src dest` | Move/rename file | `mv main.c main.c.bk` `mv main.c ./archive` |
| `rm file` | Remove file | `rm temp.log` |
| `rm -rf dir` | Remove directory recursively | `rm -rf build/` |
| `mkdir dir` | Create directory | `mkdir assets` |
| `chmod +x file` | Make file executable | `chmod +x gpp_r36s` |
| `./file` | Execute file | `./gpp_r36s` |
| `cat file` | Display file contents | `cat autostart.log` |
| `tail -f file` | Follow file updates | `tail -f game.log` |
| `grep "text" file` | Search in file | `grep "ERROR" game.log` |
| `find dir -name` | Find files | `find . -name "*.c"` |
| `df -h` | Show disk space | `df -h` |
| `du -sh dir` | Show directory size | `du -sh build/` |
| `ps aux` | List processes | `ps aux \| grep gpp_r36s` |
| `kill PID` | Kill process | `kill 1234` |
| `sudo command` | Run as administrator | `sudo systemctl start ssh` `sudo reboot` |
| `exit` | Exit shell/container | `exit` |
| `lsblk` | List block devices (find SD card device) | `lsblk` (before/after inserting card) |
| `umount` | Unmount a device or partition | `sudo umount /dev/sdh*` (unmount SD card) |
| `fdisk -l` | Show partition table of drives | `sudo fdisk -l /dev/sdh` (check SD card) |
| `wipefs -a` | Wipe filesystem signatures (reset card) | `sudo wipefs -a /dev/sdh` (reset SD card) |
| `badblocks -v` | Check SD card for errors | `sudo badblocks -v /dev/sdh` (test card health, the supplied SD cards can fail) |


#### Docker-Commands

| Command | Description |
|---------|-------------|
| `docker ps` | List running containers |
| `docker ps -a` | List all containers |
| `docker images` | List images |
| `docker rm <id>` | Remove container |
| `docker rmi <image>` | Remove image |
| `docker system prune` | Clean up unused resources |

#### File Transfer over SSH

| Command | Description |
|---------|-------------|
| `scp file user@host:path` | Copy file to remote |
| `scp user@host:path local` | Copy file from remote |
| `scp -r dir user@host:path` | Copy directory to remote |
| `rsync -avz src dest` | Sync directories |


[Back to main README](../README.md)