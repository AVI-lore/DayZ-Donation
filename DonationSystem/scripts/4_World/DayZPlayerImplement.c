// DonationSystem/scripts/4_World/DonationSystem/DayZPlayerImplement.c
// КЛИЕНТСКАЯ ЧАСТЬ - обработка RPC от сервера

modded class DayZPlayerImplement
{
    // Временное хранилище для данных RPC (Donation)
    private ref array<int> m_PendingVaultIds;
    private ref array<string> m_PendingVaultClassnames;
    private bool m_HasPendingVaultData = false;
    
    private ref array<int> m_PendingHistoryIds;
    private ref array<string> m_PendingHistoryClassnames;
    private ref array<string> m_PendingHistoryDates;
    private bool m_HasPendingHistoryData = false;
    
    private bool m_HasPendingClaimResult = false;
    private bool m_PendingClaimSuccess;
    private string m_PendingClaimMessage;
    
    // Временное хранилище для данных RPC (Roulette)
    private ref array<string> m_PendingRouletteClassnames;
    private ref array<string> m_PendingRouletteNames;
    private ref array<string> m_PendingRouletteRarities;
    private bool m_HasPendingRouletteResult = false;
    
    private ref array<int> m_PendingPrizeIds;
    private ref array<string> m_PendingPrizeClassnames;
    private ref array<string> m_PendingPrizeNames;
    private ref array<string> m_PendingPrizeTypes;
    private ref array<string> m_PendingPrizeDates;
    private bool m_HasPendingPrizesList = false;
    
    private bool m_HasPendingPrizeClaimResult = false;
    private bool m_PendingPrizeClaimSuccess;
    private string m_PendingPrizeClaimMessage;
    
    private bool m_HasPendingRouletteError = false;
    private string m_PendingRouletteErrorMessage;
    
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);
        
        Print("[DonationSystem] CLIENT: OnRPC type: " + rpc_type);
        
        // Только на клиенте
        if (GetGame().IsServer())
            return;
        
        // Donation RPC
        switch (rpc_type)
        {
            case 21002: // SEND_VAULT
                ReceiveVaultData(ctx);
                break;
                
            case 21004: // CLAIM_RESULT
                ReceiveClaimResult(ctx);
                break;
                
            case 21006: // SEND_HISTORY
                ReceiveHistoryData(ctx);
                break;
                
            case 22101: // RouletteRPC.SEND_SPIN_RESULT
                ReceiveRouletteSpinResult(ctx);
                break;
                
            case 22102: // RouletteRPC.SEND_WINNINGS_LIST
                ReceiveRoulettePrizesList(ctx);
                break;
                
            case 22103: // RouletteRPC.CLAIM_RESULT (roulette)
                ReceiveRoulettePrizeClaimResult(ctx);
                break;
                
            case 22104: // RouletteRPC.ERROR_MESSAGE
                ReceiveRouletteError(ctx);
                break;
        }
    }
    
    // ========== DONATION HANDLERS ==========
    
    private void ReceiveVaultData(ParamsReadContext ctx)
    {
        int count;
        if (!ctx.Read(count)) return;
        
        m_PendingVaultIds = new array<int>;
        m_PendingVaultClassnames = new array<string>;
        
        for (int i = 0; i < count; i++)
        {
            int id;
            string classname;
            
            if (!ctx.Read(id) || !ctx.Read(classname))
                continue;
            
            m_PendingVaultIds.Insert(id);
            m_PendingVaultClassnames.Insert(classname);
        }
        
        m_HasPendingVaultData = true;
    }
    
    private void ReceiveClaimResult(ParamsReadContext ctx)
    {
        if (!ctx.Read(m_PendingClaimSuccess) || !ctx.Read(m_PendingClaimMessage))
            return;
        
        m_HasPendingClaimResult = true;
    }
    
    private void ReceiveHistoryData(ParamsReadContext ctx)
    {
        int count;
        if (!ctx.Read(count)) return;
        
        m_PendingHistoryIds = new array<int>;
        m_PendingHistoryClassnames = new array<string>;
        m_PendingHistoryDates = new array<string>;
        
        for (int i = 0; i < count; i++)
        {
            int id;
            string classname;
            string date;
            
            if (!ctx.Read(id) || !ctx.Read(classname) || !ctx.Read(date))
                continue;
            
            m_PendingHistoryIds.Insert(id);
            m_PendingHistoryClassnames.Insert(classname);
            m_PendingHistoryDates.Insert(date);
        }
        
        m_HasPendingHistoryData = true;
    }
    
    // ========== ROULETTE HANDLERS ==========
    
    private void ReceiveRouletteSpinResult(ParamsReadContext ctx)
    {
        int count;
        if (!ctx.Read(count)) return;
        
        Print("[Roulette] CLIENT: Receiving spin result, count: " + count);
        
        m_PendingRouletteClassnames = new array<string>;
        m_PendingRouletteNames = new array<string>;
        m_PendingRouletteRarities = new array<string>;
        
        for (int i = 0; i < count; i++)
        {
            string classname, name, rarity;
            
            if (!ctx.Read(classname) || !ctx.Read(name) || !ctx.Read(rarity))
                continue;
            
            m_PendingRouletteClassnames.Insert(classname);
            m_PendingRouletteNames.Insert(name);
            m_PendingRouletteRarities.Insert(rarity);
        }
        
        m_HasPendingRouletteResult = true;
        Print("[Roulette] CLIENT: Spin result stored");
    }
    
    private void ReceiveRoulettePrizesList(ParamsReadContext ctx)
    {
        int count;
        if (!ctx.Read(count)) return;
        
        Print("[Roulette] CLIENT: Receiving prizes list, count: " + count);
        
        m_PendingPrizeIds = new array<int>;
        m_PendingPrizeClassnames = new array<string>;
        m_PendingPrizeNames = new array<string>;
        m_PendingPrizeTypes = new array<string>;
        m_PendingPrizeDates = new array<string>;
        
        for (int i = 0; i < count; i++)
        {
            int id;
            string classname, name, type, date;
            
            if (!ctx.Read(id) || !ctx.Read(classname) || !ctx.Read(name) || !ctx.Read(type) || !ctx.Read(date))
                continue;
            
            m_PendingPrizeIds.Insert(id);
            m_PendingPrizeClassnames.Insert(classname);
            m_PendingPrizeNames.Insert(name);
            m_PendingPrizeTypes.Insert(type);
            m_PendingPrizeDates.Insert(date);
        }
        
        m_HasPendingPrizesList = true;
    }
    
    private void ReceiveRoulettePrizeClaimResult(ParamsReadContext ctx)
    {
        if (!ctx.Read(m_PendingPrizeClaimSuccess) || !ctx.Read(m_PendingPrizeClaimMessage))
            return;
        
        m_HasPendingPrizeClaimResult = true;
    }
    
    private void ReceiveRouletteError(ParamsReadContext ctx)
    {
        if (!ctx.Read(m_PendingRouletteErrorMessage))
            return;
        
        m_HasPendingRouletteError = true;
        Print("[Roulette] CLIENT: Error received: " + m_PendingRouletteErrorMessage);
    }
    
    // ========== PUBLIC DONATION METHODS ==========
    
    bool HasPendingVaultData()
    {
        return m_HasPendingVaultData;
    }
    
    void ConsumePendingVaultData(out array<int> ids, out array<string> classnames)
    {
        if (!m_HasPendingVaultData) return;
        
        ids = m_PendingVaultIds;
        classnames = m_PendingVaultClassnames;
        
        m_HasPendingVaultData = false;
        m_PendingVaultIds = NULL;
        m_PendingVaultClassnames = NULL;
    }
    
    bool HasPendingClaimResult()
    {
        return m_HasPendingClaimResult;
    }
    
    void ConsumePendingClaimResult(out bool success, out string message)
    {
        if (!m_HasPendingClaimResult) return;
        
        success = m_PendingClaimSuccess;
        message = m_PendingClaimMessage;
        
        m_HasPendingClaimResult = false;
    }
    
    bool HasPendingHistoryData()
    {
        return m_HasPendingHistoryData;
    }
    
    void ConsumePendingHistoryData(out array<int> ids, out array<string> classnames, out array<string> dates)
    {
        if (!m_HasPendingHistoryData) return;
        
        ids = m_PendingHistoryIds;
        classnames = m_PendingHistoryClassnames;
        dates = m_PendingHistoryDates;
        
        m_HasPendingHistoryData = false;
        m_PendingHistoryIds = NULL;
        m_PendingHistoryClassnames = NULL;
        m_PendingHistoryDates = NULL;
    }
    
    // ========== PUBLIC ROULETTE METHODS ==========
    
    bool HasPendingRouletteResult()
    {
        return m_HasPendingRouletteResult;
    }
    
    void ConsumePendingRouletteResult(out array<string> classnames, out array<string> names, out array<string> rarities)
    {
        if (!m_HasPendingRouletteResult) return;
        
        classnames = m_PendingRouletteClassnames;
        names = m_PendingRouletteNames;
        rarities = m_PendingRouletteRarities;
        
        m_HasPendingRouletteResult = false;
        m_PendingRouletteClassnames = NULL;
        m_PendingRouletteNames = NULL;
        m_PendingRouletteRarities = NULL;
    }
    
    bool HasPendingPrizesList()
    {
        return m_HasPendingPrizesList;
    }
    
    void ConsumePendingPrizesList(out array<int> ids, out array<string> classnames, out array<string> names, out array<string> types, out array<string> dates)
    {
        if (!m_HasPendingPrizesList) return;
        
        ids = m_PendingPrizeIds;
        classnames = m_PendingPrizeClassnames;
        names = m_PendingPrizeNames;
        types = m_PendingPrizeTypes;
        dates = m_PendingPrizeDates;
        
        m_HasPendingPrizesList = false;
        m_PendingPrizeIds = NULL;
        m_PendingPrizeClassnames = NULL;
        m_PendingPrizeNames = NULL;
        m_PendingPrizeTypes = NULL;
        m_PendingPrizeDates = NULL;
    }
    
    bool HasPendingPrizeClaimResult()
    {
        return m_HasPendingPrizeClaimResult;
    }
    
    void ConsumePendingPrizeClaimResult(out bool success, out string message)
    {
        if (!m_HasPendingPrizeClaimResult) return;
        
        success = m_PendingPrizeClaimSuccess;
        message = m_PendingPrizeClaimMessage;
        
        m_HasPendingPrizeClaimResult = false;
    }
    
    bool HasPendingRouletteError()
    {
        return m_HasPendingRouletteError;
    }
    
    void ConsumePendingRouletteError(out string errorMsg)
    {
        if (!m_HasPendingRouletteError) return;
        
        errorMsg = m_PendingRouletteErrorMessage;
        
        m_HasPendingRouletteError = false;
        m_PendingRouletteErrorMessage = "";
    }
}