class ActionSearchFurnitureCB : ActionContinuousBaseCB
{
	protected EffectSound m_FurnitureCustomSound;

	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(4.0); // Время обыска мебели — 4 секунды
	}

	override void InitActionComponent()
	{
		super.InitActionComponent();
		
		if (!m_ActionData || !m_ActionData.m_Player || !m_ActionData.m_Target) return;
		PlayerBase player = m_ActionData.m_Player;
		Object targetObj = m_ActionData.m_Target.GetObject();
		if (!targetObj) targetObj = m_ActionData.m_Target.GetParent();
		if (!targetObj) return;

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			vector worldHitPos = m_ActionData.m_Target.GetCursorHitPos();
			if (worldHitPos != vector.Zero)
			{
				vector localHitPos = targetObj.WorldToModel(worldHitPos);
				string matrixCategory = HUSHazardConfigHolder.GetFurnitureCategoryByVector(targetObj.GetType(), localHitPos);
				matrixCategory.ToLower();

				// НАМЕРТВО ЗАФИКСИРОВАНО: refrigerator идет в чистый мебельный саундсет!
				if (matrixCategory == "refrigerator")
				{
					m_FurnitureCustomSound = SEffectManager.PlaySoundOnObject("HH_Fridge_Search_SoundSet", player);
				}
				else
				{
					// На всех остальных точках стен пока играет чистый звук багажника
					m_FurnitureCustomSound = SEffectManager.PlaySoundOnObject("action_interact_SoundSet", player);
				}


				if (m_FurnitureCustomSound)
				{
					m_FurnitureCustomSound.SetSoundAutodestroy(true);
					m_FurnitureCustomSound.SetSoundLoop(true);
					m_FurnitureCustomSound.SetSoundVolume(3.5);
				}
			}
		}
	}

	override void OnFinish(bool pCanceled)
	{
		super.OnFinish(pCanceled);
		if (m_FurnitureCustomSound) 
		{
			m_FurnitureCustomSound.SoundStop();
		}
	}
};


class ActionSearchEngineWreckCB : ActionContinuousBaseCB
{
	protected EffectSound m_WrenchSound;

	override void CreateActionComponent()
	{
		// 4 секунды плавного разбора мотора стоя в полный рост
		m_ActionData.m_ActionComponent = new CAContinuousTime(4.0); 
	}

	override void InitActionComponent()
	{
		super.InitActionComponent();
		
		PlayerBase player = m_ActionData.m_Player;
		if (!player) return;

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			// Принудительно запускаем оригинальный сочный звук гаечного ключа из config.cpp
			m_WrenchSound = SEffectManager.PlaySoundOnObject("HH_Wrench_Loop_SoundSet", player);
			if (m_WrenchSound)
			{
				m_WrenchSound.SetSoundAutodestroy(true);
				m_WrenchSound.SetSoundLoop(true);   // Зацикливание скрежета
				m_WrenchSound.SetSoundVolume(3.5); // Сочная громкость
			}
		}
	}

	override void OnFinish(bool pCanceled)
	{
		super.OnFinish(pCanceled);
		if (m_WrenchSound) 
		{
			m_WrenchSound.SoundStop(); // Чистая остановка звуковой петли ключа при отмене/финише
		}
	}
};


class ActionSearchTrunkWreckCB : ActionContinuousBaseCB
{
	protected EffectSound m_TrunkSound;

	override void CreateActionComponent()
	{
		// 1.5 секунды для идеального одиночного стоячего цикла рук
		m_ActionData.m_ActionComponent = new CAContinuousTime(1.5); 
	}

	override void InitActionComponent()
	{
		super.InitActionComponent();
		
		PlayerBase player = m_ActionData.m_Player;
		if (!player) return;

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			// ПРИНУДИТЕЛЬНЫЙ ЗАПУСК ПО ТЗ: Включаем ваш кастомный звук из config.cpp поверх немой анимации!
			m_TrunkSound = SEffectManager.PlaySoundOnObject("action_interact_SoundSet", player);
			if (m_TrunkSound)
			{
				m_TrunkSound.SetSoundAutodestroy(true);
				m_TrunkSound.SetSoundLoop(true);   // Зацикливание эффекта
				m_TrunkSound.SetSoundVolume(3.5); // Сочная, отчетливая громкость
			}
		}
	}

	override void OnFinish(bool pCanceled)
	{
		super.OnFinish(pCanceled);
		if (m_TrunkSound) 
		{
			m_TrunkSound.SoundStop(); // Чистая остановка звуковой петли багажника при отмене/финише
		}
	}
};





