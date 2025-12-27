// DonationServer/scripts/3_Game/DonationServer/DonationConfig.c

class DonationConfig
{
    // API настройки - ИСПРАВЛЕНО: HTTP вместо HTTPS
    static const string API_URL = "http://192.168.88.5/api";
    static const string API_KEY = "91Xchm3QatDHDeJCBjfDEZMeiYVq1NrC";
    
    // Эндпоинты
    static const string EP_PENDING_TO_VAULT = "pending_to_vault.php";
    static const string EP_GET_VAULT = "get_vault.php";
    static const string EP_CLAIM_ITEM = "claim_item.php";
    static const string EP_GET_HISTORY = "get_history.php";
    static const string EP_CONFIRM_ITEM = "confirm_item.php";
    
    // Настройки
    static const int HTTP_TIMEOUT = 10;
    static const bool DEBUG_MODE = true;
    
    // Построить ПОЛНЫЙ URL
    static string BuildURL(string endpoint, string steam64, int purchase_id = 0)
    {
        string url = API_URL + "/" + endpoint + "?api_key=" + API_KEY + "&steam64_id=" + steam64;
        
        if (purchase_id > 0)
            url += "&purchase_id=" + purchase_id;
        
        return url;
    }
    
    static void Log(string msg)
    {
        if (DEBUG_MODE)
            Print("[DonationServer] " + msg);
    }
}