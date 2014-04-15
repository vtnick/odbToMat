function a = loadAssembly(assemblyFolder)

assemblyFiles = dir(assemblyFolder);
assemblyFiles = assemblyFiles(arrayfun(@(x) x.name(1), assemblyFiles) ~= '.');

nFiles = length(assemblyFiles);
% Load Nodes
nEls = 1;
for i=1:nFiles
    cFile = assemblyFiles(i).name;
    if strcmp(cFile, 'nodes.mat')
        nodes = load(fullfile(assemblyFolder, cFile));
        [r, ~] = size(nodes.nodes);
        nodeStruct = repmat(struct('label', 0, 'coordinates', zeros(1,3)), r, 1);
        for j=1:r
            nodeStruct(j).label = nodes.nodes(j,1);
            nodeStruct(j).coordinates = nodes.nodes(j,2:end);
        end
        a.nodes = nodeStruct;
    
    elseif strcmp(cFile(1:8), 'element_')
        element = load(fullfile(assemblyFolder, cFile));
        [~, fname, ~] = fileparts(fullfile(assemblyFolder, cFile));
        c = strsplit(fname, '_');
        elementNumber = str2double(c{2});
        fields = fieldnames(element);
        connect = {};
        for k=1:length(fields)
            if ~strcmp(fields{k}, 'eType')
                d = strsplit(fields{k}, '_');
                w = element.(fields{k});
                if isempty(w)
                    connect{str2double(d{2})} = w;
                else
                    connect{str2double(d{2})} = strrep(w,'-','_');
                end
            end
        end
        a.elements(nEls).label = elementNumber;
        a.elements(nEls).eType = element.eType;
        a.elements(nEls).connect = connect;
        nEls = nEls+1;
        
    elseif strcmp(cFile(1:5), 'nset_')
        nset = load(fullfile(assemblyFolder, cFile));
        fields = fieldnames(nset);
        if ismember('instance', fields)
            nset.instance = strrep(nset.instance, '-', '_');
        end
        [~, fname, ~] = fileparts(fullfile(assemblyFolder, cFile));
        c = strsplit(fname, 'nset_');
        nsetName = c{end};
        if strcmp(nsetName(1), '_')
            nsetName = nsetName(2:end);
        end
        a.nsets.(nsetName) = nset;
        
    elseif strcmp(cFile(1:6), 'elset_')
        elset = load(fullfile(assemblyFolder, cFile));
        fields = fieldnames(elset);
        if ismember('instance', fields)
            elset.instance = strrep(elset.instance, '-', '_');
        end
        [~, fname, ~] = fileparts(fullfile(assemblyFolder, cFile));
        c = strsplit(fname, 'elset_');
        elsetName = c{end};
        if strcmp(elsetName(1), '_')
            elsetName = elsetName(2:end);
        end
        a.elsets.(elsetName) = elset;
        
    end
end
    