// ----------------------------------------------------------------------------
// ЧАСТЬ 1: УЛИЧНЫЙ СЕКТОР (КУРЯТНИКИ, БУДКИ, СУХИЕ ТУАЛЕТЫ)
// ----------------------------------------------------------------------------
class ActionSearchHazard : ActionContinuousBase
{
	void ActionSearchHazard()
	{
		m_CallbackClass = ActionSearchFurnitureCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
		m_FullBody = true;
		m_Text = "Обыскать";
		m_LockTargetOnUse = false;
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
		m_ConditionItem = new CCINone();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Object targetObj = target.GetObject();
		if (!targetObj) targetObj = target.GetParent();
		if (!targetObj) return false;

		string typeName = targetObj.GetType();
		typeName.ToLower();

		if (typeName.Contains("toilet") || typeName.Contains("kennel") || typeName.Contains("dog") || typeName.Contains("coop") || typeName.Contains("chicken"))
		{
			string uniqueCooldownKey = targetObj.GetID().ToString() + "_street";
			if (ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Contains(uniqueCooldownKey))
			{
				if (GetGame().GetTime() < ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Get(uniqueCooldownKey))
					return false;
			}
			return true;
		}
		return false;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);
		if (!action_data || !action_data.m_Player) return;

		PlayerBase player = action_data.m_Player;
		Object targetObj = action_data.m_Target.GetObject();
		if (!targetObj) targetObj = action_data.m_Target.GetParent();
		if (!targetObj) return;

		string typeName = targetObj.GetType();
		typeName.ToLower();

		// НАКАЗАНИЕ НА СЕРВЕРЕ ПРИ ОБЫСКЕ УЛИЦЫ БЕЗ ПЕРЧАТОК
		EntityAI gloves = player.GetInventory().FindAttachment(InventorySlots.GLOVES);
		if (!gloves || gloves.IsRuined())
		{
			player.SetBloodyHands(true);
			if (player.GetBleedingManagerServer())
			{
				player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("LeftArm");
				player.MessageAction("[HUSHazard]: Ай! Вы сильно порезали незащищенную руку о занозу!");
			}
		}
		else
		{
			gloves.DecreaseHealth("", "", 6.0);
		}

		string targetCategory = typeName;
		if (targetCategory.Contains("toilet"))       targetCategory = "toilet_dry";
		else if (targetCategory.Contains("kennel") || targetCategory.Contains("dog")) targetCategory = "dog_kennel";
		else if (targetCategory.Contains("coop")   || targetCategory.Contains("chicken")) targetCategory = "chicken_coop";

		HUSHazardServerManager.ProcessSearch(player, targetObj, targetCategory);
		
		string uniqueCooldownKey = targetObj.GetID().ToString() + "_street";
		ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Set(uniqueCooldownKey, GetGame().GetTime() + 3600000);
		
		ref Param1<string> rpcKeyParam = new Param1<string>(uniqueCooldownKey);
		GetGame().RPCSingleParam(player, 95202, rpcKeyParam, true, player.GetIdentity());
	}
};
// ============================================================================
// ЧАСТЬ 2: СНАЙПЕРСКИЙ ОБЫСК МЕБЕЛИ (ИСПРАВЛЕНО — СТЕРИЛЬНЫЙ ТЕКСТОВЫЙ ОБХОД)
// ============================================================================
class ActionSearchFurniture : ActionContinuousBase
{
	static ref map<string, int> m_HH_GlobalFurnitureCooldowns = new map<string, int>();
	const int FURNITURE_COOLDOWN_TIME = 3600;
	protected EffectSound m_FurnitureCustomSound;

	void ActionSearchFurniture()
	{
		m_CallbackClass = ActionSearchFurnitureCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT; // Ваша стабильная стоячая анимация рук
		m_FullBody = true; // Заполняет прогресс-бар без обрывов
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_Text = "Обыскать"; 
		m_LockTargetOnUse = false; 
	}
	
