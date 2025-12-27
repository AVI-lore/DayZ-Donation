// DonationServer/scripts/3_Game/DonationShared.c
// ОБЩИЙ ФАЙЛ ДЛЯ КЛИЕНТА И СЕРВЕРА

// Защита от повторного объявления
#ifndef DONATION_SHARED_DEFINED
#define DONATION_SHARED_DEFINED

// RPC константы
class DS_RPC
{
    // Client → Server
    static const int REQUEST_VAULT = 21001;
    static const int CLAIM_ITEM = 21003;
    static const int REQUEST_HISTORY = 21005;
    
    // Server → Client
    static const int SEND_VAULT = 21002;
    static const int CLAIM_RESULT = 21004;
    static const int SEND_HISTORY = 21006;
}

// Классы данных
class DS_VaultItem
{
    int id;
    string classname;
    
    void DS_VaultItem(int _id = 0, string _classname = "")
    {
        id = _id;
        classname = _classname;
    }
}

class DS_HistoryItem
{
    int id;
    string classname;
    string date_claimed;
    
    void DS_HistoryItem(int _id = 0, string _classname = "", string _date = "")
    {
        id = _id;
        classname = _classname;
        date_claimed = _date;
    }
}

#endif