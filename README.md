MatLab MEX library for receiving multicast UDP packets. This implementation splits connect and receive functionality into separate functions, so you don't have to keep opening and closing connection for every packet (which can be a big bottleneck in the code).

**Basic usage example:**
```matlab
MC_IP   = '224.0.0.0';
MC_PORT = 1024;
RX_WAIT = 1;     % ms
NUM_RX  = 100;

% Open UDP connection and store handle in udpH
udpH    = udpOpen(MC_IP,MC_PORT,RX_WAIT);

for irx=1:NUM_RX,
    
    % Receive single packet through udpH connection
    rxPkt   = udpReceive(udpH);
    
    % Check if anything received
    if ~isempty(rxPkt),
        fprintf('Received: %s \n',char(rxPkt));
    else
        fprintf('Nothing received (timed-out)\n');
    end
end

% Close udpH connection
udpClose(udpH);
```