	override int GetActionCategory() { return AC_INTERACT; }
	override bool CanBeUsedOnBack() { return false; }
	override bool IsLockTargetOnUse() { return false; }
	override typename GetInputType() { return ContinuousInteractActionInput; }
	override bool HasTarget() { return true; }

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
		m_ConditionItem = new CCINone();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		HumanMovementState movementState = new HumanMovementState();
		player.GetMovementState(movementState);
		if (movementState.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_PRONE) return false;

		Object targetObj = target.GetObject();
		if (!targetObj) targetObj = target.GetParent();
		if (!targetObj) return false;

		string typeName = targetObj.GetType(); typeName.ToLower();
		if (typeName.Contains("zmb") || typeName.Contains("corpse")) return false;
		if (typeName.Contains("wreck") || typeName.Contains("volha") || typeName.Contains("offroad")) return false;

		if (targetObj.IsInherited(House) || targetObj.IsInherited(Building))
		{
			int compIdx = target.GetComponentIndex();
			if (compIdx != -1)
			{
				string uniqueCooldownKey = targetObj.GetID().ToString() + "_" + compIdx.ToString();
				int currentTime = GetGame().GetTime();

				if (m_HH_GlobalFurnitureCooldowns.Contains(uniqueCooldownKey))
				{
					int cooldownEndTime = m_HH_GlobalFurnitureCooldowns.Get(uniqueCooldownKey);
					if (currentTime < cooldownEndTime) return false;
				}

				vector worldHitPos = target.GetCursorHitPos();
				if (worldHitPos != vector.Zero)
				{
					vector localHitPos = targetObj.WorldToModel(worldHitPos);
					string matrixCategory = HUSHazardConfigHolder.GetFurnitureCategoryByVector(targetObj.GetType(), localHitPos);
					if (matrixCategory != string.Empty) 
					{
						if (!HUSHazardServerManager.PlayerHasValidToolForCategory(matrixCategory, item)) return false;
						return true;
					}
				}
			}
		}
		return false;
	}

	// НАДЕЖНЫЙ СТАРТ ЗВУКА НА КЛИЕНТЕ: Воспроизводится без веток!
	override void OnStartClient(ActionData action_data)
	{
		super.OnStartClient(action_data);
		if (action_data.m_Player && action_data.m_Target && (GetGame().IsClient() || !GetGame().IsMultiplayer()))
		{
			Object targetObj = action_data.m_Target.GetObject();
			if (!targetObj) targetObj = action_data.m_Target.GetParent();
			if (!targetObj) return;

			vector worldHitPos = action_data.m_Target.GetCursorHitPos();
			if (worldHitPos != vector.Zero)
			{
				vector localHitPos = targetObj.WorldToModel(worldHitPos);
				string matrixCategory = HUSHazardConfigHolder.GetFurnitureCategoryByVector(targetObj.GetType(), localHitPos);
				matrixCategory.ToLower();

				// Проверяем ваш точный маркер refrigerator из дебаг-лога
				if (matrixCategory == "refrigerator" || targetObj.GetType().Contains("fridge"))
				{
					m_FurnitureCustomSound = SEffectManager.PlaySoundOnObject("HH_Fridge_Search_SoundSet", action_data.m_Player);
				}
				else
				{
					// Откат по умолчанию на звук багажника для остальных зон
					m_FurnitureCustomSound = SEffectManager.PlaySoundOnObject("HH_Trunk_Search_SoundSet", action_data.m_Player);
				}

				if (m_FurnitureCustomSound)
				{
					m_FurnitureCustomSound.SetSoundAutodestroy(true);
					m_FurnitureCustomSound.SetSoundLoop(true);
					m_FurnitureCustomSound.SetSoundVolume(4.0); // Громкий, сочный звук
				}
			}
		}
	}

	override void OnEndClient(ActionData action_data)
	{
		super.OnEndClient(action_data);
		if (m_FurnitureCustomSound)
		{
			m_FurnitureCustomSound.SoundStop(); // Гашение звуковой петли
		}
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		PlayerBase player = action_data.m_Player;
		Object targetObj = action_data.m_Target.GetObject();
		if (!targetObj) targetObj = action_data.m_Target.GetParent();
		if (!player || !targetObj) return;

		int compIdx = action_data.m_Target.GetComponentIndex();
		vector worldHitPos = action_data.m_Target.GetCursorHitPos();
		if (worldHitPos == vector.Zero || compIdx == -1) return;

		vector localHitPos = targetObj.WorldToModel(worldHitPos);
		string matrixCategory = HUSHazardConfigHolder.GetFurnitureCategoryByVector(targetObj.GetType(), localHitPos);
		if (matrixCategory == string.Empty) return;

		if (!HUSHazardServerManager.PlayerHasValidToolForCategory(matrixCategory, action_data.m_MainItem)) return;

		string uniqueCooldownKey = targetObj.GetID().ToString() + "_" + compIdx.ToString();
		if (m_HH_GlobalFurnitureCooldowns.Contains(uniqueCooldownKey))
		{
			int checkEndTime = m_HH_GlobalFurnitureCooldowns.Get(uniqueCooldownKey);
			if (GetGame().GetTime() < checkEndTime) return; 
		}

		int endTime = GetGame().GetTime() + (FURNITURE_COOLDOWN_TIME * 1000);
		m_HH_GlobalFurnitureCooldowns.Set(uniqueCooldownKey, endTime);

		bool bIsMedicalSafe = false;
		if (matrixCategory == "medical" || matrixCategory.Contains("safe") || matrixCategory.Contains("lock") || matrixCategory.Contains("cabinet")) bIsMedicalSafe = true;

		EntityAI gloves = player.GetInventory().FindAttachment(InventorySlots.GLOVES);
		if (!gloves || gloves.IsRuined())
		{
			if (bIsMedicalSafe)
			{
				if (Math.RandomFloat01() < 0.30) 
				{
					if (player.GetBleedingManagerServer())
					{
						player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("LeftArm");
						player.MessageAction("[HUSHazard]: Вы сильно порезали ладонь о металлические зазубрины взломанного замка аптечки!");
					}

					player.InsertAgent(eAgents.WOUND_AGENT, 100); 
					if (player.GetModifiersManager() && !player.GetModifiersManager().IsModifierActive(31))
					{
						player.GetModifiersManager().ActivateModifier(31); 
					}
					player.MessageAction("[HUSHazard]: В рану попала грязь. Развивается заражение крови!");
				}
			}
		}
		else
		{
			if (bIsMedicalSafe) gloves.DecreaseHealth("", "", 3.0); 
			else gloves.DecreaseHealth("", "", 6.0); 
		}

		HUSHazardServerManager.ProcessSearch(player, targetObj, matrixCategory);
	}
};



