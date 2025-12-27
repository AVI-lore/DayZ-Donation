// DonationServer/scripts/4_World/Roulette/RouletteManager.c

class RouletteManager
{
    private static const string COOLDOWNS_PATH = "$profile:Donation/RouletteCooldowns.json";
    private static const string WINNINGS_PATH = "$profile:Donation/RouletteWinnings.json";
    
    private static ref map<string, string> s_Cooldowns;
    private static ref map<string, ref array<ref RouletteWinning>> s_Winnings;
    private static int s_NextWinningID = 1;
    
    static void Init()
    {
        Print("[Roulette] Initializing RouletteManager");
        
        s_Cooldowns = new map<string, string>;
        s_Winnings = new map<string, ref array<ref RouletteWinning>>;
        
        LoadCooldowns();
        LoadWinnings();
        RouletteConfigLoader.LoadConfig();
        
        Print("[Roulette] RouletteManager initialized");
    }
    
    static void HandleFreeSpin(PlayerIdentity identity)
    {
        string steam64 = identity.GetPlainId();
        PlayerBase player = PlayerBase.Cast(identity.GetPlayer());
        
        if (!player)
            return;
        
        if (!CanSpinFree(steam64))
        {
            int remaining = GetFreeCooldownRemaining(steam64);
            string error = "Free spin available in " + FormatTime(remaining);
            SendError(player, identity, error);
            return;
        }
        
        RouletteConfigData config = RouletteConfigLoader.GetConfig();
        RouletteItem prize = SelectRandomItem(config.free_roulette.items);
        
        if (!prize)
        {
            SendError(player, identity, "No prizes available");
            return;
        }
        
        UpdateFreeCooldown(steam64);
        AddWinning(steam64, prize, "free");
        
        array<ref RouletteItem> sequence = GenerateSpinSequence(config.free_roulette.items, prize);
        SendSpinResult(player, identity, sequence, prize);
    }
    
    static void HandlePaidSpin(PlayerIdentity identity)
    {
        string steam64 = identity.GetPlainId();
        PlayerBase player = PlayerBase.Cast(identity.GetPlayer());
        
        if (!player)
            return;
        
        int cost = 50;
        
        if (!RouletteMoneyHelper.HasEnoughMoney(player, cost))
        {
            int playerMoney = RouletteMoneyHelper.CountPlayerMoney(player);
            string error = "Not enough money. Need $" + cost + ", you have $" + playerMoney;
            SendError(player, identity, error);
            return;
        }
        
        string moneyError;
        if (!RouletteMoneyHelper.TakeMoney(player, cost, moneyError))
        {
            SendError(player, identity, moneyError);
            return;
        }
        
        RouletteConfigData config = RouletteConfigLoader.GetConfig();
        RouletteItem prize = SelectRandomItem(config.paid_roulette.items);
        
        if (!prize)
        {
            SendError(player, identity, "No prizes available");
            RouletteMoneyHelper.GiveChange(player, cost);
            return;
        }
        
        AddWinning(steam64, prize, "paid");
        
        array<ref RouletteItem> sequence = GenerateSpinSequence(config.paid_roulette.items, prize);
        SendSpinResult(player, identity, sequence, prize);
    }
    
    static void SendWinningsList(PlayerIdentity identity)
    {
        string steam64 = identity.GetPlainId();
        PlayerBase player = PlayerBase.Cast(identity.GetPlayer());
        
        if (!player)
            return;
        
        array<ref RouletteWinning> winnings = GetPlayerWinnings(steam64);
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(winnings.Count());
        
        int i;
        for (i = 0; i < winnings.Count(); i = i + 1)
        {
            RouletteWinning w = winnings[i];
            rpc.Write(w.id);
            rpc.Write(w.item_classname);
            rpc.Write(w.display_name);
            rpc.Write(w.roulette_type);
            rpc.Write(w.date);
        }
        
        rpc.Send(player, RouletteRPC.SEND_WINNINGS_LIST, true, identity);
    }
    
    static void ClaimWinning(int winningID, PlayerIdentity identity)
    {
        string steam64 = identity.GetPlainId();
        PlayerBase player = PlayerBase.Cast(identity.GetPlayer());
        
        if (!player)
            return;
        
        RouletteWinning winning = FindWinning(steam64, winningID);
        
        if (!winning)
        {
            SendClaimResult(player, identity, false, "Prize not found");
            return;
        }
        
        string error;
        bool result = DonationItemSpawner.GiveItemToPlayer(player, winning.item_classname, error);
        
        if (result)
        {
            RemoveWinning(steam64, winningID);
            SendClaimResult(player, identity, true, "Prize received");
        }
        else
        {
            SendClaimResult(player, identity, false, error);
        }
    }
    
