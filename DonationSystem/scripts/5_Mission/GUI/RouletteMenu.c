// DonationSystem/scripts/5_Mission/GUI/RouletteMenu.c

class RouletteMenu extends UIScriptedMenu
{
    protected Widget m_Root;
    protected ButtonWidget m_BtnFreeType;
    protected ButtonWidget m_BtnPaidType;
    protected ButtonWidget m_BtnSpin;
    protected ButtonWidget m_BtnClose;
    protected TextWidget m_Slot1Name;
    protected TextWidget m_Slot2Name;
    protected TextWidget m_Slot3Name;
    protected TextWidget m_Slot4Name;
    protected TextWidget m_Slot5Name;
    protected TextWidget m_InfoText;
    
    private bool m_IsFreeType = true; // true = Free, false = Paid
    private bool m_IsSpinning = false;
    private int m_SpinFrame = 0;
    private int m_SpinDelay = 100; // ms между кадрами
    private float m_SpinTimer = 0;
    
    private ref array<string> m_SpinSequence; // Classnames для анимации
    private ref array<string> m_SpinNames; // Названия для отображения
    
    void RouletteMenu()
    {
        Print("[Roulette] RouletteMenu constructor");
    }
    
    void ~RouletteMenu()
    {
        Print("[Roulette] RouletteMenu destructor");
        
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
        Print("[Roulette] RouletteMenu Init()");
        
        m_Root = GetGame().GetWorkspace().CreateWidgets("DonationSystem/GUI/layouts/RouletteMenu.layout");
        
        if (!m_Root)
        {
            Error("[Roulette] Failed to load RouletteMenu.layout!");
            return NULL;
        }
        
        m_BtnFreeType = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnFreeType"));
        m_BtnPaidType = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnPaidType"));
        m_BtnSpin = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnSpin"));
        m_BtnClose = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClose"));
        m_Slot1Name = TextWidget.Cast(m_Root.FindAnyWidget("Slot1Name"));
        m_Slot2Name = TextWidget.Cast(m_Root.FindAnyWidget("Slot2Name"));
        m_Slot3Name = TextWidget.Cast(m_Root.FindAnyWidget("Slot3Name"));
        m_Slot4Name = TextWidget.Cast(m_Root.FindAnyWidget("Slot4Name"));
        m_Slot5Name = TextWidget.Cast(m_Root.FindAnyWidget("Slot5Name"));
        m_InfoText = TextWidget.Cast(m_Root.FindAnyWidget("InfoText"));
        
        // Подсвечиваем активную кнопку
        UpdateTypeButtons();
        
        Print("[Roulette] RouletteMenu initialized");
        
        return m_Root;
    }
    
    override void OnShow()
    {
        super.OnShow();
        Print("[Roulette] RouletteMenu OnShow()");
        
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
        GetGame().GetMission().GetHud().Show(false);
        GetGame().GetUIManager().ShowUICursor(true);
        
        PPEffects.SetBlurMenu(0.5);
        
        UpdateInfoText();
    }
    
    override void OnHide()
    {
        super.OnHide();
        PPEffects.SetBlurMenu(0);
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);
        
        if (m_IsSpinning)
            return true; // Блокируем клики во время спина
        
        if (w == m_BtnFreeType)
        {
            Print("[Roulette] Free type selected");
            m_IsFreeType = true;
            UpdateTypeButtons();
            UpdateInfoText();
            return true;
        }
        
        if (w == m_BtnPaidType)
        {
            Print("[Roulette] Paid type selected");
            m_IsFreeType = false;
            UpdateTypeButtons();
            UpdateInfoText();
            return true;
        }
        
