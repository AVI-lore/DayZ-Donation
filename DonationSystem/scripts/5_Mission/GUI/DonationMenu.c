// DonationSystem/scripts/5_Mission/GUI/DonationMenu.c

class DonationMenu extends UIScriptedMenu
{
    protected Widget m_Root;
    protected ButtonWidget m_BtnVault;
    protected ButtonWidget m_BtnHistory;
    protected ButtonWidget m_BtnRoulette;
    protected ButtonWidget m_BtnPrizes;
    protected ButtonWidget m_BtnClose;
    
    private bool m_OpeningSubmenu = false;
    
    void DonationMenu()
    {
        Print("[DonationSystem] DonationMenu constructor");
    }
    
    void ~DonationMenu()
    {
        Print("[DonationSystem] DonationMenu destructor");
        
        if (!m_OpeningSubmenu)
        {
            Print("[DonationSystem] Restoring game controls");
            GetGame().GetInput().ResetGameFocus();
            GetGame().GetMission().PlayerControlEnable(true);
            GetGame().GetMission().GetHud().Show(true);
            GetGame().GetUIManager().ShowUICursor(false);
            PPEffects.SetBlurMenu(0);
        }
        else
        {
            Print("[DonationSystem] Opening submenu, not restoring controls");
        }
        
        if (m_Root)
        {
            m_Root.Unlink();
            m_Root = NULL;
        }
    }
    
    override Widget Init()
    {
        Print("[DonationSystem] DonationMenu Init() started");
        
        m_Root = GetGame().GetWorkspace().CreateWidgets("DonationSystem/GUI/layouts/MainMenu.layout");
        
        if (!m_Root)
        {
            Error("[DonationSystem] CRITICAL: Failed to load MainMenu.layout!");
            return NULL;
        }
        
        Print("[DonationSystem] Layout loaded successfully");
        
        m_BtnVault = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnVault"));
        m_BtnHistory = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnHistory"));
        m_BtnRoulette = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnRoulette"));
        m_BtnPrizes = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnPrizes"));
        m_BtnClose = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClose"));
        
        if (!m_BtnVault) Error("[DonationSystem] BtnVault not found!");
        if (!m_BtnHistory) Error("[DonationSystem] BtnHistory not found!");
        if (!m_BtnRoulette) Error("[DonationSystem] BtnRoulette not found!");
        if (!m_BtnPrizes) Error("[DonationSystem] BtnPrizes not found!");
        if (!m_BtnClose) Error("[DonationSystem] BtnClose not found!");
        
        Print("[DonationSystem] DonationMenu Init() completed");
        
        return m_Root;
    }
    
    override void OnShow()
    {
        super.OnShow();
        Print("[DonationSystem] DonationMenu OnShow()");
        
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
        GetGame().GetMission().GetHud().Show(false);
        GetGame().GetUIManager().ShowUICursor(true);
        
        PPEffects.SetBlurMenu(0.5);
    }
    
    override void OnHide()
    {
        super.OnHide();
        Print("[DonationSystem] DonationMenu OnHide()");
        
        if (!m_OpeningSubmenu)
        {
            PPEffects.SetBlurMenu(0);
        }
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);
        
        Print("[DonationSystem] OnClick: " + w.GetName());
        
        if (w == m_BtnVault)
        {
            Print("[DonationSystem] Vault button clicked");
            m_OpeningSubmenu = true;
            OpenVaultMenu();
            return true;
        }
        
        if (w == m_BtnHistory)
        {
            Print("[DonationSystem] History button clicked");
            m_OpeningSubmenu = true;
            OpenHistoryMenu();
            return true;
        }
        
        if (w == m_BtnRoulette)
        {
            Print("[DonationSystem] Roulette button clicked");
            m_OpeningSubmenu = true;
            OpenRouletteMenu();
            return true;
        }
        
        if (w == m_BtnPrizes)
        {
            Print("[DonationSystem] Prizes button clicked");
            m_OpeningSubmenu = true;
            OpenPrizesMenu();
            return true;
        }
        
        if (w == m_BtnClose)
        {
            Print("[DonationSystem] Close button clicked - closing menu");
            Close();
            return true;
        }
        
        return false;
    }
    
    void OpenVaultMenu()
    {
        Print("[DonationSystem] Opening Vault menu");
        
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (mission)
        {
            mission.OpenVaultMenu();
        }
    }
    
    void OpenHistoryMenu()
    {
        Print("[DonationSystem] Opening History menu");
        
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (mission)
        {
            mission.OpenHistoryMenu();
        }
    }
    
    void OpenRouletteMenu()
    {
        Print("[DonationSystem] Opening Roulette menu");
        
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (mission)
        {
            mission.OpenRouletteMenu();
        }
    }
    
    void OpenPrizesMenu()
    {
        Print("[DonationSystem] Opening Prizes menu");
        
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (mission)
        {
            mission.OpenPrizesMenu();
        }
    }
}

// Helper class для возврата в главное меню из подменю
class DonationMenuHelper
{
    static void OpenMainMenuStatic()
    {
        Print("[DonationSystem] Opening main menu");
        
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (mission)
        {
            mission.OpenDonationMenu();
        }
    }
}