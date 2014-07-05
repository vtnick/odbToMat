clear
clc

% load mat file
x = load('../mats/freq.mat');

% extract field names
q = fieldnames(x);

% establish frame number structs first
for i=1:length(q)
    z = strsplit(q{i},'__');
    zlength = length(z);
    if zlength > 4
        n = str2double(z{5})+1;
    end
    if zlength == 5
        odb.(z{1}).(z{2}).(z{3}).(z{4})(n) = x.(q{i});
    end
end

% assign remaining
for i=1:length(q)
    z = strsplit(q{i},'__');
    zlength = length(z);
    if zlength > 4
        n = str2double(z{5})+1;
    end
    if zlength == 8
        odb.(z{1}).(z{2}).(z{3}).(z{4})(n).(z{6}).(z{7}).(z{8}) = x.(q{i});
    elseif zlength == 7
        odb.(z{1}).(z{2}).(z{3}).(z{4})(n).(z{6}).(z{7}) = x.(q{i});
    elseif zlength == 6
        odb.(z{1}).(z{2}).(z{3}).(z{4})(n).(z{6}) = x.(q{i});
    elseif zlength == 4
        odb.(z{1}).(z{2}).(z{3}).(z{4}) = x.(q{i});
    elseif zlength == 3
        odb.(z{1}).(z{2}).(z{3}) = x.(q{i});
    end
end

