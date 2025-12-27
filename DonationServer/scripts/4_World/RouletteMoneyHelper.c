// DonationServer/scripts/4_World/Roulette/RouletteMoneyHelper.c
// Работа с долларами TraderPlus

class RouletteMoneyHelper
{
    // Список всех купюр TraderPlus (от меньшего к большему)
    private static ref array<string> MONEY_CLASSES = {
        "TraderPlus_Money_Dollar1",
        "TraderPlus_Money_Dollar5",
        "TraderPlus_Money_Dollar10",
        "TraderPlus_Money_Dollar20",
        "TraderPlus_Money_Dollar50",
        "TraderPlus_Money_Dollar100"
    };
    
    private static ref array<int> MONEY_VALUES = {
        1, 5, 10, 20, 50, 100
    };
    
    // Считает сколько денег у игрока
    static int CountPlayerMoney(PlayerBase player)
    {
        if (!player)
            return 0;
        
        int totalMoney = 0;
        array<EntityAI> items = new array<EntityAI>;
        
        // Получаем все предметы в инвентаре
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        
        for (int i = 0; i < items.Count(); i++)
        {
            EntityAI item = items[i];
            if (!item)
                continue;
            
            string type = item.GetType();
            int value = GetMoneyValue(type);
            
            if (value > 0)
            {
                totalMoney += value;
            }
        }
        
        Print("[Roulette] Player has $" + totalMoney);
        return totalMoney;
    }
    
    // Проверяет хватает ли денег
    static bool HasEnoughMoney(PlayerBase player, int requiredAmount)
    {
        return CountPlayerMoney(player) >= requiredAmount;
    }
    
    // Забирает деньги у игрока и даёт сдачу
    static bool TakeMoney(PlayerBase player, int amount, out string error)
    {
        if (!player)
        {
            error = "Player not found";
            return false;
        }
        
        int playerMoney = CountPlayerMoney(player);
        
        if (playerMoney < amount)
        {
            error = "Not enough money. Need $" + amount + ", you have $" + playerMoney;
            return false;
        }
        
        Print("[Roulette] Taking $" + amount + " from player");
        
        // Удаляем все деньги
        array<EntityAI> allMoney = new array<EntityAI>;
        FindAllMoney(player, allMoney);
        
        for (int i = 0; i < allMoney.Count(); i++)
        {
            GetGame().ObjectDelete(allMoney[i]);
        }
        
        // Даём сдачу
        int change = playerMoney - amount;
        
        if (change > 0)
        {
            Print("[Roulette] Giving change: $" + change);
            GiveChange(player, change);
        }
        
        Print("[Roulette] Money taken successfully");
        return true;
    }
    
    // Даёт сдачу крупными купюрами
    static void GiveChange(PlayerBase player, int amount)
    {
        if (amount <= 0)
            return;
        
        Print("[Roulette] Giving $" + amount + " change");
        
        // Идём от больших купюр к меньшим
        for (int i = MONEY_VALUES.Count() - 1; i >= 0; i--)
        {
            int value = MONEY_VALUES[i];
            string moneyClass = MONEY_CLASSES[i];
            
            while (amount >= value)
            {
                // Пытаемся выдать в инвентарь
                EntityAI money = player.GetInventory().CreateInInventory(moneyClass);
                
                if (!money)
                {
                    // Если инвентарь полон - на землю
                    vector pos = player.GetPosition();
                    money = EntityAI.Cast(GetGame().CreateObjectEx(moneyClass, pos, ECE_PLACE_ON_SURFACE));
                }
                
                if (money)
                {
                    amount -= value;
                    Print("[Roulette] Gave $" + value + " bill");
                }
                else
                {
                    Print("[Roulette] ERROR: Failed to give $" + value + " bill");
                    break;
                }
            }
        }
        
        if (amount > 0)
        {
            Print("[Roulette] WARNING: Could not give full change, remaining: $" + amount);
        }
    }
    
    // Получает стоимость купюры
    private static int GetMoneyValue(string className)
    {
        for (int i = 0; i < MONEY_CLASSES.Count(); i++)
        {
            if (className == MONEY_CLASSES[i])
            {
                return MONEY_VALUES[i];
            }
        }
        return 0;
    }
    
    // Находит все деньги в инвентаре
    private static void FindAllMoney(PlayerBase player, out array<EntityAI> moneyItems)
    {
        array<EntityAI> items = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        
        for (int i = 0; i < items.Count(); i++)
        {
            EntityAI item = items[i];
            if (!item)
                continue;
            
            string type = item.GetType();
            
            if (GetMoneyValue(type) > 0)
            {
                moneyItems.Insert(item);
            }
        }
    }
}