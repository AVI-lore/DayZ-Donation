// DonationSystem/scripts/5_Mission/DonationSystem/MissionGameplay.c

modded class MissionGameplay
{
    private ref DonationMenu m_DonationMenu;
    private ref VaultMenu m_VaultMenu;
    private ref HistoryMenu m_HistoryMenu;
    private ref RouletteMenu m_RouletteMenu;
    private ref PrizesMenu m_PrizesMenu;
    
    void MissionGameplay()
    {
        Print("[DonationSystem] MissionGameplay constructor");
    }
    
    override void OnInit()
    {
        super.OnInit();
        Print("[DonationSystem] MissionGameplay OnInit()");
    }
    
    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        
        if (GetGame().GetInput().LocalPress("UADonationMenu", false))
        {
            Print("[DonationSystem] KEY B PRESSED");
            
            UIScriptedMenu currentMenu = GetGame().GetUIManager().GetMenu();
            
            if (currentMenu == NULL)
            {
                OpenDonationMenu();
            }
            else
            {
                Print("[DonationSystem] Another menu is open: " + currentMenu.ClassName());
            }
        }
    }
    
    void OpenDonationMenu()
    {
        Print("[DonationSystem] OpenDonationMenu()");
        
        CloseAllSubMenus();
        
        if (m_DonationMenu)
        {
            m_DonationMenu.Close();
            m_DonationMenu = NULL;
        }
        
        m_DonationMenu = new DonationMenu();
        
        if (!m_DonationMenu)
        {
            Error("[DonationSystem] Failed to create DonationMenu!");
            return;
        }
        
        GetGame().GetUIManager().ShowScriptedMenu(m_DonationMenu, NULL);
    }
    
    void OpenVaultMenu()
    {
        Print("[DonationSystem] OpenVaultMenu()");
        
        if (m_DonationMenu)
        {
            m_DonationMenu.Close();
            m_DonationMenu = NULL;
        }
        
        CloseAllSubMenus();
        
        m_VaultMenu = new VaultMenu();
        GetGame().GetUIManager().ShowScriptedMenu(m_VaultMenu, NULL);
    }
    
    void OpenHistoryMenu()
    {
        Print("[DonationSystem] OpenHistoryMenu()");
        
        if (m_DonationMenu)
        {
            m_DonationMenu.Close();
            m_DonationMenu = NULL;
        }
        
        CloseAllSubMenus();
        
        m_HistoryMenu = new HistoryMenu();
        GetGame().GetUIManager().ShowScriptedMenu(m_HistoryMenu, NULL);
    }
    
    void OpenRouletteMenu()
    {
        Print("[DonationSystem] OpenRouletteMenu()");
        
        if (m_DonationMenu)
        {
            m_DonationMenu.Close();
            m_DonationMenu = NULL;
        }
        
        CloseAllSubMenus();
        
        m_RouletteMenu = new RouletteMenu();
        GetGame().GetUIManager().ShowScriptedMenu(m_RouletteMenu, NULL);
    }
    
    void OpenPrizesMenu()
    {
        Print("[DonationSystem] OpenPrizesMenu()");
        
        if (m_DonationMenu)
        {
            m_DonationMenu.Close();
            m_DonationMenu = NULL;
        }
        
        CloseAllSubMenus();
        
        m_PrizesMenu = new PrizesMenu();
        GetGame().GetUIManager().ShowScriptedMenu(m_PrizesMenu, NULL);
    }
    
    void CloseAllSubMenus()
    {
        if (m_VaultMenu)
        {
            m_VaultMenu.Close();
            m_VaultMenu = NULL;
        }
        if (m_HistoryMenu)
        {
            m_HistoryMenu.Close();
            m_HistoryMenu = NULL;
        }
        if (m_RouletteMenu)
        {
            m_RouletteMenu.Close();
            m_RouletteMenu = NULL;
        }
        if (m_PrizesMenu)
        {
            m_PrizesMenu.Close();
            m_PrizesMenu = NULL;
        }
    }
}