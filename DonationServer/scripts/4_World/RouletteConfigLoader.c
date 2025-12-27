// DonationServer/scripts/4_World/Roulette/RouletteConfigLoader.c

class RouletteConfig
{
    int cooldown_seconds;
    ref array<ref RouletteItem> items;
    
    void RouletteConfig()
    {
        items = new array<ref RouletteItem>;
    }
}

class RouletteConfigData
{
    ref RouletteConfig free_roulette;
    ref RouletteConfig paid_roulette;
    
    void RouletteConfigData()
    {
        free_roulette = new RouletteConfig();
        paid_roulette = new RouletteConfig();
    }
}

class RouletteConfigLoader
{
    private static ref RouletteConfigData s_Config;
    private static const string CONFIG_PATH = "$profile:Donation/RouletteConfig.json";
    
    static RouletteConfigData LoadConfig()
    {
        Print("[Roulette] Loading config from: " + CONFIG_PATH);
        
        FileHandle file = OpenFile(CONFIG_PATH, FileMode.READ);
        
        if (file == 0)
        {
            Print("[Roulette] ERROR: Config file not found! Creating default...");
            CreateDefaultConfig();
            file = OpenFile(CONFIG_PATH, FileMode.READ);
            
            if (file == 0)
            {
                Print("[Roulette] ERROR: Failed to create default config!");
                return CreateEmptyConfig();
            }
        }
        
        CloseFile(file);
        
        s_Config = new RouletteConfigData();
        
        string jsonContent = LoadFileContent(CONFIG_PATH);
        
        if (jsonContent == "")
        {
            Print("[Roulette] ERROR: Config file is empty!");
            return CreateEmptyConfig();
        }
        
        ParseConfig(jsonContent);
        
        Print("[Roulette] Config loaded successfully");
        Print("[Roulette] Free roulette items: " + s_Config.free_roulette.items.Count());
        Print("[Roulette] Paid roulette items: " + s_Config.paid_roulette.items.Count());
        
        return s_Config;
    }
    
    static RouletteConfigData GetConfig()
    {
        if (!s_Config)
        {
            s_Config = LoadConfig();
        }
        return s_Config;
    }
    
    private static void ParseConfig(string json)
    {
        int freeStart = json.IndexOf("\"free_roulette\"");
        if (freeStart != -1)
        {
            string afterFree = json.Substring(freeStart, json.Length() - freeStart);
            int cooldownPos = afterFree.IndexOf("\"cooldown_seconds\"");
            
            if (cooldownPos != -1)
            {
                s_Config.free_roulette.cooldown_seconds = ExtractInt(afterFree, cooldownPos);
            }
            
            ParseItems(json, freeStart, s_Config.free_roulette.items);
        }
        
        int paidStart = json.IndexOf("\"paid_roulette\"");
        if (paidStart != -1)
        {
            ParseItems(json, paidStart, s_Config.paid_roulette.items);
        }
    }
    
    private static void ParseItems(string json, int startPos, array<ref RouletteItem> items)
    {
        string afterStart = json.Substring(startPos, json.Length() - startPos);
        int itemsStart = afterStart.IndexOf("\"items\":[");
        
        if (itemsStart == -1)
            return;
        
        string afterItems = afterStart.Substring(itemsStart, afterStart.Length() - itemsStart);
        int itemsEnd = afterItems.IndexOf("]");
        
        if (itemsEnd == -1)
            return;
        
        string itemsStr = afterItems.Substring(9, itemsEnd - 9);
        
        array<string> objects = new array<string>;
        SplitJsonObjects(itemsStr, objects);
        
        int i;
        for (i = 0; i < objects.Count(); i = i + 1)
        {
            string obj = objects.Get(i);
            
            string classname = ExtractStringValue(obj, "classname");
            string display_name = ExtractStringValue(obj, "display_name");
            string rarity = ExtractStringValue(obj, "rarity");
            int weight = ExtractIntValue(obj, "weight");
            
            if (classname != "")
            {
                items.Insert(new RouletteItem(classname, display_name, rarity, weight));
            }
        }
    }
    
