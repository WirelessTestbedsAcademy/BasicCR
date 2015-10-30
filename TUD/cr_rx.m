% load default HaLo config
[ halo ] = halo_defaultConfig('halo');

%% Configuration of the cognitive radio
% Split the available bandwidth in several channels
% halo.rx.samplerate --> defines the BW of the used spectrum
% halo.tx.samplerate --> defines the BW one channel within the used spectrum
number_of_channels = round(halo.rx.samplerate / halo.tx.samplerate);
%noise threshold to define free or occupied space
noise_threshold = 30;
%Simulate everything in Matlab or not
simulate = 0;

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

%% additional configuration for the GFDM receiver
halo.rx.fetch_rate = 0;
% catch enough data to contain at least 2 complete frames
halo.rx.buffer_size = 3*siglen;
halo.rx.gain = 30;
halo.rx.id = 2;
halo.rx.type = 150;
halo.rx.device = '192.168.200.93';
halo.rx.fetch_rate = 1;
% Load calibration data for USRP
calibration_data = loadSingleVariableMATFile('usrp_cal.mat');

%% init RX Hardware
if (simulate == 0)
    halo_init_rx(halo);
end

%% Generate the channellist with each individual center frequency and set all channels as occupied.
[ channel_list, channel_bw ] = get_channel_list(halo);
%redefine sample rate with BW of one channel within the used spectrum
%because Receiver tries to receiver only one channel not the whole spectrum
halo.rx.samplerate = channel_bw;
halo.tx.samplerate = channel_bw;

%% Create window with spectrum plot
figure(101);
uicontrol('pos',[2 2 60 25],'string','Close','fontsize',12, 'callback','global running; running = 0;');
status_label = uicontrol('Style','text', 'Position',[65 2 120 20], 'String', 'Initialize');
channel_label = uicontrol('Style','text', 'Position',[185 2 120 20], 'String', '1');

%% Main loop running which tries to receive and demodulate the signal for a given amount of time

lock = 0;
versuch = 0;
%Current used channel
actual_channel = 1;
%Metric to define if GFDM frame has been found
metric = 60;
change = 1; % 1 = Channel has switched otherwise 0

%% main loop, which is excuted as long the 'close' button is not pressed.
global running;
running = 1;
while(running)
    
    %% Channel has changed --> Reconfiguration of the hardware
    if ((simulate == 0) && (change == 1))
        %make sure USRP is stopped.
        while(1)
            [result, return_message] = send_command('command', 'is_running', 'device', halo.rx.device, 'mode', 'rx', 'type', halo.rx.type, 'id', halo.rx.id);
            if ((strcmpi(return_message, 'false') == 1) && (result == 1)) break; end
            send_command('command', 'stop', 'device', halo.rx.device, 'mode', 'rx', 'type', halo.rx.type, 'id', halo.rx.id);
            pause(2);
        end
        %change carrier frequency
        halo.rx.carrier_frequency = channel_list(actual_channel);
        
        %confifure!
        halo_init_rx(halo);

        %run RX!
        halo_start_rx(halo);
        
        %dummy read
        samples = halo_getSignal(halo);
        change = 0;
    end
    
	%% Get signal from the hardware
    if (simulate == 0)
        rec = halo_getSignal(halo);
    else 
        rec = rand(halo.rx.buffer_size, 1);
    end
    
    if ~isempty(rec)
        
        try
			% Calculate PSD
            spectrum = 10*log10(abs(fftshift(fft(rec, length(calibration_data))))) - calibration_data;
            subplot(2,2,1);
            plot(spectrum, '-b');
            axis([0 length(spectrum) -40 40]);
            
			% Perform correlation with transmit signal for the GUI
            correlation = abs(xcorr(signal, rec));
            subplot(2,2,2)
            plot(correlation,'-b');
            axis([0 length(correlation) 0 100]);
            
            subplot(2,2,3)
            plot(abs(xcorr(signal, signal)),'-r');
            
            %Process received signal
            [ cons, maxSync ] = process_signal(p, rec, signal);
            
            subplot(2,2,4)
            plot(cons, 'x');
            axis([-2 2 -2 2]);
            
			%Check if the GFDM frame has been found
            if (maxSync < metric)
				
                if (lock >= 0)
					%Try again on the same channel, maybe the signal is still there
                    lock = lock - 1;
                    set(status_label, 'String', ['Will switch in ' num2str(lock) '...']);
                else		
					%GFDM frame is not found, try again?
                    versuch = versuch + 1;
                    if (versuch > 4)
						%Switch channel
                        versuch = 0;
                        change = 1;
                        actual_channel = actual_channel + 1;
                        if (actual_channel > length(channel_list))
                            actual_channel = 1;
                        end
                        set(channel_label,'String', ['Current channel: ' num2str(actual_channel)]);
                        set(status_label, 'String', 'Switch channel!');                     
                    else
                        set(status_label, 'String', 'Searching...');
                    end
                end
            else
				%GFDM frame found
                lock = 8;
                set(status_label, 'String', 'Locked!');
            end
            disp([lock maxSync]);
        catch err
            disp(['Error evaluating RX data: ' err.message]);
        end
        
    end
    %Don't freeze gui
    drawnow;
    pause(0.1);
    %dummy read
    if (simulate == 0) receive_packet(0,1); end   
end

%% shutdown
if (simulate == 0)
    send_command('command', 'stop', 'device', halo.rx.device, 'mode', 'rx', 'type', halo.rx.type, 'id', halo.rx.id);
end

close(figure(101));