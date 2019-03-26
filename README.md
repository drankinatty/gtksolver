# gtksolver
**GtkSolver** - Linear System of Equations Solver written in C and Gtk+2.

GtkSolver is a simple but fast and capable linear system solver that is simply a GtkTextView wrapped around a command-line solver to allow simple copy/paste of the coefficent matrix (including the constant vector as the last column) into the textview editor for solving. Basically it is an editor with a `[Solve...]` button.  A convenience that saves creating the file with the coefficent matrix and constant vector before calling the solver from the command line. The underlying parser and solver is written entirely in C. (the `mtrx_t.[ch]` source files contain code for handling direct file input as well)

The linear system solver uses Gauss-Jordan Elimination with full pivoting to solve a system of equations containing any number of unknowns up to the physical memory limits of your computer. See [Gaussian Elimination](https://en.wikipedia.org/wiki/Gaussian_elimination)

### Solver Use

The interface is straight forward. The program lauches with a short help message and example of the input format expected in the textview itself. Simply paste your coefficent matrix with the constant vector as the final column replacing the help message and click `[Solve...]`. Or if you need to middle-mouse paste from your select-buffer, the `[Clear]` button allows you to clear the textview and without disturbing the contents of your select buffer.

The contents of the textview (i.e. the GtkTextBuffer) is read and passed to the solver. The input format is flexible, but must be an `[N x N+1]` matrix (representing `N` equations and `N` unknowns **PLUS** the constant vector as the last column). Delimiters between the values are ignored. All values are processed as type `double`. (as defined in `mtrx_t.h`)

The contents of the textview before pressing `[Solve...]` is flexible and could equally be:

     3.0  2.0  -4.0   3.0
     2.0  3.0   3.0  15.0
     5.0 -3.0   1.0  14.0

or


    linear system of equations:

        3    2   -4  |   3
        2    3    3  |  15
        5   -3    1  |  14

or

    3,2,-4,3
    2,3,3,15
    5,-3,1,14

(all leading characters that are not `".+-[0-9]"` are discarded, so the line and leading whitespace in the second example above `"linear system of equations:"` is ignored)

Clicking `[Clear]` clears the GtkTextBuffer, and clicking `[Help]` clears the buffer and redisplays the initial help message.

### Output

Taking the contents of the second example above as the contents of the textview and clicking `[Solve...]` results in:

    linear system of equations:

        3    2   -4  |   3
        2    3    3  |  15
        5   -3    1  |  14


    Solution Vector:

     x[  0] :   3.0000000
     x[  1] :   1.0000000
     x[  2] :   2.0000000

(where the formatted solution vector is simply written back to the same GtkTextBuffer and displayed in the textview below the coefficent matrix)

### Compiling

For Linux, all that is needed is `gcc/make/pkg-config` and `Gtk+2`. (note: some distributions package the headers and development files in separate packages, for instance `Gtk+2-dev`). You may want to create an out-of-source directory for building to prevent cluttering your sources with the object and executable files. Simply create a separate directory (e.g. `gtksolver.build`) and then symlink the `Makefile`, `src` and `include` directories within your build directory. All that is needed then is to change to the build directory and type:

     $ make

For building on Windows, see the notes on obtaining the precompiled Gtk libraries and header files in the [GtkWrite Readme.md](https://github.com/drankinatty/gtkwrite). You will also need MinGW installed.

Building with **MinGW** on windows:

    $ make os=windows

(the `os=windows` simply appends `.exe` to the executable name so that `strip` finds the executable correctly.)


Building with **TDM-MinGW** on windows.

TDM-MinGW is often installed as part of the Code::Blocks IDE. The only addition for using TDM-MinGW is to pass the `CC=mingw32-gcc` make-variable on the command line and invoke make with `mingw32-make` to accommodate naming difference in TDM-MinGW, e.g.

    $ mingw32-make CC=mingw32-gcc os=windows

### Development Status

As mentioned at the beginning, this project is basically a quick GtkTextView wrapper around a linear system solver. The Gtk+2 interface and forwarding of the textview contents to the solver has minimal validations. This basically grew out of finding a convenient way to help students with physics and engineering problems. The acceptable input format is flexible enough to allow pasting of a `.csv` (*comma separated values* file) into the editor window or any other delimited set of numbers that make up the coefficent matrix for a system of equations. The underlying solver and parsing code is much more robust.

### License/Copyright

This code is licensed under the GPLv2 open-source license contained in `gpl-2.0.txt` and copyright David C. Rankin, 2019.
