// DonationServer/scripts/4_World/DonationServer/DayZPlayerImplement.c
// ОБЪЕДИНЁННЫЙ ФАЙЛ - обрабатывает RPC и Donation и Roulette

modded class DayZPlayerImplement
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);
        
        // Только на сервере
        if (!GetGame().IsServer())
            return;
        
        if (!sender)
            return;
        
        string steam64 = sender.GetPlainId();
        
        // ========== DONATION RPC ==========
        if (rpc_type == DS_RPC.REQUEST_VAULT)
        {
            DonationConfig.Log("Processing REQUEST_VAULT from " + sender.GetName());
            DonationHTTPClient.PendingToVault(steam64, sender);
        }
        else if (rpc_type == DS_RPC.CLAIM_ITEM)
        {
            DonationConfig.Log("Processing CLAIM_ITEM");
            int purchase_id;
            if (ctx.Read(purchase_id))
            {
                DonationConfig.Log("Purchase ID: " + purchase_id);
                DonationHTTPClient.ClaimItem(purchase_id, steam64, sender);
            }
            else
            {
                DonationConfig.Log("ERROR: Failed to read purchase_id");
            }
        }
        else if (rpc_type == DS_RPC.REQUEST_HISTORY)
        {
            DonationConfig.Log("Processing REQUEST_HISTORY");
            DonationHTTPClient.GetHistory(steam64, sender);
        }
        
        // ========== ROULETTE RPC ==========
        else if (rpc_type == RouletteRPC.REQUEST_FREE_SPIN)
        {
            Print("[Roulette] REQUEST_FREE_SPIN from " + sender.GetName());
            RouletteManager.HandleFreeSpin(sender);
        }
        else if (rpc_type == RouletteRPC.REQUEST_PAID_SPIN)
        {
            Print("[Roulette] REQUEST_PAID_SPIN from " + sender.GetName());
            RouletteManager.HandlePaidSpin(sender);
        }
        else if (rpc_type == RouletteRPC.REQUEST_WINNINGS)
        {
            Print("[Roulette] REQUEST_WINNINGS from " + sender.GetName());
            RouletteManager.SendWinningsList(sender);
        }
        else if (rpc_type == RouletteRPC.CLAIM_WINNING)
        {
            int winningID;
            if (ctx.Read(winningID))
            {
                Print("[Roulette] CLAIM_WINNING ID=" + winningID + " from " + sender.GetName());
                RouletteManager.ClaimWinning(winningID, sender);
            }
        }
        else if (rpc_type == RouletteRPC.CLAIM_ALL_WINNINGS)
        {
            Print("[Roulette] CLAIM_ALL_WINNINGS from " + sender.GetName());
            RouletteManager.ClaimAllWinnings(sender);
        }
    }
}