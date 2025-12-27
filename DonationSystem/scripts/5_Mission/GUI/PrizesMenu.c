// DonationSystem/scripts/5_Mission/GUI/PrizesMenu.c

class PrizesMenu extends UIScriptedMenu
{
    protected Widget m_Root;
    protected TextListboxWidget m_PrizesList;
    protected ButtonWidget m_BtnClaim;
    protected ButtonWidget m_BtnClaimAll;
    protected ButtonWidget m_BtnClose;
    protected TextWidget m_StatusText;
    
    private ref array<int> m_PrizeIds;
    private ref array<string> m_PrizeClassnames;
    private ref array<string> m_PrizeNames;
    private ref array<string> m_PrizeTypes;
    private ref array<string> m_PrizeDates;
    private int m_SelectedRow = -1;
    
    void PrizesMenu()
    {
        Print("[Roulette] PrizesMenu constructor");
        m_PrizeIds = new array<int>;
        m_PrizeClassnames = new array<string>;
        m_PrizeNames = new array<string>;
        m_PrizeTypes = new array<string>;
        m_PrizeDates = new array<string>;
    }
    
    void ~PrizesMenu()
    {
        Print("[Roulette] PrizesMenu destructor");
        
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
        Print("[Roulette] PrizesMenu Init()");
        
        m_Root = GetGame().GetWorkspace().CreateWidgets("DonationSystem/GUI/layouts/PrizesMenu.layout");
        
        if (!m_Root)
        {
            Error("[Roulette] Failed to load PrizesMenu.layout!");
            return NULL;
        }
        
        m_PrizesList = TextListboxWidget.Cast(m_Root.FindAnyWidget("PrizesList"));
        m_BtnClaim = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClaim"));
        m_BtnClaimAll = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClaimAll"));
        m_BtnClose = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClose"));
        m_StatusText = TextWidget.Cast(m_Root.FindAnyWidget("StatusText"));
        
        if (!m_PrizesList)
            Error("[Roulette] PrizesList not found!");
        if (!m_BtnClaim)
            Error("[Roulette] BtnClaim not found!");
        if (!m_BtnClaimAll)
            Error("[Roulette] BtnClaimAll not found!");
        if (!m_BtnClose)
            Error("[Roulette] BtnClose not found!");
        
        // Отключаем кнопки пока не выбран приз
        if (m_BtnClaim)
            m_BtnClaim.Enable(false);
        
        Print("[Roulette] PrizesMenu initialized");
        
        return m_Root;
    }
    
    override void OnShow()
    {
        super.OnShow();
        Print("[Roulette] PrizesMenu OnShow()");
        
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
        GetGame().GetMission().GetHud().Show(false);
        GetGame().GetUIManager().ShowUICursor(true);
        
        PPEffects.SetBlurMenu(0.5);
        
        // Запрашиваем список призов
        RequestPrizesList();
    }
    
    override void OnHide()
    {
        super.OnHide();
        PPEffects.SetBlurMenu(0);
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);
        
        if (w == m_BtnClaim)
        {
            Print("[Roulette] Claim selected clicked");
            ClaimSelected();
            return true;
        }
        
        if (w == m_BtnClaimAll)
        {
            Print("[Roulette] Claim all clicked");
            ClaimAll();
            return true;
        }
        
        if (w == m_BtnClose)
        {
            Print("[Roulette] Close clicked");
            Close();
            return true;
        }
        
