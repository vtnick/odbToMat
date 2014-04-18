function odb = loadGeometry(geometryFolder)

instanceNames = dir(geometryFolder);
instanceNames = instanceNames(arrayfun(@(x) x.name(1), instanceNames) ~= '.');

for i=1:length(instanceNames)
    x = strrep(instanceNames(i).name, '-', '_');
    
    % Load Nodes
    nodes = load(fullfile(geometryFolder, x, 'nodes.mat'));
    nodeStruct = cell2mat(nodes.nodes);
        
    % Load Elements
    elements = load(fullfile(geometryFolder, x, 'elements.mat'));
    elementStruct = cell2mat(elements.elements);
    if strcmp(x, 'rootAssembly')
        odb.rootAssembly.nodes = nodeStruct;
        odb.rootAssembly.elements = elementStruct;
    else
        odb.rootAssembly.instances.(x).nodes = nodeStruct;
        odb.rootAssembly.instances.(x).elements = elementStruct;
    end
    
    % Load Node Sets
    nSetFile = fullfile(geometryFolder, x, 'nodeSets.mat');
    if exist(nSetFile, 'file')
        z = load(nSetFile);
        if strcmp(x, 'rootAssembly')
            odb.rootAssembly.nodeSets = z.nodeSets;
        else
            odb.rootAssembly.instances.(x).nodeSets = z.nodeSets;
        end
    end
    
    % Load Element Sets
    elSetFile = fullfile(geometryFolder, x, 'elementSets.mat');
    if exist(elSetFile, 'file')
        elsets = load(elSetFile);
        if strcmp(x, 'rootAssembly')
            odb.rootAssembly.elementSets = elsets.elementSets;
        else
            odb.rootAssembly.instances.(x).elementSets = elsets.elementSets;
        end
    end
end