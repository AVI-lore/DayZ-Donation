// DonationSystem/scripts/5_Mission/GUI/VaultMenu.c

class VaultMenu extends UIScriptedMenu
{
    protected Widget m_Root;
    protected TextListboxWidget m_ItemList;
    protected ButtonWidget m_BtnRefresh;
    protected ButtonWidget m_BtnClaim;
    protected ButtonWidget m_BtnClose;
    protected TextWidget m_StatusText;
    
    private ref array<ref DS_VaultItem> m_Items;
    private int m_SelectedRow = -1;
    private bool m_ClaimInProgress = false;
    
    void VaultMenu()
    {
        m_Items = new array<ref DS_VaultItem>;
        Print("[DonationSystem] VaultMenu constructor");
    }
    
    void ~VaultMenu()
    {
        Print("[DonationSystem] VaultMenu destructor - restoring game controls");
        
        // Всегда восстанавливаем управление при закрытии
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetMission().PlayerControlEnable(true);
        GetGame().GetMission().GetHud().Show(true);
        GetGame().GetUIManager().ShowUICursor(false);
        PPEffects.SetBlurMenu(0);
        
        if (m_Root)
        {
            m_Root.Unlink();
            m_Root = NULL;
        }
    }
    
    override Widget Init()
    {
        Print("[DonationSystem] VaultMenu Init()");
        
        m_Root = GetGame().GetWorkspace().CreateWidgets("DonationSystem/GUI/layouts/VaultMenu.layout");
        
        if (!m_Root)
        {
            Error("[DonationSystem] Failed to load VaultMenu.layout!");
            return NULL;
        }
        
        m_ItemList = TextListboxWidget.Cast(m_Root.FindAnyWidget("ItemList"));
        m_BtnRefresh = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnRefresh"));
        m_BtnClaim = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClaim"));
        m_BtnClose = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClose"));
        m_StatusText = TextWidget.Cast(m_Root.FindAnyWidget("StatusText"));
        
        if (!m_ItemList)
            Error("[DonationSystem] ItemList not found!");
        if (!m_BtnRefresh)
            Error("[DonationSystem] BtnRefresh not found!");
        if (!m_BtnClaim)
            Error("[DonationSystem] BtnClaim not found!");
        if (!m_BtnClose)
            Error("[DonationSystem] BtnClose not found!");
        
        Print("[DonationSystem] VaultMenu initialized");
        
        return m_Root;
    }
    
    override void OnShow()
    {
        super.OnShow();
        Print("[DonationSystem] VaultMenu OnShow()");
        
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
        GetGame().GetMission().GetHud().Show(false);
        GetGame().GetUIManager().ShowUICursor(true);
        
        PPEffects.SetBlurMenu(0.5);
        
        RequestVaultItems();
    }
    
    override void OnHide()
    {
        super.OnHide();
        Print("[DonationSystem] VaultMenu OnHide()");
        PPEffects.SetBlurMenu(0);
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);
        
        if (w == m_BtnRefresh)
        {
            Print("[DonationSystem] Refresh clicked");
            RequestVaultItems();
            return true;
        }
        
        if (w == m_BtnClaim)
        {
            Print("[DonationSystem] Claim clicked");
            ClaimSelectedItem();
            return true;
        }
        
        if (w == m_BtnClose)
        {
            Print("[DonationSystem] Close clicked - returning to game");
            Close();
            return true;
        }
        
