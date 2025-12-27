// DonationServer/scripts/5_Mission/Roulette/MissionServer_Roulette.c
// Инициализация рулетки на сервере

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        
        Print("[Roulette] Initializing roulette system...");
        RouletteManager.Init();
        Print("[Roulette] Roulette system initialized");
    }
}