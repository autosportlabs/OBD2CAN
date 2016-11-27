setTickRate(10)

--0=no logging; 1=info logging; 2=trace logging
log_level=2

--0=no reset; 1 = reset
do_reset=1

--0=auto detect; 1=J1850 PWM; 2=J1850 VPW; 3=ISO 9141-2; 4=ISO 14230-4
protocol=0

res = txCAN(0, 62344, 1, {1, log_level, do_reset,protocol})
println('txCAN reset rc: ' ..res)

sleep(2000)

function doVIN()

res = txCAN(0, 2015, 0, {0x02, 0x09, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55})
println('VIN request: ' ..res)

end

function onTick() 
	doVIN()
    sleep(2000)
	repeat 
		id, ext, data = rxCAN(0, 100)
		if id ~= nil then
			print(id ..', ')
			for i = 1,#data do
				print(data[i] ..', ')
			end
			println('')
		end
	until id == nil
end