// ============================================================================
// ЧАСТЬ 3: АВТОМОБИЛЬНЫЙ СЕКТОР (КАПОТЫ И БАГАЖНИКИ ОСТОВОВ МАШИН — ФИНАЛ)
// ============================================================================

class ActionSearchEngineWreck : ActionContinuousBase
{
	void ActionSearchEngineWreck()
	{
		m_CallbackClass = ActionSearchEngineWreckCB;
		
		// ИСПРАВЛЕНО ПО ТЗ: Переключаем на красивую стоячую анимацию рук взаимодействия!
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT; 
		
		m_FullBody = true;
		m_Text = "Попытаться разобрать"; 
		m_LockTargetOnUse = false;
	}
	
	override typename GetInputType() { return ContinuousInteractActionInput; }
	override void CreateConditionComponents() { m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT); m_ConditionItem = new CCINone(); }

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!player || !player.GetHumanInventory()) return false;
		
		EntityAI itemInHands = player.GetHumanInventory().GetEntityInHands();
		if (!itemInHands || (itemInHands.GetType() != "PipeWrench" && !itemInHands.GetType().Contains("Wrench"))) return false; 

		Object targetObj = target.GetObject();
		if (!targetObj) targetObj = target.GetParent();
		if (!targetObj) return false;
		if (targetObj.IsKindOf("CarScript") || targetObj.IsKindOf("Car")) return false;

		string typeName = targetObj.GetType(); typeName.ToLower();
		if (typeName.Contains("wreck") || typeName.Contains("volha") || typeName.Contains("offroad"))
		{
			string uniqueCooldownKey = targetObj.GetID().ToString() + "_engine";
			if (ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Contains(uniqueCooldownKey))
			{
				if (GetGame().GetTime() < ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Get(uniqueCooldownKey)) return false;
			}
			vector modelPos = targetObj.WorldToModel(player.GetPosition());
			
			// НАМЕРТВО ЗАФИКСИРОВАНО В ПАМЯТИ: Строгий продольный индекс оси Z вектора капота!
			if (modelPos[2] < 0.0) return true; 
		}
		return false;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		if (!action_data || !action_data.m_Target || !action_data.m_Target.GetObject() || !action_data.m_MainItem) return;
		
		PlayerBase player = action_data.m_Player;
		Object targetObj = action_data.m_Target.GetObject();
		string uniqueCooldownKey = targetObj.GetID().ToString() + "_engine";

		action_data.m_MainItem.DecreaseHealth("", "", 4.0);

		EntityAI gloves = player.GetInventory().FindAttachment(InventorySlots.GLOVES);
		if (!gloves || gloves.IsRuined())
		{
			if (Math.RandomFloat01() < 0.30)
			{
				if (player.GetBleedingManagerServer())
				{
					if (Math.RandomIntInclusive(0, 1) == 0)
					{
						if (!player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("LeftForeArmRoll"))
							player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("RightForeArmRoll");
					}
					else if (!player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("RightForeArmRoll"))
					{
						player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("LeftForeArmRoll");
					}
					
					player.MessageAction("[HUSHazard]: Вы сильно распороли ладонь о ржавые детали моторного отсека!");
				}

				player.InsertAgent(eAgents.WOUND_AGENT, 20); 
				if (player.GetModifiersManager() && !player.GetModifiersManager().IsModifierActive(31))
				{
					player.GetModifiersManager().ActivateModifier(31); 
				}
				player.MessageAction("[HUSHazard]: Грязь и моторное масло попали в кровь. Рана начинает стремительно гноиться!");
			}
		}
		else
		{
			gloves.DecreaseHealth("", "", 2.0);
		}

		HUSHazardServerManager.ProcessSearch(player, targetObj, "wreck_engine");
		ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Set(uniqueCooldownKey, GetGame().GetTime() + 3600000);
	}
};


