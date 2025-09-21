
progressArraySize = 0
playerNumOfRound = 0
numTotalChar = 0
isGameOver = false
winnerId = -1
filename = ""

player_progress = {}
player_rounds = {}

function getTotalCharsFromFile(fileName)
	filename = fileName or "text.txt"

    if not filename or type(filename)~="string" then
        return nil,"Invalid filename".. tostring(filename)
    end

    local file,err = io.open(filename,"r")
    if not file then 
        return nil,"Cannot open file:"..(err or "unknown error")
    end

    local content = file:read("*a")
    if not content then
        file:close()
        return nil, "Failed to read content from file"
    end
    file:close()

    return #content
end

function init(size , round)
    progressArraySize = size or 2
    playerNumOfRound = round or 1
    
    -- 读取文件获取总字符数
    local charCount, err = getTotalCharsFromFile("text.txt")
    if err then
        print("Error: " .. err)
        numTotalChar = 100 -- 设置默认值
    else
        numTotalChar = charCount
    end

	for i = 1,progressArraySize do
        player_progress[i] = -1
        player_rounds[i] = 0
    end

    isGameOver = false
    winnerId = -1
end




function updateProcess( player_id, new_progress )
	player_id = player_id + 1
    if(new_progress >= player_progress[player_id]) then 
        player_progress[player_id] = new_progress
    end

    isInNewRound = false

    if(new_progress >= numTotalChar) then
        player_rounds[player_id] =  player_rounds[player_id] + 1
        player_progress[player_id] = new_progress - numTotalChar

        isInNewRound = true
    end

    if(player_rounds[player_id] == playerNumOfRound) then
        winnerId = player_id
        isGameOver = true
        --player_progress
        print("winner is " .. player_id .. " progressArraySize: " .. progressArraySize .. " playerNumOfRound: " .. playerNumOfRound .. " numTotalChar: " .. numTotalChar .. " isGameOver: " .. isGameOver)

    end
    return  winnerId,isGameOver, isInNewRound, player_progress[player_id]
end



function getGameMessage(  )
	return winnerId,isGameOver
end




