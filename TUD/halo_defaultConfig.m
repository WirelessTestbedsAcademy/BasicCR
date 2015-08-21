function [ halo ] = halo_defaultConfig(mode )
%Add path to librarys provided by TUD
addpath(genpath('C:\sync\global\Library\Matlab'));

%Different halo modes available:
if strcmp(mode, 'halo')    
    %% General settings
    halo.mode = 'halo';                     %Working mode
    halo.own_id = 99;                       %id of the current matlab instance
    halo.own_type = 140;                    %type of the current matlab instance
    halo.number_of_tries_to_setup = 3;      %if a command fails, number of retries
    halo.save_packets = 50;                 %how many packets should be save by the halo script
    
    %% TX Parameter
    halo.tx.type = 150;
    halo.tx.id = 1;    
    halo.tx.carrier_frequency = 1990000000; %GHz
    halo.tx.samplerate = 1000000;
    halo.tx.gain = 0;
    halo.tx.clock = 'internal';
    halo.tx.device = '192.168.200.90';      %Sometimes a TX Application can control serveral units
                                            %USRP: enter IP-Adresse
                                            %PXI: enter name of the fpga
    halo.tx.buffer_size = 1000000;          %Length of the transamitted data
    
    %% RX Parameter                                        
    halo.rx.type = 150;
    halo.rx.id = 2;     
    halo.rx.carrier_frequency = 1990000000;
    halo.rx.gain = 0;
    halo.rx.clock = 'internal';
    halo.rx.device = '192.168.200.91';      %Sometimes a RX Application can control serveral units
                                            %USRP: enter IP-Adresse
                                            %PXI: enter name of the fpga
    halo.rx.samplerate = 1000000;
    halo.rx.fetch_rate = 0;                 %How often should the rx device send the received data to matlab
    halo.rx.min_power = 1e-8;               %Minimum power that the halo script will identify that something was received and not only 0
    halo.rx.buffer_size = 100000;           %Length of receive samples   
    
elseif strcmp(mode, 'replay') 
    %% Mode to replay stored data    
    halo = struct;
    halo.mode = 'replay';
    halo.return_packet_number = 1;
end

%Folder to store recorded IQ-data
halo.recorded_samples_dir = 'records';

end