    private static void SplitJsonObjects(string arr, array<string> objects)
    {
        int depth = 0;
        int start = -1;
        int i;
        
        for (i = 0; i < arr.Length(); i = i + 1)
        {
            string c = arr.Get(i);
            if (c == "{")
            {
                if (depth == 0)
                    start = i;
                depth = depth + 1;
            }
            else if (c == "}")
            {
                depth = depth - 1;
                if (depth == 0 && start != -1)
                {
                    objects.Insert(arr.Substring(start, i - start + 1));
                    start = -1;
                }
            }
        }
    }
    
    private static string ExtractStringValue(string json, string key)
    {
        string search = "\"" + key + "\":\"";
        int pos = json.IndexOf(search);
        
        if (pos == -1)
            return "";
        
        int start = pos + search.Length();
        string afterKey = json.Substring(start, json.Length() - start);
        int end = afterKey.IndexOf("\"");
        
        if (end == -1)
            return "";
        
        return afterKey.Substring(0, end);
    }
    
    private static int ExtractIntValue(string json, string key)
    {
        string search = "\"" + key + "\":";
        int pos = json.IndexOf(search);
        
        if (pos == -1)
            return 0;
        
        int start = pos + search.Length();
        int end = start;
        
        while (end < json.Length())
        {
            string c = json.Get(end);
            if (c == "," || c == "}")
                break;
            end = end + 1;
        }
        
        string val = json.Substring(start, end - start);
        val.Trim();
        return val.ToInt();
    }
    
    private static int ExtractInt(string json, int pos)
    {
        string afterPos = json.Substring(pos, json.Length() - pos);
        int colonIndex = afterPos.IndexOf(":");
        
        if (colonIndex == -1)
            return 0;
        
        int start = colonIndex + 1;
        int end = start;
        
        while (end < afterPos.Length())
        {
            string c = afterPos.Get(end);
            if (c == "," || c == "}")
                break;
            end = end + 1;
        }
        
        string val = afterPos.Substring(start, end - start);
        val.Trim();
        return val.ToInt();
    }
    
    private static string LoadFileContent(string path)
    {
        FileHandle file = OpenFile(path, FileMode.READ);
        if (file == 0)
            return "";
        
        string content = "";
        string line;
        
        while (FGets(file, line) > 0)
        {
            content = content + line;
        }
        
        CloseFile(file);
        return content;
    }
    
    private static RouletteConfigData CreateEmptyConfig()
    {
        RouletteConfigData config = new RouletteConfigData();
        config.free_roulette.cooldown_seconds = 14400;
        return config;
    }
    
    private static void CreateDefaultConfig()
    {
        Print("[Roulette] Creating default config...");
        
        if (!FileExist("$profile:Donation"))
        {
            MakeDirectory("$profile:Donation");
        }
        
        FileHandle file = OpenFile(CONFIG_PATH, FileMode.WRITE);
        if (file == 0)
        {
            Print("[Roulette] ERROR: Could not create config file!");
            return;
        }
        
        string defaultConfig = "{\n";
        defaultConfig = defaultConfig + "  \"free_roulette\": {\n";
        defaultConfig = defaultConfig + "    \"cooldown_seconds\": 14400,\n";
        defaultConfig = defaultConfig + "    \"items\": [\n";
        defaultConfig = defaultConfig + "      {\"classname\": \"Apple\", \"display_name\": \"Apple\", \"rarity\": \"common\", \"weight\": 100}\n";
        defaultConfig = defaultConfig + "    ]\n";
        defaultConfig = defaultConfig + "  },\n";
        defaultConfig = defaultConfig + "  \"paid_roulette\": {\n";
        defaultConfig = defaultConfig + "    \"cost_dollars\": 50,\n";
        defaultConfig = defaultConfig + "    \"items\": [\n";
        defaultConfig = defaultConfig + "      {\"classname\": \"AKM\", \"display_name\": \"AKM\", \"rarity\": \"rare\", \"weight\": 50}\n";
        defaultConfig = defaultConfig + "    ]\n";
        defaultConfig = defaultConfig + "  }\n";
        defaultConfig = defaultConfig + "}";
        
        FPrintln(file, defaultConfig);
        CloseFile(file);
        
        Print("[Roulette] Default config created");
    }
}