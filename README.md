# MIPS43DS
Simple port of a MIPS32 simulator for Nintendo 3DS.
This was originally written in C for Windows and Linux as a university project but has now been ported to 3DS as a homebrew test. Note that not all instructions will work, and some are still broken. Uses the libctru library by smealum.

# Usage
Place MIPS binary files in the same directory as mips43ds.3dsx. Run the program with a 3DS exploit (e.g. Homebrew Launcher) and select the binary file to run. The program does not support any I/O and is limited to displaying the register values of the CPU.
