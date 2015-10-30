%White space sensing
%Input: frequency and corresponding magnitude (column vectors)
%Output: matrix whitespace - first column: start of white space area -
%second column - end of white space areas

function whitespace = sense(frequency, magnitude, treshold, min_space)

    %Input data - data(:,1) = frequency - data(:,2) = magnitude
    data = [frequency, magnitude];

    %Define treshold boundary
    if ~exist('treshold', 'var')
        treshold = [];     
    end
    if isempty(treshold)
        treshold = -90;
    end
    
    if isempty(treshold)
        min_space = 1000;
    end    
    
%     %Define minimum size of white spaces in frequency samples
%     min_samples = 10;
     delta_f =(data(2,1)-data(1,1));
%     min_space = min_samples*delta_f;    
    
    %Generate plot? -->y/n
    plot1 = 'y';

    %Sense spectral samples
    %Find indices of data where the spectrum is free (magnitude<treshold)
    free = find(data(:,2)<treshold);
    %Generate a new cloumn for the spectral sensing result
    data = [data, zeros(size(data,1),1)];
    %If spectral sample is sensed free --> data(:,3)=1 , if not --> data(:,3)=0 
    data(free,3)=1; 
    
    %Detect white space areas
    %sflag=1 denotes whether a start frequency was found or not
    sflag=0;
    whitespace = [];
    for i=1:size(data,1)
      
          %If the sample is free and there is no start frequency found yet  
          if ((data(i,3) == 1) && (sflag == 0))
            %Define start of white space area  
            start = data(i,1);
            sflag = 1;
          %If the sample is not free but a start frequency was found before
          elseif ((data(i,3) == 0) && (sflag == 1))
            %Define end of white space area  
            stop = data(i,1);
            %Save the start and end of the free space area in whitespace
            if (stop - start > min_space) 
                whitespace = [whitespace; start, stop];
            end
            sflag=0;
          end
          
          %If no end is found before end of spectrum 
          if ((i == size(data,1)) && (sflag == 1))
            stop = data(i,1);
            whitespace = [whitespace; start, stop];  
          end
          
    end
    
%     if ~isempty(whitespace)
%         %Delete white spaces smaller than minimum size 
%         too_small = find((whitespace(:,2)-whitespace(:,1)) < min_space);
%         whitespace(too_small,:) = [];
%     end

    %Generate plot 
    if plot1=='y'
        figure(99);
        %Normalization to GHz
        plotdata = [data(:,1)/10^9, data(:,2)];
        plotwhitespace = whitespace/10^9;
        %Plot spectrum
        h = plot(plotdata(:,1),plotdata(:,2));
        xlim([plotdata(1,1) (plotdata(end,1)+delta_f/10^9)]);
        grid on
        %Plot treshold line
        line(xlim, [treshold treshold],'Linewidth',2.2,'color',[154/255 1 154/255]);
        xlabel('Frequency / GHz','FontWeight','bold'); 
        title('White space sensing','FontWeight','bold');
        ylabel('Magnitude / dBm','FontWeight','bold');
        legend('Spectrum','Treshold');
        %Mark white spaces
        y = ylim;
        for i=1:size(plotwhitespace,1) 
            rectangle('position',[plotwhitespace(i,1),y(1,1),abs(plotwhitespace(i,1)-plotwhitespace(i,2)),abs(y(1,1))+treshold],'FaceColor',[154/255 1 154/255],'EdgeColor',[154/255 1 154/255]);
        end
        %Get spectrum in front layer 
        uistack(h, 'top');        
    end
     
end