        if (w == m_BtnSpin)
        {
            Print("[Roulette] Spin button clicked");
            RequestSpin();
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
    
    override void Update(float timeslice)
    {
        super.Update(timeslice);
        
        // Проверяем данные от сервера
        CheckForServerData();
        
        // Обновляем анимацию спина
        if (m_IsSpinning)
        {
            UpdateSpinAnimation(timeslice);
        }
    }
    
    // Обновление подсветки кнопок типа
    private void UpdateTypeButtons()
    {
        if (m_BtnFreeType)
        {
            if (m_IsFreeType)
                m_BtnFreeType.SetColor(ARGB(255, 100, 200, 100));
            else
                m_BtnFreeType.SetColor(ARGB(255, 150, 150, 150));
        }
        
        if (m_BtnPaidType)
        {
            if (!m_IsFreeType)
                m_BtnPaidType.SetColor(ARGB(255, 100, 200, 100));
            else
                m_BtnPaidType.SetColor(ARGB(255, 150, 150, 150));
        }
    }
    
    // Обновление информационного текста
    private void UpdateInfoText()
    {
        if (!m_InfoText)
            return;
        
        if (m_IsFreeType)
        {
            m_InfoText.SetText("Free roulette - Next spin available soon");
        }
        else
        {
            m_InfoText.SetText("Paid roulette - Cost: $50");
        }
    }
    
    // Запрос спина на сервер
    private void RequestSpin()
    {
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
        {
            ShowError("Player not found");
            return;
        }
        
        Print("[Roulette] Requesting spin, type: " + (m_IsFreeType ? "FREE" : "PAID"));
        
        ShowInfo("Requesting...");
        m_BtnSpin.Enable(false);
        
        ScriptRPC rpc = new ScriptRPC();
        
        if (m_IsFreeType)
        {
            rpc.Send(player, 22001, true, player.GetIdentity()); // REQUEST_FREE_SPIN
        }
        else
        {
            rpc.Send(player, 22002, true, player.GetIdentity()); // REQUEST_PAID_SPIN
        }
    }
    
    // Проверка данных от сервера
    private void CheckForServerData()
    {
        DayZPlayerImplement player = DayZPlayerImplement.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        
        // Проверяем результат спина
        if (player.HasPendingRouletteResult())
        {
            Print("[Roulette] Received spin result from server");
            
            array<string> classnames;
            array<string> names;
            array<string> rarities;
            player.ConsumePendingRouletteResult(classnames, names, rarities);
            
            if (classnames && names)
            {
                StartSpinAnimation(classnames, names);
            }
        }
        
        // Проверяем ошибки
        if (player.HasPendingRouletteError())
        {
            string errorMsg;
            player.ConsumePendingRouletteError(errorMsg);
            ShowError(errorMsg);
            m_BtnSpin.Enable(true);
        }
    }
    
    // Начало анимации спина
    private void StartSpinAnimation(array<string> classnames, array<string> names)
    {
        Print("[Roulette] Starting spin animation with " + names.Count() + " items");
        
        m_SpinSequence = classnames;
        m_SpinNames = names;
        m_SpinFrame = 0;
        m_SpinTimer = 0;
        m_SpinDelay = 100; // Начальная скорость
        m_IsSpinning = true;
        
        ShowInfo("SPINNING...");
    }
    
    // Обновление анимации
    private void UpdateSpinAnimation(float timeslice)
    {
        m_SpinTimer += timeslice * 1000; // в миллисекундах
        
        if (m_SpinTimer >= m_SpinDelay)
        {
            m_SpinTimer = 0;
            m_SpinFrame++;
            
            // Показываем 5 слотов начиная с текущего кадра
            UpdateSlots();
            
            // Постепенно замедляем
            if (m_SpinFrame > 10)
            {
                m_SpinDelay = 150;
            }
            if (m_SpinFrame > 15)
            {
                m_SpinDelay = 250;
            }
            if (m_SpinFrame > 20)
            {
                m_SpinDelay = 500;
            }
            
            // Останавливаемся на последнем (25-й кадр = приз)
            if (m_SpinFrame >= m_SpinSequence.Count())
            {
                StopSpinAnimation();
            }
        }
    }
    
    // Обновление слотов
    private void UpdateSlots()
    {
        int total = m_SpinSequence.Count();
        
        // Показываем 5 предметов: frame-2, frame-1, frame, frame+1, frame+2
        SetSlotText(m_Slot1Name, GetItemAt(m_SpinFrame - 2));
        SetSlotText(m_Slot2Name, GetItemAt(m_SpinFrame - 1));
        SetSlotText(m_Slot3Name, GetItemAt(m_SpinFrame));     // Центр = выигрыш
        SetSlotText(m_Slot4Name, GetItemAt(m_SpinFrame + 1));
        SetSlotText(m_Slot5Name, GetItemAt(m_SpinFrame + 2));
    }
    
    // Получить предмет на позиции (с зацикливанием)
    private string GetItemAt(int index)
    {
        int total = m_SpinNames.Count();
        if (total == 0)
            return "???";
        
        // Зацикливаем индекс
        while (index < 0)
            index += total;
        while (index >= total)
            index -= total;
        
        return m_SpinNames[index];
    }
    
    // Установить текст слота
    private void SetSlotText(TextWidget slot, string text)
    {
        if (slot)
        {
            slot.SetText(text);
        }
    }
    
    // Остановка анимации
    private void StopSpinAnimation()
    {
        Print("[Roulette] Spin animation stopped");
        
        m_IsSpinning = false;
        
        // Последний предмет = приз
        string prize = m_SpinNames[m_SpinNames.Count() - 1];
        
        ShowInfo("YOU WON: " + prize + "! Check Prizes menu");
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.EnableSpinButton, 3000, false);
    }
    
    // Включение кнопки спина
    private void EnableSpinButton()
    {
        if (m_BtnSpin && m_Root && m_Root.IsVisible())
        {
            m_BtnSpin.Enable(true);
            UpdateInfoText();
        }
    }
    
    // Показать информацию
    private void ShowInfo(string text)
    {
        if (m_InfoText)
        {
            m_InfoText.SetText(text);
            m_InfoText.SetColor(ARGB(255, 200, 200, 200));
        }
    }
    
    // Показать ошибку
    private void ShowError(string text)
    {
        if (m_InfoText)
        {
            m_InfoText.SetText("ERROR: " + text);
            m_InfoText.SetColor(ARGB(255, 255, 100, 100));
        }
    }
}