class ActionSearchTrunkWreck : ActionContinuousBase
{
	void ActionSearchTrunkWreck()
	{
		m_CallbackClass = ActionSearchTrunkWreckCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT; 
		m_FullBody = true; 
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_Text = "Обыскать багажник";
		m_LockTargetOnUse = false;
	}

	override void CreateConditionComponents()  
	{
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
		m_ConditionItem = new CCINotPresent(); // Доступно строго с пустыми руками!
	}

	override typename GetInputType() { return ContinuousInteractActionInput; }

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Object targetObj = target.GetObject();
		if (!targetObj) targetObj = target.GetParent();
		if (!targetObj) return false;
		if (targetObj.IsKindOf("CarScript") || targetObj.IsKindOf("Car")) return false;

		string typeName = targetObj.GetType(); typeName.ToLower();
		if (typeName.Contains("wreck") || typeName.Contains("volha") || typeName.Contains("offroad"))
		{
			string uniqueCooldownKey = targetObj.GetID().ToString() + "_trunk";
			if (ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Contains(uniqueCooldownKey))
			{
				if (GetGame().GetTime() < ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Get(uniqueCooldownKey)) return false;
			}

			if (!HUSHazardServerManager.PlayerHasValidToolForCategory("wreck_trunk", item)) return false;

			vector modelPos = targetObj.WorldToModel(player.GetPosition());
			
			// НАМЕРТВО И СТРОГО ЗАФИКСИРОВАНО: Продольный индекс оси Z вектора багажника!
			if (modelPos[2] >= 0.0) return true;
		}
		return false;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{	
		if (!action_data || !action_data.m_Target || !action_data.m_Target.GetObject()) return;
		
		PlayerBase player = action_data.m_Player;
		Object targetObj = action_data.m_Target.GetObject();
		string uniqueCooldownKey = targetObj.GetID().ToString() + "_trunk";

		if (!HUSHazardServerManager.PlayerHasValidToolForCategory("wreck_trunk", action_data.m_MainItem)) return;

		EntityAI gloves = player.GetInventory().FindAttachment(InventorySlots.GLOVES);
		if (!gloves || gloves.IsRuined())
		{
			if (Math.RandomFloat01() < 0.30)
			{
				if (player.GetBleedingManagerServer())
				{
					if (Math.RandomIntInclusive(0, 1) == 0)
					{
						if (!player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("LeftForeArmRoll"))
							player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("RightForeArmRoll");
					}
					else if (!player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("RightForeArmRoll"))
					{
						player.GetBleedingManagerServer().AttemptAddBleedingSourceBySelection("LeftForeArmRoll");
					}
					
					player.MessageAction("[HUSHazard]: Вы глубоко распороли ладонь об острый край ржавого багажника!");
				}

				player.InsertAgent(eAgents.WOUND_AGENT, 20); 
				if (player.GetModifiersManager() && !player.GetModifiersManager().IsModifierActive(31))
				{
					player.GetModifiersManager().ActivateModifier(31); 
				}
				player.MessageAction("[HUSHazard]: Ржавчина попала в кровь. Развивается острое заражение раны!");
			}
		}
		else
		{
			gloves.DecreaseHealth("", "", 2.0);
		}

		HUSHazardServerManager.ProcessSearch(player, targetObj, "wreck_trunk");
		ActionSearchFurniture.m_HH_GlobalFurnitureCooldowns.Set(uniqueCooldownKey, GetGame().GetTime() + 3600000);
	}
};

