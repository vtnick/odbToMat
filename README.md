odbToMat
========

Outputs Abaqus odb information to MATLAB mat file. Designed with the intention of performing data analysis on Abaqus results using MATLAB. Uses the SciPy library from [scipy.org](scipy.org). If using built-in Numpy in Abaqus, Scipy needs to work with that version of Numpy. SciPy 0.7- 0.7.2 seems to work ok.

What it does:
========

1. Outputs assemlby and part information (nodes, elements) from Abaqus odbs to mat files.
2. Outputs field outputs from odb to mat. Field outputs are broken down by instance as well as combined into one matrix.
3. Attempts to replicate odb structure in MATLAB.
4. Example odb files are included.

Limitations:
=======
1. Currently does not work for files without part instances.
2. Currently does not output history output.
3. Currently requires inp/pes file to accompany the odb to extract assembly node and element definitions. Currently Abaqus Python has some errors when requesting some information directly from the odb file.
4. Does not preserve Abaqus class structures and functions.
5. User is responsible for installation and linking of Scipy library to use with Abaqus Python. Can copy the SciPy library into the folder with odbToMat, use PYTHONPATH enviroment variables, or add sys.path.append to odbToMat.py with correct path.
6. Only captures data found in the "data" portion of the Abaqus field outputs. Does not output other fields like "maxPrincipal" for stress for example.
7. Other things not yet found. Only limited testing thus far.

How to use:
=======

In MATLAB fill in required variables in the "odbToMatScript.m" file. File and folder paths can be given relative or full path. MATLAB files and Python files are written to use cross-platform path functions and convert to full paths when needed. "odbToMatScript.m" will call the necessary python file.

"odbToMatFunction.m" is a function version of the script. By default does not save the resulting mat file. Variable inputs to the function can specifiy the location to save the file.

Versions of Abaqus and MATLAB:
====================

Developed using Abaqus 6-13-1 and MATLAB R2013a.

To-Do:
====

1. Clean up code for readability and include comments.
2. Break up the code into smaller modules.
3. Add support for odbs without part instances.
4. Try to drop the need for inp/pes files.
