local config = dofile("start.lua")

print("========================================")
print("Lua Control Center")
print("========================================")
print(string.format("Starting server on port %d", config.server.port))
print(string.format("Launching %d clients", config.clients.count))
print("----------------------------------------")

-- ȷ��·����ʽ��ȷ��Windowsʹ�÷�б�ܣ�
local function normalize_path(path)
    return path:gsub("/", "\\")
end

-- ����ļ��Ƿ����
local function verify_file_exists(path, filename)
    local full_path = path .. filename
    local file = io.open(full_path, "r")
    if file then
        file:close()
        return true, full_path
    else
        print(string.format("[ERROR] �ļ�������: %s", full_path))
        return false, full_path
    end
end

-- ����������
local function start_server()
    local exe_path = normalize_path(config.server.executable)
    local working_dir = normalize_path(config.server.working_dir)
    
    -- ����ִ���ļ��Ƿ����
    local exe_exists = verify_file_exists("", exe_path)
    if not exe_exists then
        print("[SERVER] ����ʧ��: ��ִ���ļ�������")
        return
    end
    
    -- ��������ļ��Ƿ����
    local config_exists, config_path = verify_file_exists(working_dir, config.server.config_file)
    if not config_exists then
        print("[SERVER] ����ʧ��: �����ļ�������")
        return
    end
    
    -- ���������б�
    local args = {
        "--port=" .. config.server.port,
        table.unpack(config.server.args)
    }
    
    -- ʹ����ȷ�������ʽ���ù���Ŀ¼
    local cmd = string.format('start "" /B /D "%s" "%s" %s',
        working_dir,  -- ���ù���Ŀ¼
        exe_path,
        table.concat(args, " ")
    )
    
    print("[SERVER] Starting: " .. cmd)
    print(string.format("[SERVER] ����Ŀ¼: %s", working_dir))
    print(string.format("[SERVER] �����ļ�: %s", config_path))
    os.execute(cmd)
    print(string.format("[SERVER] Started"))
end

-- �����ͻ���
local function start_client(client_id)
    local exe_path = normalize_path(config.clients.executable)
    local working_dir = normalize_path(config.clients.working_dir)
    
    -- ����ִ���ļ��Ƿ����
    local exe_exists = verify_file_exists("", exe_path)
    if not exe_exists then
        print(string.format("[CLIENT %d] ����ʧ��: ��ִ���ļ�������", client_id))
        return
    end
    
    -- ��������ļ��Ƿ����
    local config_exists, config_path = verify_file_exists(working_dir, config.clients.config_file)
    if not config_exists then
        print(string.format("[CLIENT %d] ����ʧ��: �����ļ�������", client_id))
        return
    end
    
    -- ���������б�
    local args = {
        "--id=" .. client_id,
        "--server-port=" .. config.server.port,
        table.unpack(config.clients.args)
    }
    
    -- ʹ����ȷ�������ʽ���ù���Ŀ¼
    local cmd = string.format('start "" /B /D "%s" "%s" %s',
        working_dir,  -- ���ù���Ŀ¼
        exe_path,
        table.concat(args, " ")
    )
    
    print(string.format("[CLIENT %d] Starting: %s", client_id, cmd))
    print(string.format("[CLIENT %d] ����Ŀ¼: %s", client_id, working_dir))
    print(string.format("[CLIENT %d] �����ļ�: %s", client_id, config_path))
    os.execute(cmd)
    print(string.format("[CLIENT %d] Started", client_id))
end

-- �ӳٺ���
local function sleep(seconds)
    local start = os.clock()
    while os.clock() - start < seconds do end
end

-- ����������
start_server()
sleep(config.delay.server_start)

for i = 1, config.clients.count do
    start_client(i)
    sleep(config.delay.between_clients)
end

print("----------------------------------------")
print("All processes started successfully!")
print("========================================")