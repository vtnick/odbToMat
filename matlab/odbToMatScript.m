clear
clc
%% Set Paths
odbName = 'Frame_Freq'; % name of odb to extract
odbPath = fullfile('..', 'odbs', 'frame'); % path to odb
work_dir = fullfile('..', 'work'); % directory where scratch files are saved
pathToPy = fullfile('..', 'python'); % path to location of odbToMat.py
matDir = fullfile('..', 'mats'); % directory where mat will be saved
d = cd; % get current working directory

%% Call Abaqus Python
cd(pathToPy)
cmd = sprintf('abaqus python odbToMat.py %s %s %s', odbName, odbPath, work_dir);
[s,~] = system(cmd);
cd(d)
%% Read Mat Files From Python
if ~s
    odbFields = dir(fullfile(work_dir, odbName));
    odbFields = odbFields(arrayfun(@(x) x.name(1), odbFields) ~= '.');
    for i=1:length(odbFields)
        cField = odbFields(i);
        if strcmp(cField.name, 'parts')
            partFolder = fullfile(work_dir, odbName, cField.name);
            odb.parts = loadParts(partFolder);
            
        elseif strcmp(cField.name, 'assembly')
            assemblyFolder = fullfile(work_dir, odbName, cField.name);
            odb.assembly = loadAssembly(assemblyFolder);
            
        elseif strcmp(cField.name, 'results')
            resultsFolder = fullfile(work_dir, odbName, cField.name);
            odb.steps = loadResults(resultsFolder);
        end
    end
    odb = buildAllFieldOutput(odb);
    
    %% Save Mat File
    save(fullfile(matDir,odbName),'odb');
else
    disp('Abaqus Python Failed')
end
