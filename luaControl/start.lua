return {
    server = {
        executable = "D:\\C++learning\\HJMadventrue++\\x64\\Debug\\server.exe",
        working_dir = "D:\\C++learning\\HJMadventrue++\\server\\",  -- ��ӷ���������Ŀ¼
        config_file = "text.txt",  -- �����������ļ����·��
        port = 25565,
        args = {"--max-connections=100"}
    },
    
    clients = {
        executable = "D:\\C++learning\\HJMadventrue++\\x64\\Debug\\client.exe",
        working_dir = "D:\\C++learning\\HJMadventrue++\\client\\",  -- ��ӿͻ��˹���Ŀ¼
        config_file = "config.cfg",  -- �ͻ��������ļ����·��
        count = 2,
        args = {"--name=Client_"}
    },
    
    delay = {
        server_start = 1.0,
        between_clients = 0.5
    }
}