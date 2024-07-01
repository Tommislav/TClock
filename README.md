# Super simple win32 floating window displaying current time

![](/screenshot.jpg?raw=true "Optional Title")

Keep an eye at the time at all times, if you (like me) hide your taskbar and then get into flow and lose track of time...

* Windows only (duh)
* One single code file
* Right click on window (or press escape) to close

Threw in the visual studio files too (I'm using Visual Studio 2019 Community), get rid of them if you want. It's a single code file. Just compile it.
* The only libs you need to link against (Linker -> Input -> Additional Dependencies): `kernel32.lib;user32.lib;gdi32.lib;`
* And under Linker -> System -> SubSystem, be sure to select `Windows(/SUBSYSTEM:WINDOWS)` - or it will complain about the entrypoint.

KTHXBYE
