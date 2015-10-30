function [ constellation, syncMetric ] = process_signal( p, signal, sigTx)

P = get_preamble(p);
lP = length(P);
%% Correlation with the preamble to find synch-point
a = signal;
b = P;
A = fft(a);
B = fft(b,length(a));
for n = 1:100;
f = n-50;    
XX(:,n) = ifft(A.*circshift(conj(B),f-1));
end

if 0
plot(abs(XX))
end

[peaks row] = max(abs(XX));
[peak col] = max(abs(peaks));

epsilon = col-50-1;

start = row(col);
if start>length(sigTx)
    start = start - length(sigTx);
end

% CFO compensation
signal = signal.*exp(1i*2*pi*-epsilon/length(signal).*(0:(length(signal)-1))');

syncMetric = max(abs(XX(:,col)));

%% Channel estimation with the known preamble
Phat = signal(start+(0:lP-1));

if 0
close all;
getFigureDims(2,1)
getFigure(1,1)
plot(abs([Phat P]))
getFigure(2,1)
plot(abs([fft(Phat) fft(P)]))
end

Pcp = P(p.NCP+1:end);
Phatcp = Phat(p.NCP+1:end);
FP = fft(Pcp);
FPh = fft(Phatcp);
damp = 0.01;

Hhat = conj(FP).*FPh ./ (abs(FP.*FP)+damp);
hhat = ifft(Hhat);
lY = p.M * p.K;
Hhat = fft(hhat, lY);

%% Extract the data block
y = signal(start+lP+p.NCP+(0:lY-1));

% do channel equalization
Y = fft(y); Yeq = conj(Hhat).*Y ./ (abs(Hhat.*Hhat)+damp); yhat = ifft(Yeq);

Dhat = do_demodulate(p, yhat, 'ZF');

dhat = do_unmap(p, Dhat);

%% done
constellation = dhat;

