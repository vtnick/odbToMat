function steps = loadResults(resultsFolder)

stepFolders = dir(resultsFolder);
stepFolders = stepFolders(arrayfun(@(x) x.name(1), stepFolders) ~= '.');

for i=1:length(stepFolders)
    if stepFolders(i).isdir
        cStep = stepFolders(i).name;
        steps.(cStep) = load(fullfile(resultsFolder, cStep, 'stepInfo.mat'));
        frames = dir(fullfile(resultsFolder, cStep));
        frames = frames(arrayfun(@(x) x.name(1), frames) ~= '.');
        for j=1:length(frames)
            if frames(j).isdir
                cFrame = frames(j).name;
                frameNumStr = strsplit(cFrame, '_');
                frameNumber = str2double(frameNumStr{end})+1;
                resultfiles = dir(fullfile(resultsFolder, cStep, cFrame));
                resultfiles = resultfiles(arrayfun(@(x) x.name(1), resultfiles) ~= '.');
                frameFile = sprintf('frame_%d_Dict.mat', frameNumber-1);
                d = load(fullfile(resultsFolder, cStep, cFrame, frameFile));
                d.fieldOutputs = [];
                steps.(cStep).frames(frameNumber) = d;
                for f=1:length(resultfiles)
                    cfile = resultfiles(f).name;
                    cdataCells = load(fullfile(resultsFolder, cStep, cFrame, cfile));
                    [~,cfileName,~] = fileparts(cfile);
                    q = strsplit(cfileName, '_');
                    if strcmp(q{3}, 'Assembly')
                        cdata = cell2mat(cdataCells.allValues);
                        a = 'rootAssembly';
                        unit = strjoin(q(4:end),'_');
                        steps.(cStep).frames(frameNumber).fieldOutputs.(unit).(a).values = cdata;
                    elseif strcmp(q{3}, 'Dict')
                        z = 1;
                    elseif strcmp(q{3}, 'all')
                        cdata = cell2mat(cdataCells.allValues);
                        unit = strjoin(q(4:end),'_');
                        steps.(cStep).frames(frameNumber).fieldOutputs.(unit).values = cdata;
                    else
                        cdata = cell2mat(cdataCells.allValues);
                        a = 'instances';
                        instanceName = strjoin(q(3:4),'_');
                        unit = strjoin(q(5:end),'_');
                        steps.(cStep).frames(frameNumber).fieldOutputs.(unit).(a).(instanceName).values = cdata;
                    end
                end
            end
        end
    end
end