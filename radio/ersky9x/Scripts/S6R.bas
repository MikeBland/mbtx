array byte configRead[2]
array byte settingsRead[24]
array byte configAddress[2]
array byte settingsAddress[25]
array byte configValue[2]
array byte settingsValue[24]
if init = 0
 init = 1
 refreshIndex = 0
 pageOffset = 0
 page = 1
 current = 0
 refreshState = 0
 fields = 2
 fieldlist = 0
 configAddress[0] = 0x80
 configAddress[1] = 0x81
 type = 0
 combotype = 0
 min = 0
 max = 1
 value = 0
 sportfield = 0
 result = 0
 attr = 0
 modified = 0
 telemetryPopTimeout = 0
 strtoarray( settingsAddress[0], "\x9C\xAA\xA8\xA9\x82\x83\x84\x9A\x9B\x85\x86\x87\x88\x89\x8C\x8D\x8E\x90\x91\x92\x95\x96\x97\x99" )
end

goto main


redrawFieldsPage:
drawclear()
drawtext( 0, 0, "SXR", INVERS )
if refreshIndex < fields then gosub drawProgressBar

index = 1
drawloop:

 selected = pageOffset+index-1
 if selected >= fields then goto drawdone
 if page = 1
  if selected > 1 then goto drawdone
  valueread = configRead[selected]
 else
  if selected > 23 then goto drawdone
  valueread = settingsRead[selected]
 end
 gosub showname
 attr = 0
 if selected = current
  attr = INVERS
  if edit then attr = BLINK
 end
 if valueread = 0
  drawtext( 109, 8*index, "---" )
 else
  gosub getType
  valueindex = selected
  gosub getvalue
  if type = 0
   drawnumber( 127, 8*index, value, attr )
  else
   gosub getcombotype
   if combotype = 0
    gosub showwingtype
   elseif combotype = 1
    gosub showmounttype
   elseif combotype = 2
    if value = 0
     drawtext( 103, 8*index, "AIL2", attr )
    else
     drawtext( 103, 8*index, "AUX1", attr )
    end
   elseif combotype = 3
    if value = 0
     drawtext( 103, 8*index, "ELE2", attr )
    else
     drawtext( 103, 8*index, "AUX2", attr )
    end
   elseif combotype = 4
    gosub showenable
   elseif combotype = 5
    gosub showinverse
   end
  end
 end

index += 1
if index < 8 then goto drawloop
drawdone:
rem drawnumber( 50, 0, sportfield )
rem drawnumber( 80, 0, refreshIndex )
return

drawProgressBar:
 pbwidth = (68 * refreshIndex) / fields
 drawrectangle(30, 1, 70, 6)
 if pbwidth
  drawline(31, 3, 31+pbwidth, 3 )
  drawline(31, 4, 31+pbwidth, 4 )
 end
return

showname:
if fieldlist = 0
 if selected = 0
  drawtext(0, 8*index, "Wing type:" )
 else
  drawtext(0, 8*index, "Mounting type:" )
 end
else
 if selected < 8
  if selected = 0 then drawtext(0, 8*index, "SXfunctions" )
  if selected = 1 then drawtext(0, 8*index, "Quick Mode:" )
  if selected = 2 then drawtext(0, 8*index, "CH5 mode" )
  if selected = 3 then drawtext(0, 8*index, "CH6 mode" )
  if selected = 4 then drawtext(0, 8*index, "AIL direction:" )
  if selected = 5 then drawtext(0, 8*index, "ELE direction:" )
  if selected = 6 then drawtext(0, 8*index, "RUD direction" )
  if selected = 7 then drawtext(0, 8*index, "AIL2 direction" )
 elseif selected < 16
  if selected = 8 then drawtext(0, 8*index, "ELE2 direction" )
  if selected = 9 then drawtext(0, 8*index, "AIL stab gain" )
  if selected = 10 then drawtext(0, 8*index, "ELE stab gain" )
  if selected = 11 then drawtext(0, 8*index, "RUD stab gain" )
  if selected = 12 then drawtext(0, 8*index, "AIL autolvl gain" )
  if selected = 13 then drawtext(0, 8*index, "ELE autolvl gain" )
  if selected = 14 then drawtext(0, 8*index, "ELE hover gain" )
  if selected = 15 then drawtext(0, 8*index, "RUD hover gain" )
 else
  if selected = 16 then drawtext(0, 8*index, "AIL knife gain" )
  if selected = 17 then drawtext(0, 8*index, "RUD knife gain" )
  if selected = 18 then drawtext(0, 8*index, "AIL autolvl offset" )
  if selected = 19 then drawtext(0, 8*index, "ELE autolvl offset" )
  if selected = 20 then drawtext(0, 8*index, "ELE hover offset" )
  if selected = 21 then drawtext(0, 8*index, "RUD hover offset" )
  if selected = 22 then drawtext(0, 8*index, "AIL knife offset" )
  if selected = 23 then drawtext(0, 8*index, "RUD knife offset" )
 end
end
return

getcombotype:
if fieldlist = 0
 if valueindex = 0
  combotype = 0
 else
  combotype = 1
 end	
else
 if valueindex < 2
  combotype = 4
 elseif valueindex < 3
  combotype = 2
 elseif valueindex < 4
  combotype = 3
 elseif valueindex < 9
  combotype = 5
 end	
end
return

getType:
if fieldlist = 0
 type = 1
else
 if selected < 9
  type = 1
 else
 	type = 0
 end
end
return

getvalue:
if fieldlist = 0
 value = configValue[valueindex]
else
 value = settingsValue[valueindex]
end
return

setvalue:
if fieldlist = 0
 configValue[current] = value
else
 settingsValue[current] = value
