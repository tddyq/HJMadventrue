return {
    server = {
        executable = "D:\\C++learning\\HJMadventrue++\\x64\\Debug\\server.exe",
        working_dir = "D:\\C++learning\\HJMadventrue++\\server\\",  -- 添加服务器工作目录
        config_file = "text.txt",  -- 服务器配置文件相对路径
        port = 25565,
        args = {"--max-connections=100"}
    },
    
    clients = {
        executable = "D:\\C++learning\\HJMadventrue++\\x64\\Debug\\client.exe",
        working_dir = "D:\\C++learning\\HJMadventrue++\\client\\",  -- 添加客户端工作目录
        config_file = "config.cfg",  -- 客户端配置文件相对路径
        count = 2,
        args = {"--name=Client_"}
    },
    
    delay = {
        server_start = 1.0,
        between_clients = 0.5
    }
}