#include "loader.h"
#include "ScriptMgr.h"
#include "Configuration/Config.h"
#include "GossipDef.h"
#include "Player.h"
#include "ScriptedGossip.h"

enum Actions
{
    ACTION_CLOSE                 = 0,
    ACTION_TRANSFER_FROST       = 1001,
    ACTION_TRANSFER_TRIUMPH     = 1002,
    ACTION_TRANSFER_CONQUEST    = 1003
};

enum Items
{
    ITEM_EMBLEM_OF_FROST    = 49426,
    ITEM_EMBLEM_OF_TRIUMPH  = 47241,
    ITEM_EMBLEM_OF_CONQUEST = 45624
};

enum SenderMenu
{
    GOSSIP_SENDER_TRANSFER_FROST    = 1001,
    GOSSIP_SENDER_TRANSFER_TRIUMPH  = 1002,
    GOSSIP_SENDER_TRANSFER_CONQUEST = 1003
};

/*
 * How does this works?
 * 1) Select the type of emblem you want to transfer
 * 2) Select the character you want to transfer to (from your account)
 * 3) Input the amount of emblems to transfer
 */
class npc_emblem_transfer : public CreatureScript
{
public:
    npc_emblem_transfer() : CreatureScript("npc_emblem_transfer") { }

    // Step 1
    bool OnGossipHello(Player* player, Creature* creature) 
    {
        if (sConfigMgr->GetBoolDefault("EmblemTransfer.allowEmblemsFrost", true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "Transfer my Emblems of Frost", GOSSIP_SENDER_MAIN, ACTION_TRANSFER_FROST);

        if (sConfigMgr->GetBoolDefault("EmblemTransfer.allowEmblemsTriumph", false))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "Transfer my Emblems of Triumph", GOSSIP_SENDER_MAIN, ACTION_TRANSFER_TRIUMPH);

        if (sConfigMgr->GetBoolDefault("EmblemTransfer.allowEmblemsConquest", false))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "Transfer my Emblems of Conquest", GOSSIP_SENDER_MAIN, ACTION_TRANSFER_CONQUEST);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());

        return true;
    }

    // Step 2
    void SendCharactersList(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        uint32 minAmount = sConfigMgr->GetIntDefault("EmblemTransfer.minAmount", 10);

        // Get the character's emblems of the selected type
        uint32 emblems = 0;
        uint32 newSender = sender;
        switch (action)
        {
            case ACTION_TRANSFER_FROST:
                newSender = GOSSIP_SENDER_TRANSFER_FROST;
                emblems = player->GetItemCount(ITEM_EMBLEM_OF_FROST);
                break;
            case ACTION_TRANSFER_TRIUMPH:
                newSender = GOSSIP_SENDER_TRANSFER_TRIUMPH;
                emblems = player->GetItemCount(ITEM_EMBLEM_OF_TRIUMPH);
                break;
            case ACTION_TRANSFER_CONQUEST:
                newSender = GOSSIP_SENDER_TRANSFER_CONQUEST;
                emblems = player->GetItemCount(ITEM_EMBLEM_OF_CONQUEST);
                break;
        }

        if (emblems < minAmount)
        {
            player->GetSession()->SendNotification("You don't have enough emblems! The minimum amount is %d", minAmount);
            player->CLOSE_GOSSIP_MENU();
            return;
        }

        // Send characters list
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_GUID_NAME_BY_ACC);
        stmt->setUInt32(0, player->GetSession()->GetAccountId());
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result) {
            do
            {
                Field* characterFields  = result->Fetch();
                uint32 guid             = characterFields[0].GetUInt32();
                std::string name        = characterFields[1].GetString();

                if (!(guid == player->GetSession()->GetGuidLow()))
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, name, newSender, guid);
            } while (result->NextRow());
        }
    }

    // Step 3
    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
    {
        if (!isNumber(code))
        {
            player->GetSession()->SendNotification("Please enter a valid number!");
            return OnGossipSelect(player, creature, sender, ACTION_CLOSE);
        }

        int amount;
        std::stringstream ss(code);
        ss >> amount;

        uint32 emblemsCount = 0;
        uint32 emblemId = 0;
        uint32 newSender = sender;
        float penalty = sConfigMgr->GetFloatDefault("EmblemTransfer.penalty", 0.1f);

        switch (action)
        {
            case GOSSIP_SENDER_TRANSFER_FROST:
                emblemId = ITEM_EMBLEM_OF_FROST;
                break;
            case GOSSIP_SENDER_TRANSFER_TRIUMPH:
                emblemId = ITEM_EMBLEM_OF_TRIUMPH;
                break;
            case GOSSIP_SENDER_TRANSFER_CONQUEST:
                emblemId = ITEM_EMBLEM_OF_CONQUEST;
                break;
        }
        emblemsCount = player->GetItemCount(emblemId);

        if (emblemsCount < amount)
        {
            player->GetSession()->SendNotification("You don't have enough emblems!");
            return OnGossipSelect(player, creature, sender, ACTION_CLOSE);
        }

        Player* target = ObjectAccessor::FindPlayerInOrOutOfWorld(MAKE_NEW_GUID(action, 0, HIGHGUID_PLAYER));

        if (!target)
        {
            player->GetSession()->SendNotification("Character not found!");
            return OnGossipSelect(player, creature, sender, ACTION_CLOSE);
        }

        if (target->AddItem(emblemId, amount * (1.0f - penalty)))
        {
            player->DestroyItemCount(emblemId, -amount, true, false);
        }

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        if (action == ACTION_CLOSE)
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        player->PlayerTalkClass->ClearMenus();
        
        if (sender == GOSSIP_SENDER_MAIN)
        {
            SendCharactersList(player, creature, sender, action);
        } else {
            player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, "Last step!", sender, action, "Enter the amount of emblems to transfer:", 0, true);
        }

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool isNumber(const char* c)
    {
        const std::string s = c;
        // C++11
        return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
    }
};

void AddNpcEmblemTransferScripts()
{
    new npc_emblem_transfer();
}