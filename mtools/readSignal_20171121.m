function [ data, symbolPeriod, samplingPeriod, type, numberOfSymbols ] = readSignal_20171121( fname, nReadr )

%READSIGNALDATA Reads signal data to "visualizer".
%   [ data, samplingFrequency ] = READSIGNALDATA(fid, type, symbolPeriod, samplingPeriod)
%   just reads data ("data") from a file ("fid")
%   knowing the data parameters ("type", "symbolPeriod" and "samplingPeriod") and 
%   returning the new sampling simulation frequency ("samplingFrequency").

%   nReadr specifies the number of symbols to read
%% Global variables related to AWG limitations of sampling rate and waveform memory
maximumSamplingPeriod = 6.2500e-11;   % set the maximum sampling rate of the AWG
maximumWaveformMemory = 8e9;        % set the maximum waveform memory of the AWG

%% Open the "*.sgn" file generated by the simulator and read the content of the file
fid = fopen(fname,'r');                                     % read a particular "*.sgn" signal

type = fscanf(fid,'Signal type: %s\n',1);                   % Extraxt the type of the signal (i.e. type = TimeContinuousAmplitudeContinuousReal)
symbolPeriod = fscanf(fid,'Symbol Period (s): %f\n',1);     % Extract the symbol period of the signal (i.e. symbolPeriod = 2e-10)
samplingPeriod = fscanf(fid,'Sampling Period (s): %f\n',1); % Extract the sampling period of the signal (i.e. samplingPeriod = 1.25e-11). 
%                                                             REMEMBER OUR LIMITATION IS maximumSamplingRate = 6.2500e-11
fscanf(fid,'// ### HEADER TERMINATOR ###\n',1);             % It will just check our header terminator

%% Set samplingPeriod < 6.2500e-11

%% Some Standard types
tb = 'Binary';
tc1 = 'TimeDiscreteAmplitudeDiscreteComplex';
tc2 = 'TimeDiscreteAmplitudeContinuousComplex';
tc3 = 'TimeContinuousAmplitudeDiscreteComplex';
tc4 = 'TimeContinuousAmplitudeContinuousComplex';
tc5 = 'BandpassSignal';
toxy = 'OpticalSignalXY';

%% Get global variable "nRead" & set data types
if nargin == 1
    nReadr = Inf;
end
t_binaryr = 'int';
t_realr = 'double';
t_complexr = 'double';

%% Sampling frequency    
samplingFrequency = 1/samplingPeriod;

%% Number of samples per period
if (symbolPeriod==1)
    samplesPerSymbol = 1;
else
    samplesPerSymbol = (symbolPeriod/samplingPeriod);
end

%% Read data
if strcmp(type, tb) % Binary signals
    data = fread(fid, nReadr, t_binaryr);
    data = data';
    
   numberOfSymbols = (length(data)/samplesPerSymbol);
    
    return;
end

 
if strcmp(type, tc1) || strcmp(type, tc2) || strcmp(type, tc3) || strcmp(type, tc4) || strcmp(type, tc5)% Complex signals
   data = fread(fid, 2*samplesPerSymbol*nReadr, t_complexr);
   data = data(1:2:end) + 1i.*data(2:2:end);
   data = real(data)' + imag(data)'.*1i;
   
   numberOfSymbols = (length(data)/samplesPerSymbol);
   
   return;
end

if strcmp(type, toxy)
   data = fread(fid, 4*double(samplesPerSymbol)*nReadr, t_complexr);
   
   numberOfSymbols = (length(data)/samplesPerSymbol);
   
   return;
end

% Other signals
data = fread(fid, double(samplesPerSymbol)*nReadr, t_realr);
data = data';

numberOfSymbols = (length(data)/samplesPerSymbol);

