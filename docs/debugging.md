[Back to main README](../README.md)

## Testing and Debugging

### Local (Linux)

Test on your development machine before deploying to R36S:

```bash
# Inside Docker container
make PLATFORM=linux CONFIG=debug
```

### Local Web

Build and test in a browser:

```bash
# Inside Docker container with port 8080 exposed
make PLATFORM=web CONFIG=release

# Open browser: http://localhost:8080/gpp_web.html
```

### Debugging on R36S

#### Method 1: Log Files

```bash
# Inside Docker container
make PLATFORM=lir36s CONFIG=debug
```

#### Method 2: SSH + GDB (Advanced)

For advanced debugging, you can use GDB over SSH:

```bash
# On R36S (via SSH)
gdb ./gpp_r36s

# Basic GDB commands
(gdb) run              # Start program
(gdb) break main       # Set breakpoint
(gdb) continue         # Continue execution
(gdb) print variable   # Print variable value
(gdb) quit             # Exit GDB
```

### Common Issues

**Game crashes immediately**:

- Check GLIBC compatibility (build must use Docker)
- Verify all assets are present on device
- Check file permissions: `chmod +x gpp_r36s`

**Poor performance**:

- Reduce texture sizes
- Lower frame rate target
- Optimise game logic loops
- Profile with `perf` on Linux build first

**No display output**:

- Ensure SDL_VIDEODRIVER=kmsdrm is set
- Check that no other app is using the display
- Verify DRM/GLES2 support in your build

## Deploying to R36S

### Method 1: Direct SD Card Access (Recommended)

1. Power off R36S and remove SD card
2. Insert SD card into your computer
3. Mount the `EASYROMS` partition (FAT32, readable on all OSes)
4. Create a folder: `EASYROMS/roms/ports/` (if not exists)
5. Copy `build/r36s_release/gpp_r36s` to this folder
6. Copy any game assets to the same folder
7. Safely eject SD card
8. Insert back into R36S and power on

### Method 2: SCP over WiFi

If WiFi is configured (see [WiFi Configuration](#wifi-configuration)):

```bash
# From your development machine
scp build/r36s_release/gpp_r36s ark@192.168.x.x:/home/ark/
```

### Method 3: File Browser over Network

1. Install a file manager on your PC that supports SFTP
2. Connect to R36S: `sftp://ark@192.168.x.x`
3. Browse to `/home/ark/` or `/roms/ports/`
4. Drag and drop your `gpp_r36s` binary

### Running on R36S

#### From Terminal

```bash
# SSH into R36S or use USB keyboard
cd /home/ark
./gpp_r36s
```

#### From EmulationStation

1. Place binary in `/roms/ports/`
2. Create a launch script `gpp_r36s.sh`:

```bash
#!/bin/bash
cd /roms/ports
./gpp_r36s
```

3. Make executable: `chmod +x gpp_r36s.sh`
4. Restart EmulationStation
5. Navigate to "Ports" section

#### Autostart (See Advanced Topics)

For games that should run automatically at boot, see [R36S Autostart Configuration](./advanced.md#r36s-autostart-configuration).


[Back to main README](../README.md)
