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
3. Does not currently use v7.3 MAT files, therefore large odbs will not save and give seg faults. The HDF5 v7.3 is needed to store variables larger than 2gb. Currently having troubles saving using v7.3.

How to use:
=======
1. Run corresponding build file. File will extract ODB API and move the libraries. File will also compile the odbToMat. Windows users check path to vcvars file in the .bat to make sure it is correct.
2. Fill in necessary information in odbToMat_script.m and run.

Versions of Abaqus and MATLAB:
====================
Developed using Abaqus 6-13-1 and MATLAB R2013a. Build scripts extract API according to version of Abaqus installed.

Test Cases:
===========
1. Ubuntu 12.04 x64 with g++ 4.6
2. Windows 7 x64 in virtualbox with Microsoft VS 2010.

To-Do:
====
1. Fix issues with large data sets.
2. Clean up code.
3. Add ability to output on a set basis.
4. Add history outputs.
5. Add odbToMat_function.m.
