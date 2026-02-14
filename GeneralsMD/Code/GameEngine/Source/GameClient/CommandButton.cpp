#define DEFINE_GUI_COMMAND_NAMES
#define DEFINE_COMMAND_OPTION_NAMES
#define DEFINE_WEAPONSLOTTYPE_NAMES
#define DEFINE_RADIUSCURSOR_NAMES

#include "Common/ActionManager.h"
#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/Upgrade.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Module/ProductionUpdate.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Object.h"
#include "PreRTS.h"

// Stub for TheControlBar if not linked
#if 0
ControlBar *TheControlBar = nullptr;
#endif

// CommandButton Implementation

const FieldParse CommandButton::s_commandButtonFieldParseTable[] = {
    {"Command", CommandButton::parseCommand, nullptr,
     offsetof(CommandButton, m_command)},
    {"Options", INI::parseBitString32, TheCommandOptionNames,
     offsetof(CommandButton, m_options)},
    {"Object", INI::parseThingTemplate, nullptr,
     offsetof(CommandButton, m_thingTemplate)},
    {"Upgrade", INI::parseUpgradeTemplate, nullptr,
     offsetof(CommandButton, m_upgradeTemplate)},
    {"WeaponSlot", INI::parseLookupList, TheWeaponSlotTypeNamesLookupList,
     offsetof(CommandButton, m_weaponSlot)},
    {"MaxShotsToFire", INI::parseInt, nullptr,
     offsetof(CommandButton, m_maxShotsToFire)},
    {"Science", INI::parseScienceVector, nullptr,
     offsetof(CommandButton, m_science)},
    {"SpecialPower", INI::parseSpecialPowerTemplate, nullptr,
     offsetof(CommandButton, m_specialPower)},
    {"TextLabel", INI::parseAsciiString, nullptr,
     offsetof(CommandButton, m_textLabel)},
    {"DescriptLabel", INI::parseAsciiString, nullptr,
     offsetof(CommandButton, m_descriptionLabel)},
    {"PurchasedLabel", INI::parseAsciiString, nullptr,
     offsetof(CommandButton, m_purchasedLabel)},
    {"ConflictingLabel", INI::parseAsciiString, nullptr,
     offsetof(CommandButton, m_conflictingLabel)},
    {"ButtonImage", INI::parseAsciiString, nullptr,
     offsetof(CommandButton, m_buttonImageName)},
    {"CursorName", INI::parseAsciiString, nullptr,
     offsetof(CommandButton, m_cursorName)},
    {"InvalidCursorName", INI::parseAsciiString, nullptr,
     offsetof(CommandButton, m_invalidCursorName)},
    {"ButtonBorderType", INI::parseLookupList,
     CommandButtonMappedBorderTypeNames,
     offsetof(CommandButton, m_commandButtonBorder)},
    {"RadiusCursorType", INI::parseIndexList, TheRadiusCursorNames,
     offsetof(CommandButton, m_radiusCursor)},
    {"UnitSpecificSound", INI::parseAudioEventRTS, nullptr,
     offsetof(CommandButton, m_unitSpecificSound)},
    {nullptr, nullptr, nullptr, 0}};

void CommandButton::parseCommand(INI *ini, void *instance, void *store,
                                 const void *userData) {
  const char *token = ini->getNextToken();
  Int i;
  for (i = 0; TheGuiCommandNames[i]; i++) {
    if (stricmp(TheGuiCommandNames[i], token) == 0) {
      GUICommandType *command = (GUICommandType *)store;
      *command = (GUICommandType)i;
      return;
    }
  }
  throw INI_INVALID_DATA;
}

CommandButton::CommandButton(void) {
  m_command = GUI_COMMAND_NONE;
  m_thingTemplate = nullptr;
  m_upgradeTemplate = nullptr;
  m_weaponSlot = PRIMARY_WEAPON;
  m_maxShotsToFire = 0x7fffffff;
  m_science.clear();
  m_specialPower = nullptr;
  m_buttonImage = nullptr;
  m_flashCount = 0;
  m_conflictingLabel.clear();
  m_cursorName.clear();
  m_descriptionLabel.clear();
  m_invalidCursorName.clear();
  m_name.clear();
  m_options = 0;
  m_purchasedLabel.clear();
  m_textLabel.clear();
  m_window = nullptr;
  m_commandButtonBorder = COMMAND_BUTTON_BORDER_NONE;
  m_next = nullptr;
  m_radiusCursor = RADIUSCURSOR_NONE;
}

