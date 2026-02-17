#include "Common/AsciiString.h"
#include "Common/AudioSettings.h"
#include "Common/GameAudio.h"
#include "Common/UnicodeString.h"
#include "Common/UserPreferences.h"
#include "PreRTS.h"

// Stub implementation of OptionPreferences methods that are normally in
// OptionsMenu.cpp This is to break the dependency on the full GUI system for
// now.

OptionPreferences::OptionPreferences(void) {
  // loadFromIniFile(); // Skipping INI load for now in stub
}

OptionPreferences::~OptionPreferences() {}

Bool OptionPreferences::loadFromIniFile() { return FALSE; }

Real OptionPreferences::getMoneyTransactionVolume(void) const { return 1.0f; }
Real OptionPreferences::getSoundVolume(void) { return 1.0f; }
Real OptionPreferences::get3DSoundVolume(void) { return 1.0f; }
Real OptionPreferences::getSpeechVolume(void) { return 1.0f; }
Real OptionPreferences::getMusicVolume(void) { return 1.0f; }
AsciiString OptionPreferences::getPreferred3DProvider(void) {
  return "Miles Fast 2D Positional Audio";
}
AsciiString OptionPreferences::getSpeakerType(void) { return "Stereo"; }

Bool OptionPreferences::getArchiveReplaysEnabled() const { return FALSE; }
Bool OptionPreferences::getAlternateMouseModeEnabled(void) { return FALSE; }
Bool OptionPreferences::getRetaliationModeEnabled() { return FALSE; }
Bool OptionPreferences::getDoubleClickAttackMoveEnabled(void) { return FALSE; }
Real OptionPreferences::getScrollFactor(void) { return 1.0f; }
Bool OptionPreferences::getDrawScrollAnchor(void) { return FALSE; }
Bool OptionPreferences::getMoveScrollAnchor(void) { return FALSE; }
Bool OptionPreferences::getCursorCaptureEnabledInWindowedGame() const {
  return FALSE;
}
Bool OptionPreferences::getCursorCaptureEnabledInWindowedMenu() const {
  return FALSE;
}
Bool OptionPreferences::getCursorCaptureEnabledInFullscreenGame() const {
  return TRUE;
}
Bool OptionPreferences::getCursorCaptureEnabledInFullscreenMenu() const {
  return TRUE;
}
CursorCaptureMode OptionPreferences::getCursorCaptureMode() const { return 0; }
Bool OptionPreferences::getScreenEdgeScrollEnabledInWindowedApp() const {
  return FALSE;
}
Bool OptionPreferences::getScreenEdgeScrollEnabledInFullscreenApp() const {
  return TRUE;
}
ScreenEdgeScrollMode OptionPreferences::getScreenEdgeScrollMode() const {
  return 0;
}
Bool OptionPreferences::getSendDelay(void) { return FALSE; }
Int OptionPreferences::getFirewallBehavior(void) { return 0; }
Short OptionPreferences::getFirewallPortAllocationDelta(void) { return 0; }
UnsignedShort OptionPreferences::getFirewallPortOverride(void) { return 0; }
Bool OptionPreferences::getFirewallNeedToRefresh(void) { return FALSE; }
Bool OptionPreferences::usesSystemMapDir(void) { return FALSE; }
Bool OptionPreferences::saveCameraInReplays(void) { return FALSE; }
Bool OptionPreferences::useCameraInReplays(void) { return FALSE; }
Bool OptionPreferences::getPlayerObserverEnabled() const { return FALSE; }
Int OptionPreferences::getStaticGameDetail(void) { return 2; } // Normal
Int OptionPreferences::getIdealStaticGameDetail(void) { return 2; }
Real OptionPreferences::getGammaValue(void) { return 1.0f; }
Int OptionPreferences::getTextureReduction(void) { return 0; }
void OptionPreferences::getResolution(Int *xres, Int *yres) {
#ifdef RA_MACOS
  extern void MacOS_GetScreenSize(int &w, int &h);
  int w = 0, h = 0;
  MacOS_GetScreenSize(w, h);
  if (w > 0 && h > 0) {
    if (xres)
      *xres = w;
    if (yres)
      *yres = h;
    return;
  }
#endif
  if (xres)
    *xres = 800;
  if (yres)
    *yres = 600;
}
Bool OptionPreferences::get3DShadowsEnabled(void) { return TRUE; }
Bool OptionPreferences::get2DShadowsEnabled(void) { return TRUE; }
Bool OptionPreferences::getCloudShadowsEnabled(void) { return TRUE; }
Bool OptionPreferences::getLightmapEnabled(void) { return TRUE; }
Bool OptionPreferences::getSmoothWaterEnabled(void) { return TRUE; }
Bool OptionPreferences::getTreesEnabled(void) { return TRUE; }
Bool OptionPreferences::getExtraAnimationsDisabled(void) { return FALSE; }
Bool OptionPreferences::getUseHeatEffects(void) { return TRUE; }
Bool OptionPreferences::getDynamicLODEnabled(void) { return TRUE; }
Bool OptionPreferences::getFPSLimitEnabled(void) { return TRUE; }
Bool OptionPreferences::getNoDynamicLODEnabled(void) { return FALSE; }
Bool OptionPreferences::getBuildingOcclusionEnabled(void) { return TRUE; }
Int OptionPreferences::getParticleCap(void) { return 1000; }
Int OptionPreferences::getCampaignDifficulty(void) { return 1; }
void OptionPreferences::setCampaignDifficulty(Int diff) {}
Int OptionPreferences::getNetworkLatencyFontSize(void) { return 10; }
Int OptionPreferences::getRenderFpsFontSize(void) { return 10; }
Int OptionPreferences::getSystemTimeFontSize(void) { return 10; }
Int OptionPreferences::getGameTimeFontSize(void) { return 10; }
Int OptionPreferences::getPlayerInfoListFontSize(void) { return 10; }
Real OptionPreferences::getResolutionFontAdjustment(void) { return 1.0f; }
Bool OptionPreferences::getShowMoneyPerMinute(void) const { return FALSE; }

UnsignedInt OptionPreferences::getLANIPAddress(void) { return 0; }
UnsignedInt OptionPreferences::getOnlineIPAddress(void) { return 0; }
void OptionPreferences::setLANIPAddress(AsciiString IP) {}
void OptionPreferences::setOnlineIPAddress(AsciiString IP) {}
void OptionPreferences::setLANIPAddress(UnsignedInt IP) {}
void OptionPreferences::setOnlineIPAddress(UnsignedInt IP) {}
