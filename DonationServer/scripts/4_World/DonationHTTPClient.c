// DonationServer/scripts/4_World/DonationServer/DonationHTTPClient.c

class DonationHTTPCallback: RestCallback
{
    private string m_Type;
    private PlayerIdentity m_Identity;
    private int m_PurchaseID;
    
    void DonationHTTPCallback(string type, PlayerIdentity identity, int purchaseID = 0)
    {
        m_Type = type;
        m_Identity = identity;
        m_PurchaseID = purchaseID;
    }
    
    private PlayerBase GetPlayerObject()
    {
        if (!m_Identity) 
            return NULL;
        return PlayerBase.Cast(m_Identity.GetPlayer());
    }

    void OnSuccess(string data, int dataSize)
    {
        DonationConfig.Log("HTTP Success: " + m_Type);
        DonationConfig.Log("Response: " + data);
        
        if (m_Type == "VAULT")
        {
            HandleVault(data);
        }
        else if (m_Type == "CLAIM")
        {
            HandleClaim(data);
        }
        else if (m_Type == "HISTORY")
        {
            HandleHistory(data);
        }
    }
    
    void OnError(int errorCode)
    {
        Error("[DonationServer] HTTP Error: " + errorCode);
        PlayerBase player = GetPlayerObject();
        
        if (m_Type == "CLAIM" && player)
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Write(false);
            rpc.Write("Server connection error: " + errorCode);
            rpc.Send(player, DS_RPC.CLAIM_RESULT, true, m_Identity);
        }
    }
    
    void OnTimeout()
    {
        Error("[DonationServer] HTTP Timeout");
        PlayerBase player = GetPlayerObject();
        
        if (m_Type == "CLAIM" && player)
        {
            ScriptRPC rpc = new ScriptRPC();
            rpc.Write(false);
            rpc.Write("Connection timeout");
            rpc.Send(player, DS_RPC.CLAIM_RESULT, true, m_Identity);
        }
    }
    
    private void HandleVault(string json)
    {
        PlayerBase player = GetPlayerObject();
        if (!player) 
            return;
        
        array<ref DS_VaultItem> items = new array<ref DS_VaultItem>;
        ParseVaultItems(json, items);
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(items.Count());
        
        int i;
        for (i = 0; i < items.Count(); i++)
        {
            rpc.Write(items[i].id);
            rpc.Write(items[i].classname);
        }
        
        rpc.Send(player, DS_RPC.SEND_VAULT, true, m_Identity);
        DonationConfig.Log("Sent " + items.Count() + " vault items to " + m_Identity.GetName());
    }
    
    private void HandleClaim(string json)
    {
        PlayerBase player = GetPlayerObject();
        if (!player) 
            return;

        bool apiSuccess = (json.IndexOf("\"success\":true") != -1);
        
        if (apiSuccess)
        {
            string classname = ExtractValue(json, "item_classname");
            int pID = ExtractIntValue(json, "id");
            
            if (classname == "")
            {
                ScriptRPC rpcErr = new ScriptRPC();
                rpcErr.Write(false);
                rpcErr.Write("Invalid item classname");
                rpcErr.Send(player, DS_RPC.CLAIM_RESULT, true, m_Identity);
                return;
            }

            string errorMsg;
            bool giveResult = DonationItemSpawner.GiveItemToPlayer(player, classname, errorMsg);
            
            if (giveResult)
            {
                // ИСПРАВЛЕНО: Используем GetPlainId() вместо GetId()
                string steamId = m_Identity.GetPlainId();
                string confirmUrl = DonationConfig.BuildURL(DonationConfig.EP_CONFIRM_ITEM, steamId, pID);
                
                // Отправляем подтверждение выдачи
                GetRestApi().GetRestContext(confirmUrl).GET(NULL, "");
                
                DonationConfig.Log("Item spawned successfully. Confirmation sent: " + confirmUrl);
                
                // Отправляем RPC игроку
                ScriptRPC rpcSuccess = new ScriptRPC();
                rpcSuccess.Write(true);
                rpcSuccess.Write("Item received!");
                rpcSuccess.Send(player, DS_RPC.CLAIM_RESULT, true, m_Identity);
            }
            else
            {
                // Спавн не удался - отправляем ошибку, статус остается vault
                DonationConfig.Log("Item spawn failed: " + errorMsg);
                
                ScriptRPC rpcFail = new ScriptRPC();
                rpcFail.Write(false);
                rpcFail.Write(errorMsg);
                rpcFail.Send(player, DS_RPC.CLAIM_RESULT, true, m_Identity);
            }
        }
        else
        {
            string errMsg = ExtractValue(json, "error");
            if (errMsg == "") 
                errMsg = "API error";
            
            ScriptRPC rpcApiError = new ScriptRPC();
            rpcApiError.Write(false);
            rpcApiError.Write(errMsg);
            rpcApiError.Send(player, DS_RPC.CLAIM_RESULT, true, m_Identity);
        }
    }
    
    private void HandleHistory(string json)
    {
        PlayerBase player = GetPlayerObject();
        if (!player) 
            return;

        array<ref DS_HistoryItem> items = new array<ref DS_HistoryItem>;
        ParseHistoryItems(json, items);
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(items.Count());
        
        int i;
        for (i = 0; i < items.Count(); i++)
        {
            rpc.Write(items[i].id);
            rpc.Write(items[i].classname);
            rpc.Write(items[i].date_claimed);
        }
        
        rpc.Send(player, DS_RPC.SEND_HISTORY, true, m_Identity);
        DonationConfig.Log("Sent " + items.Count() + " history items");
    }
    
    private void ParseVaultItems(string json, out array<ref DS_VaultItem> items)
    {
        int start = json.IndexOf("\"items\":[");
        if (start == -1) 
            return;
        
        string afterStart = json.Substring(start, json.Length() - start);
        int endLocal = afterStart.IndexOf("]");
        if (endLocal == -1) 
            return;
        
        string itemsStr = afterStart.Substring(9, endLocal - 9);
        array<string> objects = new array<string>;
        SplitJsonObjects(itemsStr, objects);
        
        int i;
        for (i = 0; i < objects.Count(); i++)
        {
            string obj = objects.Get(i);
            int id = ExtractIntValue(obj, "id");
            string classname = ExtractValue(obj, "item_classname");
            
            if (id > 0 && classname != "")
            {
                items.Insert(new DS_VaultItem(id, classname));
            }
        }
    }
    
    private void ParseHistoryItems(string json, out array<ref DS_HistoryItem> items)
    {
        int start = json.IndexOf("\"items\":[");
        if (start == -1) 
            return;
        
        string afterStart = json.Substring(start, json.Length() - start);
        int endLocal = afterStart.IndexOf("]");
        if (endLocal == -1) 
            return;
        
        string itemsStr = afterStart.Substring(9, endLocal - 9);
        array<string> objects = new array<string>;
        SplitJsonObjects(itemsStr, objects);
        
        int i;
        for (i = 0; i < objects.Count(); i++)
        {
            string obj = objects.Get(i);
            int id = ExtractIntValue(obj, "id");
            string classname = ExtractValue(obj, "item_classname");
            string date = ExtractValue(obj, "date_formatted");
            
            if (date == "") 
                date = ExtractValue(obj, "date");
                
            if (id > 0 && classname != "")
            {
                items.Insert(new DS_HistoryItem(id, classname, date));
            }
        }
    }
    
    private void SplitJsonObjects(string arr, out array<string> objects)
    {
        int depth = 0;
        int start = -1;
        int i;
        
        for (i = 0; i < arr.Length(); i++)
        {
            string c = arr.Get(i);
            if (c == "{")
            {
                if (depth == 0) 
                    start = i;
                depth++;
            }
            else if (c == "}")
            {
                depth--;
                if (depth == 0 && start != -1)
                {
                    objects.Insert(arr.Substring(start, i - start + 1));
                    start = -1;
                }
            }
        }
    }
    
    private string ExtractValue(string json, string key)
    {
        string search = "\"" + key + "\":\"";
        int pos = json.IndexOf(search);
        if (pos == -1)
        {
            search = "\"" + key + "\": \"";
            pos = json.IndexOf(search);
        }
        if (pos == -1) 
            return "";
            
        int start = pos + search.Length();
        string afterKey = json.Substring(start, json.Length() - start);
        int end = afterKey.IndexOf("\"");
        
        if (end == -1)
            return "";
            
        return afterKey.Substring(0, end);
    }
    
    private int ExtractIntValue(string json, string key)
    {
        string search = "\"" + key + "\":";
        int pos = json.IndexOf(search);
        if (pos == -1)
        {
            search = "\"" + key + "\": ";
            pos = json.IndexOf(search);
        }
        if (pos == -1) 
            return 0;
            
        int start = pos + search.Length();
        int end = start;
        
        while (end < json.Length())
        {
            string c = json.Get(end);
            if (c == "," || c == "}" || c == "]") 
                break;
            end++;
        }
        
        string val = json.Substring(start, end - start);
        val.Trim();
        return val.ToInt();
    }
}

