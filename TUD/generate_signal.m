function [ signal ] = generate_signal( p )
pp = p;

P = get_preamble(pp);

d = get_random_symbols(p);
s = do_qammodulate(d, p.mu);
D = do_map(p, s);

x = do_modulate(p, D);
%x(:) = 0;
xcp = prepend_cp(x, pp.NCP);

signal = [P; xcp];
ZL = length(signal);
signal = [signal; zeros(round(ZL/3),1)];

end