    static void ClaimAllWinnings(PlayerIdentity identity)
    {
        string steam64 = identity.GetPlainId();
        PlayerBase player = PlayerBase.Cast(identity.GetPlayer());
        
        if (!player)
            return;
        
        array<ref RouletteWinning> winnings = GetPlayerWinnings(steam64);
        
        if (winnings.Count() == 0)
        {
            SendClaimResult(player, identity, false, "No prizes to claim");
            return;
        }
        
        int claimed = 0;
        int failed = 0;
        int i;
        
        for (i = winnings.Count() - 1; i >= 0; i = i - 1)
        {
            RouletteWinning w = winnings[i];
            string error;
            
            if (DonationItemSpawner.GiveItemToPlayer(player, w.item_classname, error))
            {
                RemoveWinning(steam64, w.id);
                claimed = claimed + 1;
            }
            else
            {
                failed = failed + 1;
            }
        }
        
        string message = "Claimed " + claimed + " prizes";
        SendClaimResult(player, identity, true, message);
    }
    
    static bool CanSpinFree(string steam64)
    {
        if (!s_Cooldowns.Contains(steam64))
            return true;
        
        string lastSpin = s_Cooldowns.Get(steam64);
        int lastSpinTime = ParseDateTime(lastSpin);
        int currentTime = CF_Date.Now(true).GetTimestamp();
        
        RouletteConfigData config = RouletteConfigLoader.GetConfig();
        int cooldown = config.free_roulette.cooldown_seconds;
        
        int diff = currentTime - lastSpinTime;
        return diff >= cooldown;
    }
    
    static int GetFreeCooldownRemaining(string steam64)
    {
        if (!s_Cooldowns.Contains(steam64))
            return 0;
        
        string lastSpin = s_Cooldowns.Get(steam64);
        int lastSpinTime = ParseDateTime(lastSpin);
        int currentTime = CF_Date.Now(true).GetTimestamp();
        
        RouletteConfigData config = RouletteConfigLoader.GetConfig();
        int cooldown = config.free_roulette.cooldown_seconds;
        
        int elapsed = currentTime - lastSpinTime;
        int remaining = cooldown - elapsed;
        
        if (remaining > 0)
            return remaining;
        
        return 0;
    }
    
    private static RouletteItem SelectRandomItem(array<ref RouletteItem> items)
    {
        if (items.Count() == 0)
            return NULL;
        
        int totalWeight = 0;
        int i;
        RouletteItem tempItem;
        
        for (i = 0; i < items.Count(); i = i + 1)
        {
            tempItem = items[i];
            totalWeight = totalWeight + tempItem.weight;
        }
        
        int random = Math.RandomInt(0, totalWeight);
        int currentWeight = 0;
        int j;
        
        for (j = 0; j < items.Count(); j = j + 1)
        {
            tempItem = items[j];
            currentWeight = currentWeight + tempItem.weight;
            if (random < currentWeight)
            {
                return tempItem;
            }
        }
        
        return items[0];
    }
    
    private static array<ref RouletteItem> GenerateSpinSequence(array<ref RouletteItem> pool, RouletteItem prize)
    {
        array<ref RouletteItem> sequence = new array<ref RouletteItem>;
        int i;
        
        for (i = 0; i < 24; i = i + 1)
        {
            sequence.Insert(SelectRandomItem(pool));
        }
        
        sequence.Insert(prize);
        return sequence;
    }
    
    private static void AddWinning(string steam64, RouletteItem item, string type)
    {
        if (!s_Winnings.Contains(steam64))
        {
            s_Winnings.Set(steam64, new array<ref RouletteWinning>);
        }
        
        array<ref RouletteWinning> winnings = s_Winnings.Get(steam64);
        string dateStr = CF_Date.Now(true).Format("dd.mm.yyyy HH:MM");
        
        int newID = s_NextWinningID;
        s_NextWinningID = newID + 1;
        
        RouletteWinning winning = new RouletteWinning(newID, item.classname, item.display_name, type, dateStr);
        
        winnings.Insert(winning);
        SaveWinnings();
    }
    
    private static array<ref RouletteWinning> GetPlayerWinnings(string steam64)
    {
        if (s_Winnings.Contains(steam64))
        {
            return s_Winnings.Get(steam64);
        }
        
        return new array<ref RouletteWinning>;
    }
    
    private static RouletteWinning FindWinning(string steam64, int id)
    {
        array<ref RouletteWinning> winnings = GetPlayerWinnings(steam64);
        int i;
        
        for (i = 0; i < winnings.Count(); i = i + 1)
        {
            RouletteWinning currentWinning = winnings[i];
            if (currentWinning.id == id)
            {
                return currentWinning;
            }
        }
        
        return NULL;
    }
    
