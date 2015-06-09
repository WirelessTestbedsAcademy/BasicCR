
function xcp = prepend_cp(x, NCP)
    xcp = [x(end-NCP+1:end); x];
end
