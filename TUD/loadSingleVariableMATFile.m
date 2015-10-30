function y = loadSingleVariableMATFile(filename)
foo = load(filename);
whichVariables = fieldnames(foo);
if numel(whichVariables) == 1
    y = foo.(whichVariables{1});
else
    % Do something else that defines y, or error
end