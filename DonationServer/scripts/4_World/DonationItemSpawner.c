// DonationServer/scripts/4_World/DonationServer/DonationItemSpawner.c

class DonationItemSpawner
{
    static bool GiveItemToPlayer(PlayerBase player, string classname, out string error)
    {
        if (!player)
        {
            error = "Player not found";
            return false;
        }
        
        if (classname == "")
        {
            error = "Invalid classname";
            return false;
        }
        
        DonationConfig.Log("Giving " + classname + " to " + player.GetIdentity().GetName());
        
        // Если это машина - спавним через специальный метод
        if (GetGame().IsKindOf(classname, "CarScript"))
        {
            return SpawnVehicle(player, classname, error);
        }
        
        // Попытка создать в инвентаре
        EntityAI item = player.GetInventory().CreateInInventory(classname);
        
        if (item)
        {
            DonationConfig.Log("Item spawned in inventory successfully");
            return true;
        }
        
        // ИСПРАВЛЕНО: Если инвентарь полон - спавним на землю
        DonationConfig.Log("Inventory full, spawning on ground");
        
        vector pos = player.GetPosition();
        item = EntityAI.Cast(GetGame().CreateObjectEx(classname, pos, ECE_PLACE_ON_SURFACE));
        
        if (item)
        {
            DonationConfig.Log("Item spawned on ground successfully");
            return true;
        }
        
        // Если и это не сработало
        error = "Failed to spawn item";
        DonationConfig.Log("ERROR: Failed to spawn " + classname);
        return false;
    }

    static bool SpawnVehicle(PlayerBase player, string classname, out string error)
    {
        DonationConfig.Log("Spawning vehicle: " + classname);
        
        // Позиция в 6 метрах перед игроком
        vector dir = player.GetDirection();
        vector pos = player.GetPosition() + (dir * 6);
        pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
        
        DonationConfig.Log("Vehicle spawn position: " + pos.ToString());
        
        // Создаём машину
        Car car = Car.Cast(GetGame().CreateObjectEx(classname, pos, ECE_PLACE_ON_SURFACE));
        
        if (!car)
        {
            error = "Failed to create vehicle";
            DonationConfig.Log("ERROR: Failed to create vehicle object");
            return false;
        }
        
        DonationConfig.Log("Vehicle object created successfully");
        
        // ИСПРАВЛЕНО: Загружаем конфигурацию деталей из JSON
        string configPath = "$profile:Donation/VehiclesConfig.json";
        
        // Проверяем существование файла через FileHandle
        FileHandle file = OpenFile(configPath, FileMode.READ);
        
        if (file != 0)
        {
            DonationConfig.Log("Found VehiclesConfig.json, loading parts");
            CloseFile(file);
            
            // Загружаем конфигурацию
            ref map<string, ref array<string>> vConfig = new map<string, ref array<string>>;
            JsonFileLoader<map<string, ref array<string>>>.JsonLoadFile(configPath, vConfig);
            
            if (vConfig && vConfig.Contains(classname))
            {
                array<string> parts = vConfig.Get(classname);
                DonationConfig.Log("Found " + parts.Count() + " parts for " + classname);
                
                foreach (string part : parts)
                {
                    EntityAI attachment = car.GetInventory().CreateAttachment(part);
                    if (attachment)
                    {
                        DonationConfig.Log("Attached: " + part);
                    }
                    else
                    {
                        DonationConfig.Log("WARNING: Failed to attach " + part);
                    }
                }
            }
            else
            {
                DonationConfig.Log("WARNING: No configuration found for " + classname);
            }
        }
        else
        {
            DonationConfig.Log("WARNING: VehiclesConfig.json not found at " + configPath);
            DonationConfig.Log("Vehicle will spawn without custom parts");
        }
        
        // Заправляем машину
        DonationConfig.Log("Filling vehicle fluids");
        car.Fill(CarFluid.FUEL, car.GetFluidCapacity(CarFluid.FUEL));
        car.Fill(CarFluid.COOLANT, car.GetFluidCapacity(CarFluid.COOLANT));
        car.Fill(CarFluid.OIL, car.GetFluidCapacity(CarFluid.OIL));
        car.Fill(CarFluid.BRAKE, car.GetFluidCapacity(CarFluid.BRAKE));
        
        DonationConfig.Log("Vehicle spawned successfully with all fluids filled");
        
        return true;
    }
}