    private static void RemoveWinning(string steam64, int id)
    {
        array<ref RouletteWinning> winnings = GetPlayerWinnings(steam64);
        int i;
        
        for (i = 0; i < winnings.Count(); i = i + 1)
        {
            RouletteWinning currentWinning = winnings[i];
            if (currentWinning.id == id)
            {
                winnings.Remove(i);
                SaveWinnings();
                return;
            }
        }
    }
    
    private static void UpdateFreeCooldown(string steam64)
    {
        string currentTime = CF_Date.Now(true).Format("yyyy-mm-dd HH:MM:SS");
        s_Cooldowns.Set(steam64, currentTime);
        SaveCooldowns();
    }
    
    private static void SaveCooldowns()
    {
        JsonFileLoader<map<string, string>>.JsonSaveFile(COOLDOWNS_PATH, s_Cooldowns);
    }
    
    private static void LoadCooldowns()
    {
        if (FileExist(COOLDOWNS_PATH))
        {
            JsonFileLoader<map<string, string>>.JsonLoadFile(COOLDOWNS_PATH, s_Cooldowns);
        }
    }
    
    private static void SaveWinnings()
    {
        JsonFileLoader<map<string, ref array<ref RouletteWinning>>>.JsonSaveFile(WINNINGS_PATH, s_Winnings);
    }
    
    private static void LoadWinnings()
    {
        if (!FileExist(WINNINGS_PATH))
            return;
        
        JsonFileLoader<map<string, ref array<ref RouletteWinning>>>.JsonLoadFile(WINNINGS_PATH, s_Winnings);
        
        array<string> keys = s_Winnings.GetKeyArray();
        int i;
        
        for (i = 0; i < keys.Count(); i = i + 1)
        {
            string steam64 = keys[i];
            array<ref RouletteWinning> winnings = s_Winnings.Get(steam64);
            
            if (!winnings)
                continue;
            
            int j;
            for (j = 0; j < winnings.Count(); j = j + 1)
            {
                RouletteWinning currentWinning = winnings[j];
                int winID = currentWinning.id;
                if (winID >= s_NextWinningID)
                {
                    s_NextWinningID = winID + 1;
                }
            }
        }
    }
    
    private static void SendSpinResult(PlayerBase player, PlayerIdentity identity, array<ref RouletteItem> sequence, RouletteItem prize)
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(sequence.Count());
        
        int i;
        for (i = 0; i < sequence.Count(); i = i + 1)
        {
            RouletteItem seqItem = sequence[i];
            rpc.Write(seqItem.classname);
            rpc.Write(seqItem.display_name);
            rpc.Write(seqItem.rarity);
        }
        
        rpc.Send(player, RouletteRPC.SEND_SPIN_RESULT, true, identity);
    }
    
    private static void SendClaimResult(PlayerBase player, PlayerIdentity identity, bool success, string message)
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(success);
        rpc.Write(message);
        rpc.Send(player, RouletteRPC.CLAIM_RESULT, true, identity);
    }
    
    private static void SendError(PlayerBase player, PlayerIdentity identity, string error)
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(error);
        rpc.Send(player, RouletteRPC.ERROR_MESSAGE, true, identity);
    }
    
    private static string FormatTime(int seconds)
    {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        
        if (hours > 0)
            return hours + "h " + minutes + "m";
        
        if (minutes > 0)
            return minutes + "m";
        
        return seconds + "s";
    }
    
    private static int ParseDateTime(string dateStr)
    {
        array<string> parts = new array<string>;
        dateStr.Split(" ", parts);
        
        if (parts.Count() != 2)
            return 0;
        
        string datePart = parts.Get(0);
        string timePart = parts.Get(1);
        
        array<string> dateParts = new array<string>;
        datePart.Split("-", dateParts);
        
        array<string> timeParts = new array<string>;
        timePart.Split(":", timeParts);
        
        if (dateParts.Count() != 3)
            return 0;
        
        if (timeParts.Count() != 3)
            return 0;
        
        string yearStr = dateParts.Get(0);
        string monthStr = dateParts.Get(1);
        string dayStr = dateParts.Get(2);
        string hourStr = timeParts.Get(0);
        string minuteStr = timeParts.Get(1);
        string secondStr = timeParts.Get(2);
        
        int year = yearStr.ToInt();
        int month = monthStr.ToInt();
        int day = dayStr.ToInt();
        int hour = hourStr.ToInt();
        int minute = minuteStr.ToInt();
        int second = secondStr.ToInt();
        
        CF_Date date = new CF_Date(year, month, day, hour, minute, second);
        return date.GetTimestamp();
    }
}