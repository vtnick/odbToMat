%% Script to run odbToMat
% The following script sets the parameters necessary to run the
% odbToMat.exe

clear
clc
%% Set Parameters
path_to_odbDir = absolutepath(fullfile('./', 'odbs', filesep));
odbName = 'baseRuns613.odb';
% path_to_odb = [path_to_odbDir, odbName];
path_to_odb = fullfile(path_to_odbDir, odbName);

path_to_MATDir = absolutepath(fullfile('./', 'mats', filesep));
matName = strsplit(odbName, '.');
% path_to_MAT = [path_to_MAT_fileDir, matName{1} '.mat'];
path_to_MAT = fullfile(path_to_MATDir, matName{1});


parameterString = sprintf('-odb %s -matPath %s', path_to_odb, path_to_MAT);

%% Set Run Paths
cpath = cd;
path_to_odbToMat = absolutepath(fullfile('./'));
cd(path_to_odbToMat);

%% Run Program
cmd = sprintf('abaqus odbToMat.exe %s', parameterString);
s = system(cmd);
cd(cpath);
