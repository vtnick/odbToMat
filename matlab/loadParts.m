function [parts] = loadParts(partsFolder)

partNames = dir(partsFolder);
partNames = partNames(arrayfun(@(x) x.name(1), partNames) ~= '.');

for i=1:length(partNames)
    x = strrep(partNames(i).name, '-', '_');
    
    % Load Nodes
    nodes = load(fullfile(partsFolder, x, 'nodes.mat'));
    [r, ~] = size(nodes.nodes);
    nodeStruct = repmat(struct('label', 0, 'coordinates', zeros(1,3)), r, 1);
    for j=1:r
        nodeStruct(j).label = nodes.nodes(j,1);
        nodeStruct(j).coordinates = nodes.nodes(j,2:end);
    end
    parts.(x).nodes = nodeStruct;
    
    % Load Elements
    elements = load(fullfile(partsFolder, x, 'elements.mat'));
    [r,c] = size(elements.connect);
    elementStruct = repmat(struct('label', 0, 'connect', zeros(1,c), 'eType', 'B32'), r, 1);
    for j=1:r
        elementStruct(j).label = elements.labels(j);
        elementStruct(j).eType = elements.eTypes{j};
        connect = elements.connect(j, :);
        elementStruct(j).connect = connect(connect > 0);
    end
    parts.(x).elements = elementStruct;
    
    % Load Node Sets
    nSetFile = fullfile(partsFolder, x, 'nodeSets.mat');
    if exist(nSetFile, 'file')
        nodeSets = load(nSetFile);
        [r,~] = size(nodeSets.nSetNodes);
        for j=1:r
            n = nodeSets.nSetNodes(j,:);
            nsetName = strrep(nodeSets.nSets{j}, '-', '_');
            parts.(x).nodeSets.(nsetName).nodes = n(n > 0);
        end
    end
    
    % Load Element Sets
    elSetFile = fullfile(partsFolder, x, 'elementSets.mat');
    if exist(elSetFile, 'file')
        elsets = load(elSetFile);
        [r,~] = size(elsets.eSetElems);
        for j=1:r
            el = elsets.eSetElems(j,:);
            elsetName = strrep(elsets.eSets{j}, '-', '_');
            parts.(x).nodeSets.(elsetName).elements = el(el > 0);
        end
    end
end