# LibraryLoader for Counter-Strike 1.6
A **LibraryLoader** for Counter-Strike 1.6 using Visual Studio 2017 and written C++.
# Usage
1. Download from [Releases](https://github.com/Dae-Moon/LibraryLoader/releases) or compile using MSVC compiler.
2. Inject dll into **hl.exe** process or rename to .asi and move it to the folder of the game.
3. Use [commands](https://github.com/Dae-Moon/LibraryLoader?tab=readme-ov-file#commands) in console
# VAC (Valve-Anti-Cheat)
Use it at your own risk.
# Requirement
Tested on build 4554, 8684.
Testet on Windows 10.
Linux support will be available later.
# Commands
load_library 'Path\To\Dll' - Loads the module into the address space of the process.
unload_library 'Module name' - Unloads the module from the address space of the process.
log_modules 'Module name' - Displays information about the module.
log_modules - displays the list of modules loaded into the process.
