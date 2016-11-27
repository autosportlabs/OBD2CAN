setTickRate(10)

--0=no logging; 1=info logging; 2=trace logging
log_level=2

--0=no reset; 1 = reset
do_reset=1

--0=auto detect; 1=J1850 PWM; 2=J1850 VPW; 3=ISO 9141-2; 4=ISO 14230-4
protocol=0

res = txCAN(0, 62344, 1, {1, log_level, do_reset,protocol})
println('txCAN reset rc: ' ..res)

mode = 0x01
pid = 0x0C

function onTick()

txCAN(0, 2015, 0, {0x02, mode, pid, 0x55, 0x55, 0x55, 0x55, 0x55})

while true do
  id, ext, data = rxCAN(0, 1)
  if id == nil then break end
  if id == 2024 then outputPidResponse(data) ready = true end
  if id == 62345 then outputDiag(data) end
end
end

function outputPidResponse(data)
--  println(id ..' ' ..data[1] ..' ' ..data[2] ..' ' ..data[3])
end

function outputDiag(data)
  println('{"protocol":' ..data[1] ..', "last_error":' ..data[2] ..', "latency":' ..data[3] + data[4] * 256 ..', "ver":"' ..data[6] ..',' ..data[7] ..',' ..data[8] ..'"}')
end


