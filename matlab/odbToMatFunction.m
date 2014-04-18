function odb = odbToMatFunction(odbName,odbPath,work_dir,pathToPy,varargin)

%% Variable Inputs
if isempty(varargin)
    saveKey = 0;
elseif length(varargin) < 2 || length(varargin) > 2
    error('Incorrect Number of Inputs')
else
    matDir = varargin{1};
    saveKey = varargin{2};
end

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
        if strcmp(cField.name, 'geometry')
            geometryFolder = fullfile(work_dir, odbName, cField.name);
            odb = loadGeometry(geometryFolder);
            
        elseif strcmp(cField.name, 'results')
            resultsFolder = fullfile(work_dir, odbName, cField.name);
            odb.steps = loadResults(resultsFolder);
        end
    end
    
    %% Save Mat File
    if saveKey
        save(fullfile(matDir,odbName),'odb');
    end
else
    error('Abaqus Python Failed')
end