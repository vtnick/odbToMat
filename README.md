odbToMat
========

Outputs Abaqus odb information to MATLAB mat file. Designed with the intention of performing data analysis on Abaqus results using MATLAB. Uses the Abaqus ODB API as well as the MATLAB MAT-File API.

Uses [absolutepath mfile](http://www.mathworks.com/matlabcentral/fileexchange/3857-absolutepath-m) from File Exchange.

What it does:
========
1. Outputs geometry and results data from Abaqus odbs to mat files.
2. Attempts to replicate odb structure in MATLAB.
3. Example odb files are included.

Limitations:
=======
1. Most testing is done on Linux system using g++ compiler, therefore some Windows specific issues might exist.
2. Results output is limited to fieldout variables.
3. Issues with running out of memory when extracting data from large odbs.

How to use:
=======
1. Copy site level env file to run directory.
2. Add corresponding (win/linux) compile and linking flags to env file.
3. Use "abaqus make" utility to build.
4. Fill in necessary information in odbToMat_script.m and run.

Versions of Abaqus and MATLAB:
====================
Developed using Abaqus 6-13-1 and MATLAB R2013a. Abaqus C++ commands used do not appear to have any changes since at least 6-10.

Test Cases:
===========
1. Ubuntu 12.04 x64 with g++ 4.6
2. Scientific Linux 6.5 x64 with g++ 4.4.7
3. Windows 7 x64 in virtualbox with Microsoft VS 2010.

To-Do:
====
1. Fix issues with large data sets.
2. Clean up code.
3. Add ability to output on a set basis.
4. Add history outputs.
5. Add odbToMat_function.m.
