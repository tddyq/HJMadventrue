local config = dofile("start.lua")

print("========================================")
print("Lua Control Center")
print("========================================")
print(string.format("Starting server on port %d", config.server.port))
print(string.format("Launching %d clients", config.clients.count))
print("----------------------------------------")

-- 确保路径格式正确（Windows使用反斜杠）
local function normalize_path(path)
    return path:gsub("/", "\\")
end

-- 检查文件是否存在
local function verify_file_exists(path, filename)
    local full_path = path .. filename
    local file = io.open(full_path, "r")
    if file then
        file:close()
        return true, full_path
    else
        print(string.format("[ERROR] 文件不存在: %s", full_path))
        return false, full_path
    end
end

-- 启动服务器
local function start_server()
    local exe_path = normalize_path(config.server.executable)
    local working_dir = normalize_path(config.server.working_dir)
    
    -- 检查可执行文件是否存在
    local exe_exists = verify_file_exists("", exe_path)
    if not exe_exists then
        print("[SERVER] 启动失败: 可执行文件不存在")
        return
    end
    
    -- 检查配置文件是否存在
    local config_exists, config_path = verify_file_exists(working_dir, config.server.config_file)
    if not config_exists then
        print("[SERVER] 启动失败: 配置文件不存在")
        return
    end
    
    -- 构建参数列表
    local args = {
        "--port=" .. config.server.port,
        table.unpack(config.server.args)
    }
    
    -- 使用正确的命令格式设置工作目录
    local cmd = string.format('start "" /B /D "%s" "%s" %s',
        working_dir,  -- 设置工作目录
        exe_path,
        table.concat(args, " ")
    )
    
    print("[SERVER] Starting: " .. cmd)
    print(string.format("[SERVER] 工作目录: %s", working_dir))
    print(string.format("[SERVER] 配置文件: %s", config_path))
    os.execute(cmd)
    print(string.format("[SERVER] Started"))
end

-- 启动客户端
local function start_client(client_id)
    local exe_path = normalize_path(config.clients.executable)
    local working_dir = normalize_path(config.clients.working_dir)
    
    -- 检查可执行文件是否存在
    local exe_exists = verify_file_exists("", exe_path)
    if not exe_exists then
        print(string.format("[CLIENT %d] 启动失败: 可执行文件不存在", client_id))
        return
    end
    
    -- 检查配置文件是否存在
    local config_exists, config_path = verify_file_exists(working_dir, config.clients.config_file)
    if not config_exists then
        print(string.format("[CLIENT %d] 启动失败: 配置文件不存在", client_id))
        return
    end
    
    -- 构建参数列表
    local args = {
        "--id=" .. client_id,
        "--server-port=" .. config.server.port,
        table.unpack(config.clients.args)
    }
    
    -- 使用正确的命令格式设置工作目录
    local cmd = string.format('start "" /B /D "%s" "%s" %s',
        working_dir,  -- 设置工作目录
        exe_path,
        table.concat(args, " ")
    )
    
    print(string.format("[CLIENT %d] Starting: %s", client_id, cmd))
    print(string.format("[CLIENT %d] 工作目录: %s", client_id, working_dir))
    print(string.format("[CLIENT %d] 配置文件: %s", client_id, config_path))
    os.execute(cmd)
    print(string.format("[CLIENT %d] Started", client_id))
end

-- 延迟函数
local function sleep(seconds)
    local start = os.clock()
    while os.clock() - start < seconds do end
end

-- 主控制流程
start_server()
sleep(config.delay.server_start)

for i = 1, config.clients.count do
    start_client(i)
    sleep(config.delay.between_clients)
end

print("----------------------------------------")
print("All processes started successfully!")
print("========================================")