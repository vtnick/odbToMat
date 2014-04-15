from os import path, walk
from shutil import rmtree
from odbAccess import *
from abaqusConstants import *
import inpParser


def parseInputFile(odb):
    pathToOdb, odbName = path.split(odb.path)
    pathToPes = path.join(pathToOdb, odbName.split('.')[0] + '.pes')
    pathToInp = path.join(pathToOdb, odbName.split('.')[0] + '.inp')
    if path.exists(pathToPes):
        inputFile = inpParser.InputFile(pathToPes)
    elif path.exists(pathToInp):
        inputFile = inpParser.InputFile(pathToInp)
    else:
        print "No input file found in ODB directory found"
    p = inputFile.parse(usePyArray=True)
    lower, upper = -1, 0
    for i, j in enumerate(p):
        if j.name == 'assembly':
            lower = i
        elif j.name == 'endassembly':
            upper = i
    return p, lower, upper


def cleanUpEmpty(folder):
    for root, dirs, files in walk(folder):
        if not files:
            rmtree(folder)
