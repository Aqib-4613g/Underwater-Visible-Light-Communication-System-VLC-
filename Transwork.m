% sendVLCmessage_loop.m
% Send newline-terminated messages to the ESP32 transmitter (COM7) repeatedly
clear; clc;

% ---- User settings ----
txPort = 'COM7';      % <<-- HARD-CODED transmitter COM port (change here if needed)
BAUD = 115200;        % must match transmitter Serial.begin(...)
listenSeconds = 6;    % how many seconds to listen to TX logs after each send

% ---- Sanity check available ports ----
ports = serialportlist("available");
% normalize to cellstr (robust across MATLAB versions)
if isstring(ports)
    ports = cellstr(ports);
elseif ischar(ports)
    ports = {ports};
elseif ~iscell(ports)
    ports = cellfun(@char, ports, 'UniformOutput', false);
end

fprintf('Available ports:\n');
for k = 1:numel(ports)
    fprintf('  [%d] %s\n', k, ports{k});
end

if ~ismember(txPort, ports)
    error('TX port %s not found in available ports. Make sure the board is plugged in and Arduino Serial Monitor for TX is CLOSED.', txPort);
end

% ---- Open transmitter serial port once ----
fprintf('Opening %s at %d baud...\n', txPort, BAUD);
try
    sp = serialport(txPort, BAUD, "Timeout", 1);
catch ME
    error('Could not open %s. Make sure Arduino IDE Serial Monitor for the transmitter is CLOSED. Error: %s', txPort, ME.message);
end

% allow ESP32 auto-reset/boot
pause(2.2);
% flush any boot text
try flush(sp); catch; end

% ---- Main send loop ----
while true
    % ask message
    msg = input('Enter message to transmit (leave empty to cancel): ', 's');
    if isempty(msg)
        fprintf('Empty message — exiting loop.\n');
        break;
    end

    % send message (newline appended automatically by writeline)
    try
        writeline(sp, msg);
    catch ME
        warning('Failed to write to serial port: %s', ME.message);
        break;
    end

    fprintf('Message sent to transmitter: "%s"\n', msg);
    fprintf('Transmitter should start VLC transmission now.\n');

    % listen for a few seconds of transmitter logs
    fprintf('Listening for %d seconds of transmitter serial output...\n', listenSeconds);
    tstart = tic;
    while toc(tstart) < listenSeconds
        if sp.NumBytesAvailable > 0
            try
                line = readline(sp);   % reads until LF
                fprintf('[TX] %s\n', strtrim(line));
            catch
                data = read(sp, sp.NumBytesAvailable, 'char');
                if ~isempty(data)
                    fprintf('[TX] %s\n', data);
                end
            end
        else
            pause(0.05);
        end
    end

    % Ask whether to send another
    answer = input('Send another message? (Y/N): ', 's');
    if isempty(answer) || lower(answer(1)) ~= 'y'
        fprintf('Exiting send loop.\n');
        break;
    end
end

% ---- Cleanup ----
try
    clear sp;
catch
    % ignore
end
fprintf('Done. Receiver Serial Monitor (Arduino IDE) should show the decoded message(s).\n');
