function odb = buildAllFieldOutput(odb)

steps = fieldnames(odb.steps);
nSteps = length(steps);

for i=1:nSteps
    cstep = odb.steps.(steps{i});
    nFrames = length(cstep.frames);
    for frame=1:nFrames
        cframe = cstep.frames(frame);
        fieldOuputs = fieldnames(cframe.fieldOutputs);
        for j=1:length(fieldOuputs)
            cField = cframe.fieldOutputs.(fieldOuputs{j});
            partNames = fieldnames(cField.parts);
            if ismember('assembly', fieldnames(odb))
                instances = [{'rootAssembly'}; partNames];
                values = [ones(length(cField.assembly.values(:,1)),1) cField.assembly.values];
            else
                instances = partNames;
                values = [];
            end
            key = {'instanceNumber', 'Node/Element Label', 'componentLabels'};
            odb.steps.(steps{i}).frames(frame).fieldOutputs.(fieldOuputs{j}).allValues.instances = instances;
            odb.steps.(steps{i}).frames(frame).fieldOutputs.(fieldOuputs{j}).allValues.key = key;
            for k=1:length(partNames)
                c = k+1;
                nValues = length(cField.parts.(partNames{k}).values(:,1));
                values = [values;c*ones(nValues,1) cField.parts.(partNames{k}).values];
            end
            odb.steps.(steps{i}).frames(frame).fieldOutputs.(fieldOuputs{j}).allValues.values = values;
        end
    end
end
            