        return false;
    }
    
    override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
    {
        if (w == m_PrizesList)
        {
            m_SelectedRow = row;
            Print("[Roulette] Selected prize row: " + row);
            
            if (m_BtnClaim && row >= 0 && row < m_PrizeIds.Count())
            {
                m_BtnClaim.Enable(true);
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
    
    // Запрос списка призов
    private void RequestPrizesList()
    {
        Print("[Roulette] Requesting prizes list");
        
        ShowStatus("Loading prizes...", false);
        
        if (m_PrizesList)
            m_PrizesList.ClearItems();
        
        m_PrizeIds.Clear();
        m_PrizeClassnames.Clear();
        m_PrizeNames.Clear();
        m_PrizeTypes.Clear();
        m_PrizeDates.Clear();
        m_SelectedRow = -1;
        
        if (m_BtnClaim)
            m_BtnClaim.Enable(false);
        
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
        {
            ShowStatus("Error: Player not found", true);
            return;
        }
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send(player, 22003, true, player.GetIdentity()); // REQUEST_WINNINGS
    }
    
    // Забрать выбранный приз
    private void ClaimSelected()
    {
        if (m_SelectedRow < 0 || m_SelectedRow >= m_PrizeIds.Count())
        {
            ShowStatus("Please select a prize first", true);
            return;
        }
        
        int prizeId = m_PrizeIds[m_SelectedRow];
        Print("[Roulette] Claiming prize ID: " + prizeId);
        
        ShowStatus("Claiming prize...", false);
        
        if (m_BtnClaim)
            m_BtnClaim.Enable(false);
        if (m_BtnClaimAll)
            m_BtnClaimAll.Enable(false);
        
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
        {
            ShowStatus("Error: Player not found", true);
            return;
        }
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(prizeId);
        rpc.Send(player, 22004, true, player.GetIdentity()); // CLAIM_WINNING
    }
    
    // Забрать все призы
    private void ClaimAll()
    {
        if (m_PrizeIds.Count() == 0)
        {
            ShowStatus("No prizes to claim", true);
            return;
        }
        
        Print("[Roulette] Claiming all prizes");
        
        ShowStatus("Claiming all prizes...", false);
        
        if (m_BtnClaim)
            m_BtnClaim.Enable(false);
        if (m_BtnClaimAll)
            m_BtnClaimAll.Enable(false);
        
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
        {
            ShowStatus("Error: Player not found", true);
            return;
        }
        
        ScriptRPC rpc = new ScriptRPC();
        rpc.Send(player, 22005, true, player.GetIdentity()); // CLAIM_ALL_WINNINGS
    }
    
    // Проверка данных от сервера
    private void CheckForServerData()
    {
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        
        // Получен список призов
        if (player.HasPendingPrizesList())
        {
            Print("[Roulette] Received prizes list from server");
            
            array<int> ids;
            array<string> classnames;
            array<string> names;
            array<string> types;
            array<string> dates;
            
            player.ConsumePendingPrizesList(ids, classnames, names, types, dates);
            
            if (ids && classnames && names && types && dates)
            {
                DisplayPrizes(ids, classnames, names, types, dates);
            }
        }
        
        // Получен результат claim
        if (player.HasPendingPrizeClaimResult())
        {
            Print("[Roulette] Received claim result from server");
            
            bool success;
            string message;
            player.ConsumePendingPrizeClaimResult(success, message);
            
            ShowClaimResult(success, message);
        }
    }
    
    // Отображение списка призов
    private void DisplayPrizes(array<int> ids, array<string> classnames, array<string> names, array<string> types, array<string> dates)
    {
        Print("[Roulette] Displaying " + ids.Count() + " prizes");
        
        m_PrizeIds = ids;
        m_PrizeClassnames = classnames;
        m_PrizeNames = names;
        m_PrizeTypes = types;
        m_PrizeDates = dates;
        
        if (m_PrizesList)
        {
            m_PrizesList.ClearItems();
            
            for (int i = 0; i < ids.Count(); i++)
            {
                string typeLabel = types[i] == "free" ? "FREE" : "PAID";
                string displayText = names[i] + " | " + typeLabel + " | " + dates[i];
                m_PrizesList.AddItem(displayText, NULL, 0);
            }
        }
        
        if (ids.Count() == 0)
        {
            ShowStatus("No prizes available", false);
        }
        else
        {
            ShowStatus("Prizes: " + ids.Count(), false);
            
            if (m_BtnClaimAll)
                m_BtnClaimAll.Enable(true);
        }
    }
    
    // Показать результат claim
    private void ShowClaimResult(bool success, string message)
    {
        Print("[Roulette] Claim result: " + success + ", message: " + message);
        
        ShowStatus(message, !success);
        
        if (success)
        {
            // Обновляем список через 1.5 секунды
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.RequestPrizesList, 1500, false);
        }
        else
        {
            // Включаем кнопки обратно при ошибке
            if (m_BtnClaim)
                m_BtnClaim.Enable(true);
            if (m_BtnClaimAll && m_PrizeIds.Count() > 0)
                m_BtnClaimAll.Enable(true);
        }
    }
    
    // Показать статус
    private void ShowStatus(string text, bool isError)
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
}