CommandButton::~CommandButton(void) {}

Bool CommandButton::isValidRelationshipTarget(Relationship r) const {
  UnsignedInt mask = 0;
  if (r == ENEMIES)
    mask |= NEED_TARGET_ENEMY_OBJECT;
  else if (r == ALLIES)
    mask |= NEED_TARGET_ALLY_OBJECT;
  else if (r == NEUTRAL)
    mask |= NEED_TARGET_NEUTRAL_OBJECT;
  return (m_options & mask) != 0;
}

Bool CommandButton::isValidObjectTarget(const Player *sourcePlayer,
                                        const Object *targetObj) const {
  if (!sourcePlayer || !targetObj)
    return false;
  Relationship r = sourcePlayer->getRelationship(targetObj->getTeam());
  return isValidRelationshipTarget(r);
}

Bool CommandButton::isValidObjectTarget(const Object *sourceObj,
                                        const Object *targetObj) const {
  if (!sourceObj || !targetObj)
    return false;
  Relationship r = sourceObj->getRelationship(targetObj);
  return isValidRelationshipTarget(r);
}

Bool CommandButton::isValidToUseOn(const Object *sourceObj,
                                   const Object *targetObj,
                                   const Coord3D *targetLocation,
                                   CommandSourceType commandSource) const {
  if (m_upgradeTemplate) {
    ProductionUpdateInterface *pui =
        const_cast<Object *>(sourceObj)->getProductionUpdateInterface();
    if (pui) {
      const ProductionEntry *pe = pui->firstProduction();
      while (pe) {
        if (pe->getProductionUpgrade() != nullptr)
          return false;
        pe = pui->nextProduction(pe);
      }
      return sourceObj->affectedByUpgrade(m_upgradeTemplate) &&
             !sourceObj->hasUpgrade(m_upgradeTemplate);
    }
    return false;
  }

  if (BitIsSet(m_options, COMMAND_OPTION_NEED_OBJECT_TARGET) && !targetObj) {
    return false;
  }

  Coord3D pos;
  if (targetLocation) {
    pos.set(targetLocation);
  }

  if (BitIsSet(m_options, NEED_TARGET_POS) && !targetLocation) {
    if (targetObj) {
      pos.set(targetObj->getPosition());
    } else {
      return false;
    }
  }

  if (BitIsSet(m_options, COMMAND_OPTION_NEED_OBJECT_TARGET)) {
    return TheActionManager->canDoSpecialPowerAtObject(
        sourceObj, targetObj, commandSource, m_specialPower, m_options, false);
  }

  if (BitIsSet(m_options, NEED_TARGET_POS)) {
    return TheActionManager->canDoSpecialPowerAtLocation(
        sourceObj, &pos, commandSource, m_specialPower, nullptr, m_options,
        false);
  }

  return TheActionManager->canDoSpecialPower(sourceObj, m_specialPower,
                                             commandSource, m_options, false);
}

Bool CommandButton::isReady(const Object *sourceObj) const {
  SpecialPowerModuleInterface *mod =
      sourceObj->getSpecialPowerModule(m_specialPower);
  if (mod && mod->getPercentReady() == 1.0f)
    return true;

  if (m_upgradeTemplate && sourceObj->affectedByUpgrade(m_upgradeTemplate) &&
      !sourceObj->hasUpgrade(m_upgradeTemplate))
    return true;

  return false;
}

Bool CommandButton::isValidObjectTarget(const Drawable *source,
                                        const Drawable *target) const {
  return isValidObjectTarget(source ? source->getObject() : nullptr,
                             target ? target->getObject() : nullptr);
}

Bool CommandButton::isContextCommand() const {
  return BitIsSet(m_options, CONTEXTMODE_COMMAND);
}