        return false;
    }
    
    override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
    {
        if (w == m_ItemList)
        {
            m_SelectedRow = row;
            Print("[DonationSystem] Selected row: " + row);
            
            if (m_BtnClaim && !m_ClaimInProgress)
            {
                m_BtnClaim.Enable(true);
            }
            
            if (row >= 0 && row < m_Items.Count())
            {
                DS_VaultItem item = m_Items[row];
                ShowStatus("Selected: " + item.classname, false);
                GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.HideStatus, 2000, false);
            }
            
            return true;
        }
        
        return false;
    }
    
    override void Update(float timeslice)
    {
        super.Update(timeslice);
        
        CheckForServerData();
    }
    
    private void CheckForServerData()
    {
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        
        if (player.HasPendingVaultData())
        {
            Print("[DonationSystem] CLIENT: Found pending vault data!");
            
            array<int> ids;
            array<string> classnames;
            player.ConsumePendingVaultData(ids, classnames);
            
            if (ids && classnames)
            {
                DisplayVaultItems(ids, classnames);
            }
        }
        
        if (player.HasPendingClaimResult())
        {
            Print("[DonationSystem] CLIENT: Found pending claim result!");
            
            bool success;
            string message;
            player.ConsumePendingClaimResult(success, message);
            
            ShowClaimResult(success, message);
        }
    }
    
    void RequestVaultItems()
    {
        Print("[DonationSystem] RequestVaultItems");
        
        ShowStatus("Loading...", false);
        m_Items.Clear();
        m_SelectedRow = -1;
        m_ClaimInProgress = false;
        
        if (m_ItemList)
            m_ItemList.ClearItems();
        
        if (m_BtnClaim)
            m_BtnClaim.Enable(false);
        
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        
        if (!player)
        {
            Print("[DonationSystem] ERROR: Player not found!");
            ShowStatus("Error: Player not found", true);
            return;
        }
        
        Print("[DonationSystem] Sending REQUEST_VAULT RPC");
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send(player, 21001, true, player.GetIdentity());
    }
    
    void ClaimSelectedItem()
    {
        if (m_ClaimInProgress)
        {
            Print("[DonationSystem] Claim already in progress, ignoring");
            return;
        }
        
        if (m_SelectedRow < 0 || m_SelectedRow >= m_Items.Count())
        {
            ShowStatus("Select an item first!", true);
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.HideStatus, 3000, false);
            return;
        }
        
        DS_VaultItem item = m_Items[m_SelectedRow];
        if (!item)
            return;
        
        Print("[DonationSystem] Claiming item " + item.id);
        
        m_ClaimInProgress = true;
        ShowStatus("Claiming " + item.classname + "...", false);
        
        if (m_BtnClaim)
            m_BtnClaim.Enable(false);
        
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
        {
            m_ClaimInProgress = false;
            return;
        }
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(item.id);
        rpc.Send(player, 21003, true, player.GetIdentity());
    }
    
    private void DisplayVaultItems(array<int> ids, array<string> classnames)
    {
        Print("[DonationSystem] DisplayVaultItems: " + ids.Count() + " items");
        
        m_Items.Clear();
        
        if (m_ItemList)
            m_ItemList.ClearItems();
        
        int i;
        for (i = 0; i < ids.Count(); i++)
        {
            DS_VaultItem item = new DS_VaultItem(ids[i], classnames[i]);
            m_Items.Insert(item);
            
            if (m_ItemList)
            {
                string displayText = "#" + ids[i] + " | " + classnames[i];
                m_ItemList.AddItem(displayText, NULL, 0);
            }
        }
        
        if (ids.Count() == 0)
        {
            ShowStatus("No items available", false);
        }
        else
        {
            ShowStatus("Loaded: " + ids.Count() + " items", false);
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.HideStatus, 3000, false);
        }
    }
    
    private void ShowClaimResult(bool success, string message)
    {
        Print("[DonationSystem] ShowClaimResult: " + success + ", " + message);
        
        m_ClaimInProgress = false;
        
        if (success)
        {
            ShowStatus("✓ " + message, false);
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.RequestVaultItems, 1500, false);
        }
        else
        {
            ShowStatus("✗ " + message, true);
            
            if (m_BtnClaim)
                m_BtnClaim.Enable(true);
            
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.HideStatus, 5000, false);
        }
    }
    
    void ShowStatus(string text, bool isError)
    {
        if (!m_StatusText)
            return;
        
        m_StatusText.SetText(text);
        m_StatusText.Show(true);
        
        if (isError)
            m_StatusText.SetColor(ARGB(255, 255, 100, 100));
        else
            m_StatusText.SetColor(ARGB(255, 128, 255, 128));
    }
    
    void HideStatus()
    {
        if (m_StatusText && m_Root && m_Root.IsVisible())
            m_StatusText.Show(false);
    }
}