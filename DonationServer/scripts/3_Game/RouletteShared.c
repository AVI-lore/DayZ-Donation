// DonationServer/scripts/3_Game/Roulette/RouletteShared.c
// Общие классы и константы для рулетки

#ifndef ROULETTE_SHARED_DEFINED
#define ROULETTE_SHARED_DEFINED

// RPC константы для рулетки
class RouletteRPC
{
    // Client → Server
    static const int REQUEST_FREE_SPIN = 22001;
    static const int REQUEST_PAID_SPIN = 22002;
    static const int REQUEST_WINNINGS = 22003;
    static const int CLAIM_WINNING = 22004;
    static const int CLAIM_ALL_WINNINGS = 22005;
    
    // Server → Client
    static const int SEND_SPIN_RESULT = 22101;
    static const int SEND_WINNINGS_LIST = 22102;
    static const int CLAIM_RESULT = 22103;
    static const int ERROR_MESSAGE = 22104;
}

// Тип рулетки
enum RouletteType
{
    ROULETTE_FREE,
    ROULETTE_PAID
}

// Класс предмета рулетки
class RouletteItem
{
    string classname;
    string display_name;
    string rarity;
    int weight;
    
    void RouletteItem(string cls = "", string name = "", string rar = "common", int w = 1)
    {
        classname = cls;
        display_name = name;
        rarity = rar;
        weight = w;
    }
}

// Класс выигрыша
class RouletteWinning
{
    int id;
    string item_classname;
    string display_name;
    string roulette_type;
    string date;
    
    void RouletteWinning(int _id = 0, string cls = "", string name = "", string type = "", string dt = "")
    {
        id = _id;
        item_classname = cls;
        display_name = name;
        roulette_type = type;
        date = dt;
    }
}

#endif