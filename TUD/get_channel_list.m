function [ channel_list, channel_bw, start_frequency, number_of_channels] = get_channel_list(halo)
    % Split the available bandwidth in several channels
    number_of_channels = round(halo.rx.samplerate / halo.tx.samplerate);
    %% Generate the channellist with each individual center frequency and set all channels as occupied.
    channel_bw = halo.rx.samplerate / number_of_channels;
    start_frequency = halo.rx.carrier_frequency - (halo.rx.samplerate / 2);
    frequency = start_frequency+(channel_bw/2);
    channel_list = [frequency 0];
    for i = 2:number_of_channels
        frequency = frequency + channel_bw;
        channel_list = [channel_list; frequency 0];
    end
end