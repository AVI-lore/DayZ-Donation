// DonationSystem/scripts/5_Mission/GUI/HistoryMenu.c

class HistoryMenu extends UIScriptedMenu
{
    protected Widget m_Root;
    protected TextListboxWidget m_ItemList;
    protected ButtonWidget m_BtnClose;
    protected TextWidget m_StatusText;
    
    private ref array<ref DS_HistoryItem> m_Items;
    
    void HistoryMenu()
    {
        m_Items = new array<ref DS_HistoryItem>;
        Print("[DonationSystem] HistoryMenu constructor");
    }
    
    void ~HistoryMenu()
    {
        Print("[DonationSystem] HistoryMenu destructor - restoring game controls");
        
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
        Print("[DonationSystem] HistoryMenu Init()");
        
        m_Root = GetGame().GetWorkspace().CreateWidgets("DonationSystem/GUI/layouts/HistoryMenu.layout");
        
        if (!m_Root)
        {
            Error("[DonationSystem] Failed to load HistoryMenu.layout!");
            return NULL;
        }
        
        m_ItemList = TextListboxWidget.Cast(m_Root.FindAnyWidget("ItemList"));
        m_BtnClose = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClose"));
        m_StatusText = TextWidget.Cast(m_Root.FindAnyWidget("StatusText"));
        
        if (!m_ItemList)
            Error("[DonationSystem] ItemList not found!");
        if (!m_BtnClose)
            Error("[DonationSystem] BtnClose not found!");
        
        Print("[DonationSystem] HistoryMenu initialized");
        
        return m_Root;
    }
    
    override void OnShow()
    {
        super.OnShow();
        Print("[DonationSystem] HistoryMenu OnShow()");
        
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
        GetGame().GetMission().GetHud().Show(false);
        GetGame().GetUIManager().ShowUICursor(true);
        
        PPEffects.SetBlurMenu(0.5);
        
        RequestHistory();
    }
    
    override void OnHide()
    {
        super.OnHide();
        PPEffects.SetBlurMenu(0);
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);
        
        if (w == m_BtnClose)
        {
            Print("[DonationSystem] Close clicked - returning to game");
            Close();
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
        
        if (player.HasPendingHistoryData())
        {
            Print("[DonationSystem] Found pending history data!");
            
            array<int> ids;
            array<string> classnames;
            array<string> dates;
            player.ConsumePendingHistoryData(ids, classnames, dates);
            
            if (ids && classnames && dates)
            {
                DisplayHistoryItems(ids, classnames, dates);
            }
        }
    }
    
    void RequestHistory()
    {
        Print("[DonationSystem] RequestHistory");
        
        ShowStatus("Loading history...", false);
        m_Items.Clear();
        
        if (m_ItemList)
            m_ItemList.ClearItems();
        
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
        {
            ShowStatus("Error: Player not found", true);
            return;
        }
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send(player, 21005, true, player.GetIdentity());
        
        Print("[DonationSystem] History request sent via RPC 21005");
    }
    
    private void DisplayHistoryItems(array<int> ids, array<string> classnames, array<string> dates)
    {
        Print("[DonationSystem] DisplayHistoryItems: " + ids.Count() + " items");
        
        m_Items.Clear();
        
        if (m_ItemList)
            m_ItemList.ClearItems();
        
        int i;
        for (i = 0; i < ids.Count(); i++)
        {
            DS_HistoryItem item = new DS_HistoryItem(ids[i], classnames[i], dates[i]);
            m_Items.Insert(item);
            
            if (m_ItemList)
            {
                string displayText = dates[i] + " | " + classnames[i];
                m_ItemList.AddItem(displayText, NULL, 0);
                Print("[DonationSystem] Added history item: " + displayText);
            }
        }
        
        if (ids.Count() == 0)
        {
            ShowStatus("History is empty", false);
        }
        else
        {
            ShowStatus("History records: " + ids.Count(), false);
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.HideStatus, 3000, false);
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
            m_StatusText.SetColor(ARGB(255, 200, 200, 200));
    }
    
    void HideStatus()
    {
        if (m_StatusText && m_Root && m_Root.IsVisible())
            m_StatusText.Show(false);
    }
}