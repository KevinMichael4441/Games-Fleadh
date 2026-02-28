## v0.26: 27-02-2026
- Added FastMath and compilier flags
- Code Refactor
- Added Scene Manager
- Added Simulations
- Added Performance Manager
- Stops running game binary for SSH deploy
- Added Game Systems

## v0.25: 24-02-2026
- Docker compose added per platform
- Permissions fix on docker container
- Added prune of docker container
- Added specific compose for Headless, Nvidia, AMD and iGPU
- Added Memory Manager
- Added Fragment Shaders (performance needs review)
- Added Achievements System
- Code refactor

## v0.24: 16-02-2026
- Debug levels added
- Debug drawing for colliders and Spine Entities
- Sound for collectable

## v0.23: 16-02-2026
- cute_c2 tiny headers added to container
- Collision sample added
- Points update for Achievements

## v0.22: 16-02-2026
- Spine animations orientation fixed
- Animation trottling
- Exiting Spine Manager
- Flexible Spine atlas and json locations

## v0.21: 15-02-2026
- Spine Manager Added
- Web Template improved
- Default config created if does not exist
- Prep for Achivements System and Shaders
- Core libs created

## v0.20: 13-02-2026
- Sound Manager Added
_ Lazy Loading
- Linux Install Script
- Headless QEMU Test
- vscode volume removal improved
- config.mk git ignore
- GDB Debug Resuced to one Task

## v0.18: 29-01-2026
- Added Telemetry (toggle Telemetry)
- CPU Usage
- GPU Usage
- Battery Levels
- Usage of port 8080 cleanup
- Docker Cache issue fix
- VSC Docker rebuild issue note
- Fix to Start Button Control
- Main now a typical GameLoop
- Fixes to Emsripten Install
- SD Card Config Update

## v0.17: 23-01-2026
- Visual Studio Code Debugging enabled in Docker Container
- IP Address Displayed on Screen (R36S | Linux | Windows)
- Windows install WSL recommended and standard install and instructions updated
- IP Address lookup in Shell
- GPU config improved
- Fix for GPU less server
- Compose files changed for VSC debug
- WSL Compose file added

## v0.16: 18-01-2026
- Improved input readme

## v0.15: 18-01-2026
- Volumn Keys on Keyboard Added
- Improved input readme

## v0.14: 18-01-2026
- Fixed keyboard mappings, key actions
- Improved input readme
- Action special 3 and 4 added for keyboard

## v0.13: 18-01-2026
- Improved input readme

## v0.12: 18-01-2026
- Improved input readme
- Improved message structure

## v0.11: 18-01-2026
- Input mappings fixed (R36S -> Xbox)
- UI overlay added and inputs highlighted
- SD Card Config script improved

## v0.10: 16-01-2026
- Improved offline Config SD Card script
- Tidy up of input messages

## v0.9: 16-01-2026
- Improved Input Messages in Sample

## v0.8: 16-01-2026
- Cross platform input manager added with support for R36S, Linux, Windows and Web
- Input manager readme added
- Improvements to SD Card Config (ownership of autostart directory)

## v0.7: 15-01-2026
- Added support for **raylib 5.5**

## v0.6: 15-01-2026
- Added Visual Studio Code **Dev Container** support so VS Code runs inside the Docker container
- Container-installed headers and libraries (e.g. **raylib**) are now visible in IntelliSense
- Eliminates include errors and squiggles caused by host/container filesystem separation
- Ensures cross-machine consistency without duplicating container libraries on host
- Added new make target rebuild_component_db (run only when new 'c' files are added)

## v0.5: 11-01-2026
- Added build processes for Web, Linux and R36S

## v0.4: 11-01-2026
- Updated SD Card Setup Script
- Updated Readme
- Updated instructions regarding SSH Key Transfer

## v0.3: 08-01-2026
- Added notes about repo now using Docker Compose to build Projects

## v0.2: 06-01-2026
- Fixed libdrm issues where drm.h not being discovered
- Added multiple platforms compose files
- Added new build targets for convenience
- Improved SSH
- Added config SD card script in case of issues accessing via TTY
- Added Ping test to check R36S connectivity

## v0.1: 20-12-2025
- Initial release
