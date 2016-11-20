--0=no logging; 1=info logging; 2=trace logging
log_level=2


--0=no reset; 1 = reset
do_reset=1

--0=auto detect; 1=J1850 PWM; 2=J1850 VPW; 3=ISO 9141-2; 4=ISO 14230-4
protocol=0

rc = txCAN(0, 62344, 1, {1,log_level, do_reset,protocol,0})
println('txCAN rc: ' ..rc)

setTickRate(1)
function onTick()
id, ext, data = rxCAN(0,1)
if id ~= nil then outputCAN(id, ext, data) end
   
end

function outputCAN(id, ext, data)
print('blah')
  if id == 62345 then
    outputDiag(data) 
  else
    outputRaw(id, data)
  end
end

function outputDiag(data)
  println('protocol: ' ..data[1] ..': last error: ' ..data[2] ..': latency ' ..data[3] + data[4] * 256 ..': ver: ' ..data[6] ..' ' ..data[7] ..' ' ..data[8])
end

function outputRaw(id, data)
  println('RX CAN: ' ..id)
end