void CommandButton::copyImagesFrom(const CommandButton *button,
                                   Bool markUIDirtyIfChanged) const {
  if (m_buttonImage != button->getButtonImage()) {
    m_buttonImage = button->getButtonImage();
    if (markUIDirtyIfChanged && TheControlBar) {
      TheControlBar->markUIDirty();
    }
  }
}

void CommandButton::copyButtonTextFrom(const CommandButton *button,
                                       Bool shortcutButton,
                                       Bool markUIDirtyIfChanged) const {
  Bool change = FALSE;
  if (shortcutButton) {
    if (button->getConflictingLabel().isNotEmpty() &&
        m_textLabel.compare(button->getConflictingLabel())) {
      m_textLabel = button->getConflictingLabel();
      change = TRUE;
    }
  } else {
    if (button->getTextLabel().isNotEmpty() &&
        m_textLabel.compare(button->getTextLabel())) {
      m_textLabel = button->getTextLabel();
      change = TRUE;
    }
  }
  if (button->getDescriptionLabel().isNotEmpty() &&
      m_descriptionLabel.compare(button->getDescriptionLabel())) {
    m_descriptionLabel = button->getDescriptionLabel();
    change = TRUE;
  }
  if (markUIDirtyIfChanged && change && TheControlBar) {
    TheControlBar->markUIDirty();
  }
}

// CommandSet Implementation
const FieldParse CommandSet::m_commandSetFieldParseTable[] = {
    {"1", CommandSet::parseCommandButton, (void *)nullptr,
     offsetof(CommandSet, m_command)},
    {"2", CommandSet::parseCommandButton, (void *)1,
     offsetof(CommandSet, m_command)},
    {"3", CommandSet::parseCommandButton, (void *)2,
     offsetof(CommandSet, m_command)},
    {"4", CommandSet::parseCommandButton, (void *)3,
     offsetof(CommandSet, m_command)},
    {"5", CommandSet::parseCommandButton, (void *)4,
     offsetof(CommandSet, m_command)},
    {"6", CommandSet::parseCommandButton, (void *)5,
     offsetof(CommandSet, m_command)},
    {"7", CommandSet::parseCommandButton, (void *)6,
     offsetof(CommandSet, m_command)},
    {"8", CommandSet::parseCommandButton, (void *)7,
     offsetof(CommandSet, m_command)},
    {"9", CommandSet::parseCommandButton, (void *)8,
     offsetof(CommandSet, m_command)},
    {"10", CommandSet::parseCommandButton, (void *)9,
     offsetof(CommandSet, m_command)},
    {"11", CommandSet::parseCommandButton, (void *)10,
     offsetof(CommandSet, m_command)},
    {"12", CommandSet::parseCommandButton, (void *)11,
     offsetof(CommandSet, m_command)},
    {"13", CommandSet::parseCommandButton, (void *)12,
     offsetof(CommandSet, m_command)},
    {"14", CommandSet::parseCommandButton, (void *)13,
     offsetof(CommandSet, m_command)},
    {"15", CommandSet::parseCommandButton, (void *)14,
     offsetof(CommandSet, m_command)},
    {"16", CommandSet::parseCommandButton, (void *)15,
     offsetof(CommandSet, m_command)},
    {"17", CommandSet::parseCommandButton, (void *)16,
     offsetof(CommandSet, m_command)},
    {"18", CommandSet::parseCommandButton, (void *)17,
     offsetof(CommandSet, m_command)},
    {nullptr, nullptr, nullptr, 0}};

void CommandSet::parseCommandButton(INI *ini, void *instance, void *store,
                                    const void *userData) {
  const char *token = ini->getNextToken();
  AsciiString commandButtonName = token;
  const CommandButton *commandButton = nullptr;

  if (TheControlBar) {
    commandButton = TheControlBar->findCommandButton(commandButtonName);
  }

  if (commandButton) {
    CommandButton **commands = (CommandButton **)store;
    Int index = (Int)(size_t)userData;
    commands[index] = (CommandButton *)commandButton;
  } else {
    if (stricmp(token, "NONE") != 0 &&
        stricmp(token, "<NONE>") != 0) // Allows NONE to map to no button
                                       // DEBUG_CRASH( ("Command button '%s' not
                                       // found in system.\n", token) );
      ;
  }
}
