#!/usr/bin/ruby1.9.3
# this is a ruby wrapper of the usrp sensing engine (usrpse) binaries 

# Include the libraries 
require 'pty'
require 'optparse'
require "/var/lib/gems/1.8/gems/oml4r-2.9.1/lib/oml4r.rb"
def printerr(str)
	puts "ERROR: #{str}"
end

def printinfo(str)
	puts "INFO: #{str}"
end

# Some Interrupt handellers are defined to ensure proper termination of the usrpse binaries
Signal.trap("SIGINT") do
    printinfo "terminating from sigint..."
    Process.kill(:INT,$pid_global)
end
Signal.trap("SIGTERM") do
    printinfo "terminating from sigterm..."
    Process.kill(:INT,$pid_global)
end

###################################################
# 				Wrapper class					  #
###################################################
# This is the wrapper class, arguments are parsed, and the command for calling the binary is constructed 
class Wrapper
 	#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# A function to initialize the arguments
  	def initialize(args)
  		#=========================================================================================================
  		# 1) Define variables 
		## variables to be assigned by parsing input arguments  
	    @gain = 30                                                  # default RF gain value (in dB)				
		@spb = nil													# number of samples per buffer for processing value 
		@fftsize = 512 												# default FFT size used in processing
		@usrpip = nil												# USRP IP address
		@standard= nil			   									# The channel of the wireless standard the sensing engine is configured to measure 
		@output_mode = 'STDOUT'										# A varialbe to indicate the way to collect data
		@expid = nil 												# A variable to store experiment ID of OMF (ignored when @output_mode != 'OML')
		
		## variables to be derived afterwards according to the specified wireless standard and detection mode
		@numofchannel = nil											# number of channels in the specified standard
		@channelwidth = nil											# the width of the channel of the specified standard (in Hz) 
		@channeloffset = nil										# the offset between the center frequency of the specified channels  (in Hz)
		@mode = 'AVG'	 											# detection mode, by default perform averaging 
		@detector_types = {'COR'=>0, 'AVG'=>1, 'MAX'=>2, 'MIN'=>3}  # a hash to translate the @mode string into numeric values 
		
		## variables for sweeping mode only
		@bps = 4													# Number of 20 MHz blocks per sweep, needs 4 to cover the 80 MHz 2.4G ISM band
		@freqbegin = nil 											# The RF center frequency of the USRP front end at the first block 
		
		## variables for none_sweeping mode
		@freq=nil													# RF center frequency of the USRP frontend
		@rate=nil													# The sample rate 
		@firstchannel = nil											# The center frequency of the first channel that needs to be measured (only used in COR mode)
		@threshold= -80												# The threshold to determine if a channel is free (only used in COR mode)
		
		#=========================================================================================================
		# 2) Parse variables 
		## if no argument is given, provide an example and help option
		if ARGV[0].nil? 
			puts "Usage: #{$0} [options]"
			puts " e.g.: #{$0} --std WLAN_G --args 192.168.60.2 -m MAX"
			puts " e.g.: #{$0} [-h|--help]"
			exit 1
		end
		## define the options
		argParser = OptionParser.new do |argParser|
			argParser.on("-g", "--gain GAIN", "RFGain, default 30 dB") { |var|
				@gain = var 
			}
		    argParser.on("-s", "--spb SPB", "SamplePerBuffer Default 16777216 for channel occupation mode, else 1048576 ") { |var|
				@spb = var 
			}
		    argParser.on("-n", "--fftsize N", "FFTsize, default 512") { |var| 
				@fftsize = var 
	 		}
		    argParser.on("-a", "--args Addr", "The USRP IP address") { |var| 
				@usrpip = var
			}
		    argParser.on("-k", "--first_channel FIRSTCHANNEL", "FirstChannel, required for channel occupation mode, else ignored") { |var| 
		    	if numeric?(var)
					@firstchannel = var.to_i
				end
			} 
			argParser.on("-m", "--detector_mode MODE", "Detector, allowed values: COR, AVG, MAX, MIN. Default: AVG") { |var| 
				@mode = var 
			}
		    argParser.on("-d", "--std STANDARD", "Wireless Standard, allowed values: WLAN_G, ZIGBEE, BLUETOOTH") { |var| 
				@standard= var
			}
		    argParser.on("-o", "--output OUTPUT", "A varialbe to indicate the way to collect data, allowed values: STDOUT, FILE, OML") { |var| 
				@output_mode = var
			}	
			argParser.on("-e", "--expid expid", "A variable to store experiment ID of OMF (ignored when @output_mode != 'OML')") { |var| 
    			@expid = var
			}		
		end
		## call the parser 
		argParser.parse(args)
		
		## determine the usrp IP address if not specified, only works when the IP is in the pattern of 192.168.xx.2"
		if @usrpip.nil? 
			subnetfield = ` ifconfig | awk \'/192.168./ {print $2}\' | awk -F\".\" \'{print $3}\' `.delete("\n")
			if subnetfield.empty?
				printerr "invalid USRP IP, please specify with --args option or make sure the USRP IP falls into the pattern 192.168.xx.2 "
			else
				@usrpip = "192.168.#{subnetfield}.2"
				printinfo "using automatically detected usrp ip adddress: #{@usrpip}" 
			end	
    	end
    	
    	## check if the detector_mode is valid 
    	if @detector_types[@mode].nil?
    		printerr "invalid detector mode, please specify with --detector_mode option"
    		exit 1
    	end
    	
    	## check if the firstchannel is given if mode COR is selected 	
    	if @mode == 'COR' 
    		if @firstchannel.nil?
    			printerr "missing firstchannel, please specify with --first_channel option"
				exit 1	
			end
    	end 
	  	
		#=========================================================================================================
		# 3) Derive variables values according to the standard
		case @standard 
		when 'WLAN_G'
			@channelwidth = 20000000
			@channeloffset = 5000000
			if @mode == 'COR'
				if @firstchannel <1 or @firstchannel > 13
					printerr "invalid first channel, must be within [1,13]"
					exit 1
				end
				@numofchannel=1
				@freq=2412000000 + 5000000*(@firstchannel-1)
				@firstchannel = @freq 
				@rate = 25000000
			else 
				@firstchannel = 2412000000
				@numofchannel = 13
				@freqbegin = 2410000000
			end
    
		when 'ZIGBEE'
			@channelwidth = 4000000
			@channeloffset = 5000000
			if @mode == 'COR' 
				if @firstchannel <11 or @firstchannel > 23
					printerr "invalid first channel, must be within [11,23]"
					exit 1
				end
				@numofchannel=4
				@freq=2400000000 + 5000000*(@firstchannel-10+1.5)
				@firstchannel = 2400000000 + 5000000*(@firstchannel-10) 
				@rate = 25000000
			else
				@firstchannel = 2405000000
				@numofchannel = 16
				@freqbegin = 2412000000	
			end

		when 'BLUETOOTH'
			@channelwidth = 2000000
			@channeloffset = 1000000
			if @mode == 'COR' 
				if @firstchannel <1 or @firstchannel > 61
					printerr "invalid first channel, must be within [1,61]"
					exit 1
				end
				@numofchannel=20
				@freq=2401000000 + 1000000*(@firstchannel+10)
				@firstchannel=2401000000 + 1000000*@firstchannel 
				@rate = 25000000
			else
				@firstchannel = 2402000000
				@numofchannel = 79
				@freqbegin = 2410000000
			end
		else	
			printerr "invalid wireless standard, please specify with --std option"
			exit 1
		end
		#=========================================================================================================
		# 4) Construct the command 
		if @mode == 'COR' 
			if @spb.nil? 
				@spb = 4194304 * 4 	# assign default spb value when it is not specified
			end
			@cmd="./usrpse_nonesweep --freq #{@freq} --rate #{@rate} --gain #{@gain} --spb #{@spb} --fftsize #{@fftsize} --numofchannel #{@numofchannel} --firstchannel #{@firstchannel}  --channelwidth #{@channelwidth} --channeloffset #{@channeloffset} --args addr=#{@usrpip} --mode #{@detector_types[@mode]} --threshold #{@threshold}"		
		else
			if @spb.nil? 
				@spb = 4194304 	# assign default spb value when it is not specified, 1/4 of COR mode, because the there are 4 blocks per sweep 
			end	
			@cmd="./usrpse_sweeping --gain #{@gain} --spb #{@spb} --fftsize #{@fftsize} --numofchannel #{@numofchannel} --firstchannel #{@firstchannel} --channelwidth #{@channelwidth} --channeloffset #{@channeloffset} --args addr=#{@usrpip} --bps #{@bps} --freqbegin #{@freqbegin} --mode #{@detector_types[@mode]}"	
		end	
		
		#=========================================================================================================
		# 5) Prepare the data collection if needed 
		if @output_mode == 'OML' 
			initOML()		
		end
				
	end
	
	#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	## A function to initialize the parameters for OML 	
	def initOML()
		layouthash = Hash.new					# use the layout hash to describe the database structure 					
		layouthash["nodeid"] = "int32"			# column1: nodeid to identify which usrp inserted this row
		layouthash["freq"] = "int32" 			# column2: the RF freuquency of USRP (in case of sweeping mode, it is the frequency of the first block)
		
		# define the rest of the column names and the @oml_inject_str (contains ruby command to insert data) according to the number of channels and the mode 
		if @mode == 'COR'	
			@oml_inject_str="MPStatDynamic.inject(line.split(',')[1],#{(@freq/1000000).to_i},"
			for i in 1..@numofchannel
				layouthash["cor#{i}"]= "double"
				@oml_inject_str=@oml_inject_str+"line.split(',')[#{i+1}].delete('\\n').to_f,"
			end					
		else
			@oml_inject_str="MPStatDynamic.inject(line.split(',')[1],#{(@freqbegin/1000000).to_i},"
			for i in 1..@numofchannel
				layouthash["psd#{i}"]= "double"
				@oml_inject_str=@oml_inject_str+"line.split(',')[#{i+1}].delete('\\n').to_f,"
			end
		end    
		
		# create the measurement class
		classname = "#{@standard}_#{@mode}".downcase 
		MPBaseClassFactory.create_class("MPStatDynamic", classname, layouthash)
		if MPBaseClassFactory.class_exists?("MPStatDynamic")
			printinfo "measurement class #{classname} created ! "
		else
			printerr "measurement class #{classname} creation failed ! "
			exit 1 
		end 
		
		# complete the @oml_inject_str 
		@oml_inject_str = @oml_inject_str[0..-2] + ')'
		
		if @expid.nil? or @expid.include?(" ")
			printerr "invalid omf experiment ID, please specify with --expid option"
			exit 1
		end 
    	# call the OML library to initialize
    	OML4R::init(nil, { :appName => "usrpse", :collect => "tcp:am.wilab2.ilabt.iminds.be:3004", :domain => @expid} )# use oml server
   
	end 
	
	#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	## A function to return the @cmd variable  
	def getCMD()
		return @cmd 
	end
	#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	## A function to check if an object is a number	
	def numeric?(object)
  		true if Float(object) rescue false
	end
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ## A function to start the execution of the binary
	def start()
		printinfo "start execute the usrpse wrapper now..."
		printinfo "configure the system buffer size... "
		`sudo sysctl -w net.core.wmem_max=1048576`
		`sudo sysctl -w net.core.rmem_max=50000000`		
		 
		# open a child process to execute the sensing binaries 
		PTY.spawn @cmd do |r,w,p|
			# close the write pipe
			w.close
			# assign the process id to the global variable
			$pid_global=p
			# start the loop to read and process the output 
			loop { 
				line=r.gets
				puts line
				if @output_mode == 'OML'	
					if line.split(',').length >= 3 
		  				if numeric?(line.split(',')[3])
			  				eval(@oml_inject_str)
			  			end
			  		end
			  	end
			}
		end
	end

