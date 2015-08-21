function P = get_preamble(p)
    %p.M = 5;
    p.Mset = 1:(p.M-2);
    P = [];
    d = get_random_symbols(p);
    k = length(get_kset(p));
    m = length(get_mset(p));
    
    n = 0:(k*m-1);
    d(:) = mod(round(1.414*n), 16);
    s = do_qammodulate(d, 4);
    %s(:) = 1;
    D = do_map(p, s);
    x = do_modulate(p, D);
    P = prepend_cp(x, p.NCP);
    %P(:) = 0;
    %P(100) = 1j;
end
