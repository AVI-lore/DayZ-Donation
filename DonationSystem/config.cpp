class CfgPatches
{
    class DonationSystem
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};

class CfgMods
{
    class DonationSystem
    {
        dir = "DonationSystem";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "DonationSystem";
        credits = "AVI";
        author = "AVI";
        authorID = "0";
        version = "1.0";
        extra = 0;
        type = "mod";
        
        inputs = "DonationSystem/data/inputs.xml";
        
        dependencies[] = {"Game", "World", "Mission"};
        
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"DonationSystem/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"DonationSystem/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"DonationSystem/scripts/5_Mission"};
            };
        };
    };
};