end

###################################################
# 				FACTORY					  		  #
###################################################
# The class that acts as a factory to produce class dynamically according to the sensing mode and standard
# - Each created class inherits from the MPBase OML-class.
# - Only used when the output type is configured to be OML
class MPBaseClassFactory
    # this is where the magic happens:
    def self.create_class(new_class, nam, params)
        c = Class.new OML4R::MPBase do                      # create new class c that extends MPBase
            name :"#{nam}"                                  # call MPBase method name with symbol
            params.each { |par, tp|                         # for each param, call MPBase method param with :ParName and type = string, long, double
                param :"#{par}", :type => "#{tp}" 
                printinfo "param #{par} :type => #{tp}"       
           }
        end
        Kernel.const_set new_class, c                       # so that you can new_class-param now becomes a reference to this class
    end

    # check if a class exists, returns true if it does
    def self.class_exists?(class_name)
      klass = Module.const_get(class_name)
      return klass.is_a?(Class)
    rescue NameError
      return false
    end
end

###################################################
# 				main process					  #
###################################################

begin
	app = Wrapper.new(ARGV)
	
	if app.getCMD().nil?
		printerr "creating class failed due to previous error"
	else
		app.start()
	end
	
rescue Exception => ex
	puts "Received an Exception when executing the wrapper!"
	puts "The Exception is: #{ex}\n"
end
