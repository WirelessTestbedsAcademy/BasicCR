    addpath(genpath('C:\sync\global\Library\Matlab'));
    %Reference to UDP Server
    global server
    try
        if (server.udp_active ~= 1)
            init_udp(140, 200);
        end
    catch
        init_udp(140, 200);
    end

    tic
    while(1)
        zeit = toc;
        %Change load
        if (zeit > 10)
            
            %value for UE ID 0
            breite = round(rand * 3) * 10; %Remark: only 1, 10, 20, 30 are possible
            if (breite == 0) 
                breite = 1;
            end  
            breite = 10;
            start = round(rand * 94 + 2);
            if (start + breite > 96)
                start = 96 - breite;
            end    
            
            send_command('command', 'change', 'ueid', '0', 'num_prbs' , breite, 'first_prb', start, 'mcs_mod', 'qpsk', 'type', '124');
            disp(['UE0' num2str(breite) num2str(start)]);
            
            %value for UE ID 1
            breite = round(rand * 3) * 10; %Remark: only 1, 10, 20, 30 are possible
            if (breite == 0) 
                breite = 1;
            end
            breite = 10;
            start = round(rand * 94 + 2);
            if (start + breite > 96)
                start = 96 - breite;
            end    
            disp(['UE1' num2str(breite) num2str(start)]); 
            
            send_command('command', 'change', 'ueid', '0', 'num_prbs' , breite, 'first_prb', start, 'mcs_mod', 'qpsk', 'type', '124');            
            %Send command to eNB
            send_command('command', 'set', 'type', '124');
            tic
        end
        
        if (mod(zeit,3) < .1)
            disp(round(zeit))
        end
            
        %Don't freeze gui
        drawnow;
        pause(0.1);        
    end
