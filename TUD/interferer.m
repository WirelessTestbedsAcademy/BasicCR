%% init TX Hardware 
% load default HaLo config
[ halo ] = halo_defaultConfig('halo');
   
%% Generate arbitrary waveform
T=10000;%total number of samples
N=T/2;%signal burst length
Nu=N/4;%number of used subcarriers in OFDM signal

tx_signal=[ifft([sign(randn(Nu,1))+1i*sign(randn(Nu,1));zeros(N-Nu,1)]);zeros(T-N,1)];

% Generate the channellist with each individual center frequency and set all channels as occupied.
[ channel_list, channel_bw ] = get_channel_list(halo);

%% additional configuration for the interferer
halo.tx.samplerate = channel_bw;
halo.tx.buffer_size = length(tx_signal);
halo.tx.gain = 10;
halo.tx.id = 3;
halo.tx.type = 150;
halo.tx.device = '192.168.200.95';
halo_init_tx(halo);

%% Transmit waveform
halo_sendSignal(halo, tx_signal);

while(1)
    actual_channel = round(rand()*9 + 1);
    %change carrier frequency
    halo.tx.carrier_frequency = channel_list(actual_channel);    
    send_command('command', 'set_carrier_frequency', 'value', halo.tx.carrier_frequency, 'device', halo.tx.device, 'mode', 'tx', 'type', halo.tx.type, 'id', halo.tx.id);
    halo_start_tx(halo);    
    for i=6:-1:0
        pause(1);
        disp(['Will switch channel ' num2str(actual_channel) ' in ' num2str(i)]);
    end
    send_command('command', 'stop', 'device', halo.tx.device, 'mode', 'tx', 'type', halo.tx.type, 'id', halo.tx.id);
end