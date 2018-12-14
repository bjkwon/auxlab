# auxlab

AUXLAB consists of three windows: 1) AUdio syntaX console terminal, 2) variable/status window, and 3) history window. In addition, 4th window can be opened for a debugger.

AUXLAB was developed C++ with Windows API. Yacc/lex was used for syntax parsing and tokenizing.

Current release was developed with Visual Studio 2017.

To build the app in your machine, edit auxlab.props and change the two macros, DropDev and BuildDir on lines 5 and 7.
1) DropDev specifies the path for sources
2) BuildDir specifies the path for build

Change them according to the directory structure in your machine.