end
return

getminmax:
if fieldlist = 0
 min = 0
 if current = 0
  max = 2
 else
 	max = 3
 end
else
 if current < 9
  min = 0
  max = 1
 elseif current < 18
  min = 0
  max = 200
 else
  min = -20
  max = 20
 end
end
return

showwingtype:
if value = 0
 drawtext( 91, 8*index, "Normal", attr )
elseif value = 1
 drawtext( 97, 8*index, "Delta", attr )
else
 drawtext( 97, 8*index, "VTail", attr )
end
return

showmounttype:
if value = 0
 drawtext( 103, 8*index, "Horz", attr )
elseif value = 1
 drawtext( 85, 8*index, "Hor rev", attr )
elseif value = 2
 drawtext( 103, 8*index, "Vert", attr )
else
 drawtext( 85, 8*index, "Ver rev", attr )
end
return

showenable:
if value = 0
 drawtext( 85, 8*index, "Disable", attr )
else
 drawtext( 91, 8*index, "Enable", attr )
end
return

showinverse:
if value = 0
 drawtext( 91, 8*index, "Normal", attr )
else
 drawtext( 91, 8*index, "Invers", attr )
end
return

incField:
gosub getminmax
valueindex = current
gosub getvalue
value += 1
if value <= max then gosub setvalue
return

decField:
gosub getminmax
valueindex = current
gosub getvalue
value -= 1
if value >= min then gosub setvalue
return

upField:
if current
 current -= 1
else
 current = fields - 1
end
if current > 6 + pageOffset
 pageOffset = current - 6
elseif current <= pageOffset
 pageOffset = current
end	
return

downField:
current += 1
if current >= fields
 current = 0
end
if current > 6 + pageOffset
 pageOffset = current - 6
elseif current <= pageOffset
 pageOffset = current
end	
return

runFieldsPage:
if Event = EVT_MENU_BREAK 
rem toggle editing/selecting current field
 if page = 1
  valueread = configRead[current]
 else
  valueread = settingsRead[current]
 end
 if valueread
  if edit
   edit = 0
   modified = (page)*128 + current
  else
   edit = 1
  end
 end
else
 if edit
  if Event = EVT_RIGHT_FIRST then gosub incField
  if Event = EVT_LEFT_FIRST then gosub decField
 else
  if Event = EVT_UP_FIRST then gosub upField
  if Event = EVT_DOWN_FIRST then gosub downField
 end
end

pageError = 0
if pageOffset < 0 then pageError = 1
if pageOffset > 25 then pageError = 1
if pageError
 drawtext( 0, 48 "pageError" )
 drawnumber( 100, 48, pageOffset )
else
 gosub redrawFieldsPage
end
return

runconfigpage:
 fieldlist = 0
 fields = 2
 gosub runFieldsPage
 if configRead[1]
  value = configValue[1]
  drawtext(1, 32, "Pins toward tail")
	if value = 0 then drawtext(1, 40, "Label is facing the\037sky" )
	if value = 1 then drawtext(1, 40, "Label is facing\037ground" )
	if value = 2 then drawtext(1, 40, "Label is left when" )
	if value = 3 then drawtext(1, 40, "Label is right when" )
  if value > 1 then drawtext(1, 48, "looking from the tail")
 end
end

return

runsettingspage:
 fieldlist = 1
 fields = 24
 gosub runFieldsPage
return

runpage:
 if page = 1
  gosub runconfigpage
 else
  gosub runsettingspage
 end
return

refresh:
 if refreshState = 0
rem write changed data here
  if modified
   sportwrite = modified & 0x1F
   if modified >= 256
    writevalue = settingsValue[sportwrite]
    sportwrite = settingsAddress[sportwrite]
   else
    writevalue = configValue[sportwrite]
    sportwrite = configAddress[sportwrite]
   end
   sportTelemetrySend( 0x17, 0x31, 0x0C30, sportwrite + writevalue*256)
	 modified = 0
  end
  if refreshIndex < fields
   if fieldlist = 0
    sportfield = configAddress[refreshIndex]
   else
    sportfield = settingsAddress[refreshIndex]
   end
   result = sportTelemetrySend( 0x17, 0x30, 0x0C30, sportfield)
rem   gosub telemetryRead
   if result
    refreshState = 1
    telemetryPopTimeout = gettime() + 80
   end
  end
 elseif refreshState = 1
 	result = sportTelemetryReceive( physicalId, primId, dataId, value )
	cresult = result
  if result > 0
   if physicalId = 0x1A 
    if primId = 0x32
     if dataId = 0x0C30
      result = 2
      fieldId = value & 0x00FF
			if sportfield = fieldId
       value /= 256
       if fieldlist = 0
        configValue[refreshIndex] = value
        configRead[refreshIndex] = 1
       else
        if refreshIndex > 3
         if refreshIndex < 9
          if value = 0 then value = 1
          if value = 255 then value = 0
         elseif refreshIndex > 17
          value -= 128
         end
        end
        settingsValue[refreshIndex] = value
        settingsRead[refreshIndex] = 1
       end
       refreshIndex += 1
      end
      refreshState = 0
     end
    end
   end
  end
  if result # 2
   if gettime() > telemetryPopTimeout
    refreshState = 0
		retries += 1
		if retries > 3
		 retries = 0
     refreshIndex += 1
    end
   end
	end
 end
return

main:
 drawclear()
 if Event = EVT_EXIT_BREAK then goto done
 if Event = EVT_MENU_LONG
  killevents(Event)
  page += 1 ;
  if page > 2 then page = 1
  current = 0
	pageOffset = 0
  refreshIndex = 0
	retries = 0
 end
 gosub runpage
 gosub refresh

stop
done:
finish



