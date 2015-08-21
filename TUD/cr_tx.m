% load default HaLo config
[ halo ] = halo_defaultConfig('halo');

%% Configuration of the cognitive radio
% Split the available bandwidth in several channels
number_of_channels = 10;
%noise threshold to define free or occupied space
noise_threshold = 30;
%Simulate everything in Matlab or not
simulate = 1;

if (simulate == 0)
%% init TX Hardware
halo_init_tx(halo);

%% init RX Hardware
halo_init_rx(halo);

%% Waveform configuration (GFDM)
    p = get_defaultGFDM('BER');
    % number of symbols
    p.M = 15;
    % erase the first and last symbol to have small OOB
    p.Mset = 1:p.M-2;
    % total number of subcarriers
    p.K = 128;
    %activate a few subcarriers
    ks = 0:31;
    kSets = cell(0);
    kSets{end+1} = ks+3;
    p.Kset = kSets{1};
    % CP length
    p.NCP = p.K;
    % Modulation order -> 4QAM
    p.mu = 2;
    % define pulse shaping filter
    p.pulse = 'rc_fd';
    % roll of factor
    p.a = 0.1;

    %Generate the GFDM TX-signal
    signal = generate_signal(p);
    siglen = length(signal);
    halo_sendSignal(halo, signal); 
end

%% Generate the channellist with each individual center frequency and set all channels as occupied.
actual_channel = [];
channel_bw = halo.rx.samplerate / number_of_channels;
start_frequency = halo.rx.carrier_frequency - (halo.rx.samplerate / 2);
frequency = start_frequency+(channel_bw/2);
channel_list = [frequency 0];
for i = 2:number_of_channels
    frequency = frequency + channel_bw;
    channel_list = [channel_list; frequency 0];
end

%% Create window with spectrum plot
global running;
running = 1;
figure(99);
uicontrol('pos',[2 2 60 25],'string','Close','fontsize',12, 'callback','global running; running = 0;');
status_label = uicontrol('Style','text', 'Position',[65 2 120 20], 'String', 'Initialize');

%% Main loop running a state machines
state = 'Prepare sensing';
%Transmit signal a certain amount of time
timeout = 50;
cnt = 0;

while (running)
    
    %Don't freeze matlab
    pause(0.1);
    drawnow
    set(status_label, 'String', state);
    switch state
        
        case 'Prepare sensing'
            %% Configure RX device to sense the spectrum            
            if (simulate == 0)            
                %change carrier frequency
                send_command('command', 'set_carrier_frequency', 'value', halo.rx.carrier_frequency, 'device', halo.rx.device, 'mode', 'rx', 'type', halo.rx.type, 'id', halo.rx.id);
                %and samplerate
                send_command('command', 'set_samplerate', 'value', halo.rx.samplerate, 'type', halo.rx.type, 'device', halo.rx.device,'mode', 'rx', 'id', halo.rx.id);

                %run RX and make sure it does!
                halo_start_rx(halo);

                %dummy read
                samples = halo_getSignal(halo);
            end
            %next state
            state = 'Sense';
            
        case 'Sense'
            %% this state senses and evaluate the spectrum until free
            %channel found.
            if (simulate == 1)
                %simulate spectrum 
                laenge = 50000;
                start = 1;
                Nu = 20000;
                samples = rand(channel_bw,1)+1i*rand(channel_bw,1);
                samples(start:start+laenge-1) = 1000*ifft([sign(randn(Nu,1))+1i*sign(randn(Nu,1));zeros(laenge-Nu,1)]);
                samples = ifft(circshift(fft(samples),round(rand()*length(samples))));
            else
                %Receive samples from the RX device
                samples = halo_getSignal(halo);
            end
            
            if (~isempty(samples))
                %Convert the received time samples into frequency domain, sometimes a
                %fftshit is needed
                spectrum = 10*log10(abs(fftshift(fft(samples))));
                %Calibration could be necessary if a USRP is used
                %spectrum = spectrum - calibration_data;
                %generate the corresponding frequencies
                frequency = linspace(start_frequency,start_frequency + halo.rx.samplerate,length(spectrum))';
                %calculate the whitespace
                whitespace = sense(frequency, spectrum, noise_threshold);                   
                set(gca,'ytick',[1:length(frequency)/number_of_channels:length(frequency)-1]);
                %check all indivual channels
                for channel_index = 1:length(channel_list)
                    %get center frequency of current channel
                    channel_frequency = channel_list(channel_index, 1);
                    %is the channel free
                    result = intersect(find(whitespace(:,1) <= channel_frequency-(channel_bw/2)),find(channel_frequency+(channel_bw/2) <= whitespace(:,2)));
                    if ~isempty(result)
                        %Yes, channel is free
                        channel_list(channel_index, 2) = 1;
                    else
                        %No
                        channel_list(channel_index, 2) = 0;
                    end
                end
                %generate list of all free channel
                free_channels = find(channel_list(:,2)==1);
                %check if there is a free channel
                if ~isempty(free_channels)
                    %check if the channel needs to be switched
                    if isempty(actual_channel) || isempty(find(free_channels == actual_channel))
                        %pick the first one out of the list
                        actual_channel = free_channels(1);
                    end
                    %Channel found
                    disp(['Current channel ' num2str(actual_channel)]);
                    %Transmit data again.
                    state = 'Prepare TX';
                    if (simulate == 0)
                        send_command('command', 'stop', 'device', halo.rx.device, 'mode', 'rx', 'type', halo.rx.type, 'id', halo.rx.id);
                    end
                else
                    disp('All channels occupied.');
                end
            end
            
        case 'Prepare TX'
            %% Prepare TX device
            
            if (simulate == 0) 
                %change carrier frequency
                send_command('command', 'set_carrier_frequency', 'value', halo.tx.carrier_frequency, 'device', halo.tx.device, 'mode', 'tx', 'type', halo.tx.type, 'id', halo.tx.id);

                %run TX
                halo_start_tx(halo)
            end
            state = 'Transmit';
            
        case 'Transmit'
            %% Transmit until timeout reached.
            cnt = cnt + 1;
            set(status_label, 'String', [state ' ' num2str(timeout - cnt)]);
            if (cnt >= timeout)
                cnt = 0;
                state = 'Prepare sensing';
                if (simulate == 0) send_command('command', 'stop', 'device', halo.tx.device, 'mode', 'tx', 'type', halo.tx.type, 'id', halo.tx.id); end;
            end
    end
end

if (simulate == 0)
    send_command('command', 'stop', 'device', halo.tx.device, 'mode', 'tx', 'type', halo.tx.type, 'id', halo.tx.id);
    send_command('command', 'stop', 'device', halo.rx.device, 'mode', 'rx', 'type', halo.rx.type, 'id', halo.rx.id);
end

close(figure(99));