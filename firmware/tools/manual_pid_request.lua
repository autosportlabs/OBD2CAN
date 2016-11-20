setTickRate(10)
ready = true
function onTick() 



--println('tick')

if ready==true then
txCAN(0, 2015, 0, {0x02, 0x01, 0x0C, 0x55, 0x55, 0x55, 0x55, 0x55})
ready = false
end

while true do
  id, ext, data = rxCAN(0, 1)
  if id == nil then break end
  println(id ..' ' ..data[1] ..' ' ..data[2] ..' ' ..data[3])
  if id == 2024 then ready = true end
end
end