class DonationHTTPClient
{
    static void PendingToVault(string steam64, PlayerIdentity identity)
    {
        string url = DonationConfig.BuildURL(DonationConfig.EP_PENDING_TO_VAULT, steam64);
        DonationConfig.Log("===========================================");
        DonationConfig.Log("PendingToVault called!");
        DonationConfig.Log("Steam64: " + steam64);
        DonationConfig.Log("Full URL: " + url);
        DonationConfig.Log("===========================================");
        
        DonationHTTPCallback cb = new DonationHTTPCallback("VAULT", identity);
        RestContext ctx = GetRestApi().GetRestContext(url);
        ctx.SetHeader("application/json");
        ctx.GET(cb, "");
        
        DonationConfig.Log("HTTP GET request sent!");
    }
    
    static void ClaimItem(int purchase_id, string steam64, PlayerIdentity identity)
    {
        string url = DonationConfig.BuildURL(DonationConfig.EP_CLAIM_ITEM, steam64, purchase_id);
        DonationConfig.Log("ClaimItem: " + purchase_id);
        DonationConfig.Log("URL: " + url);
        
        DonationHTTPCallback cb = new DonationHTTPCallback("CLAIM", identity, purchase_id);
        RestContext ctx = GetRestApi().GetRestContext(url);
        ctx.SetHeader("application/json");
        ctx.POST(cb, "", "");
    }
    
    static void GetHistory(string steam64, PlayerIdentity identity)
    {
        string url = DonationConfig.BuildURL(DonationConfig.EP_GET_HISTORY, steam64);
        DonationConfig.Log("GetHistory: " + steam64);
        DonationConfig.Log("URL: " + url);
        
        DonationHTTPCallback cb = new DonationHTTPCallback("HISTORY", identity);
        RestContext ctx = GetRestApi().GetRestContext(url);
        ctx.SetHeader("application/json");
        ctx.GET(cb, "");
    }
}