class CfgPatches
{
    class DonationServer
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};

class CfgMods
{
    class DonationServer
    {
        dir = "DonationServer";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "DonationServer";
        credits = "AVI";
        author = "AVI";
        authorID = "0";
        version = "1.0";
        extra = 0;
        type = "mod";
        
        dependencies[] = {"Game", "World", "Mission"};
        
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"DonationServer/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"DonationServer/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"DonationServer/scripts/5_Mission"};
            };
        };
    };
};