// ----------------------------------------------------------------------------
// ЧАСТЬ 4: ОБЫСК ТРУПОВ ЗОМБИ (ОРИГИНАЛЬНАЯ РАБОЧАЯ СИСТЕМА TrinityPVE)
// ----------------------------------------------------------------------------
class ActionSearchZombieCB : ActionContinuousBaseCB
{
	protected EffectSound m_SearchSoundLoop;
	override void CreateActionComponent() { m_ActionData.m_ActionComponent = new CAContinuousTime(5.0); }
	override void InitActionComponent()
	{
		super.InitActionComponent();
		if (GetGame().IsClient() || !GetGame().IsMultiplayer()) m_SearchSoundLoop = SEffectManager.PlaySoundOnObject("HH_Zombie_Search_SoundSet", m_ActionData.m_Player);
	}
	override void OnFinish(bool pCanceled)
	{
		super.OnFinish(pCanceled);
		if (m_SearchSoundLoop) m_SearchSoundLoop.SoundStop();
	}
};

class ActionSearchZombie : ActionContinuousBase
{
	void ActionSearchZombie()
	{
		m_CallbackClass = ActionSearchZombieCB; m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_CRAFTING; m_FullBody = true; m_Text = "Search Body (Hazard)"; m_LockTargetOnUse = false; 
	}
	override bool CanBeUsedOnBack() { return false; }
	override bool IsLockTargetOnUse() { return false; }
	override typename GetInputType() { return ContinuousInteractActionInput; }
	override bool HasTarget() { return true; }
	override void CreateConditionComponents() { m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT); m_ConditionItem = new CCINone(); }
	
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		HumanMovementState movementState = new HumanMovementState(); player.GetMovementState(movementState);
		if (movementState.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_PRONE) return false;
		Object targetObj = target.GetObject(); if (!targetObj) targetObj = target.GetParent(); if (!targetObj || targetObj.IsAlive()) return false;
		string typeName = targetObj.GetType();
		if (typeName != string.Empty)
		{
			typeName.ToLower();
			if (typeName.Contains("zmb") || typeName.Contains("corpse"))
			{
				ZombieBase zBase = ZombieBase.Cast(targetObj); if (zBase && !zBase.m_HH_IsZombieSearched) return true; 
			}
		}
		return false;
	}
	
	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data); 
		PlayerBase player = action_data.m_Player; 
		ZombieBase zombie = ZombieBase.Cast(action_data.m_Target.GetObject());
		if (!zombie) zombie = ZombieBase.Cast(action_data.m_Target.GetParent()); 
		if (!player || !zombie) return;
		
		zombie.m_HH_IsZombieSearched = true; 
		ref Param p = new Param(); 
		GetGame().RPCSingleParam(zombie, 95201, p, true, player.GetIdentity());
		
		int glovesSlotId = InventorySlots.GetSlotIdFromString("Gloves"); 
		EntityAI glovesItem = player.GetInventory().FindAttachment(glovesSlotId);
		if (!glovesItem || glovesItem.IsDamageDestroyed())
		{
			PluginLifespan moduleLifespan = PluginLifespan.Cast(GetPlugin(PluginLifespan)); 
			if (moduleLifespan) moduleLifespan.UpdateBloodyHandsVisibility(player, true);
			player.InsertAgent(eAgents.BRAIN, 1000); 
			player.MessageAction("[HUSHazard]: Вы испачкали руки в зараженной крови!");
		}
		
		int maskSlotId = InventorySlots.GetSlotIdFromString("Mask"); 
		EntityAI maskItem = player.GetInventory().FindAttachment(maskSlotId);
		if (!maskItem || maskItem.IsDamageDestroyed())
		{
			
			player.GetSymptomManager().QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_VOMIT); 
			player.MessageAction("[HUSHazard]: Вдохнув трупные газы без маски, вас выворачивает наизнанку!");
		}
		player.MessageAction("[HUSHazard]: Обыск трупа зараженного завершен.");
	}
};
// ----------------------------------------------------------------------------
// ЧАСТЬ 5: ХАРДКОРНЫЙ БOЕВOЙ ФИЛЬТР ЗOМБИ И ДВОЙНОЙ РЕЕСТР (API DAYZ 1.29)
// ----------------------------------------------------------------------------
modded class DayZPlayerMeleeFightLogic_LightHeavy
{
	override void EvaluateHit_Common(InventoryItem weapon, Object target, bool forcedDummy = false, int forcedWeaponMode = -1)
	{
		if (!GetGame().IsServer() || !target)
		{
			super.EvaluateHit_Common(weapon, target, forcedDummy, forcedWeaponMode);
			return;
		}

		if (target.IsInherited(ZombieBase))
		{
			if (m_MeleeCombat.GetFinisherType() > -1)
			{
				super.EvaluateHit_Common(weapon, target, forcedDummy, forcedWeaponMode);
				return;
			}

			int currentHitZoneIdx = m_MeleeCombat.GetHitZoneIdx();
			string zoneName = target.GetActionComponentName(currentHitZoneIdx);
			string lowerZone = zoneName; lowerZone.ToLower();

			if (!weapon)
			{
				DamageSystem.CloseCombatDamage(m_Player, target, currentHitZoneIdx, "Dummy_Light", target.GetPosition(), ProcessDirectDamageFlags.NO_ATTACHMENT_TRANSFER);
				return;
			}

			if (weapon && lowerZone != "head" && lowerZone != "neck")
			{
				DamageSystem.CloseCombatDamage(m_Player, target, currentHitZoneIdx, "Dummy_Light", target.GetPosition(), ProcessDirectDamageFlags.NO_ATTACHMENT_TRANSFER);
				return;
			}

			if (lowerZone == "head" || lowerZone == "neck")
			{
				string itemType = weapon.GetType();
				if (itemType == "FirefighterAxe" || itemType == "WoodAxe" || itemType == "Hatchet" || itemType == "Sledgehammer" || itemType == "Pickaxe" || itemType == "Crowbar" || itemType == "PipeWrench" || itemType == "NailedBaseballBat" || itemType == "BarbedBaseballBat")
				{
					super.EvaluateHit_Common(weapon, target, forcedDummy, forcedWeaponMode);
					ZombieBase zomb = ZombieBase.Cast(target);
					if (zomb && zomb.IsAlive())
					{
						zomb.SetHealth("Head", "Health", 0.0);
						zomb.SetHealth("GlobalHealth", "Health", 0.0);
					}
					return;
				}
				
				DamageSystem.CloseCombatDamage(m_Player, target, currentHitZoneIdx, "MeleeFist", target.GetPosition(), ProcessDirectDamageFlags.NO_ATTACHMENT_TRANSFER);
				return;
			}
		}

		super.EvaluateHit_Common(weapon, target, forcedDummy, forcedWeaponMode);
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(ActionSearchHazard);     
		actions.Insert(ActionSearchFurniture);  
		actions.Insert(ActionSearchZombie);    
		actions.Insert(ActionSearchTrunkWreck);   
		actions.Insert(ActionSearchEngineWreck);  
	}
};

modded class PlayerBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionSearchHazard);
		AddAction(ActionSearchFurniture);
		AddAction(ActionSearchZombie);
		AddAction(ActionSearchTrunkWreck);   
		AddAction(ActionSearchEngineWreck);  
	}
}
