/*
Copyright (C) 2004-2008 Christian Wieninger

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html

The author can be reached at cwieninger@gmx.de

The project's page is at http://winni.vdr-developer.org/epgsearch
*/

#include <vector>

#include <vdr/interface.h>
#include <vdr/menu.h>
#include <vdr/plugin.h>

#include "menu_commands.h"
#include "menu_searchresults.h"
#include "menu_myedittimer.h"
#include "menu_search.h" 
#include "menu_searchedit.h" 
#include "epgsearchcfg.h"
#include "recdone.h"
#include "epgsearchtools.h"
#include "switchtimer.h"
#include "switchtimer_thread.h"
#include "blacklist.h"
#include "menu_blacklistedit.h"
#include "menu_deftimercheckmethod.h"
#include "menu_quicksearch.h"

// --- cMenuSearchCommands ---------------------------------------------------------

cMenuSearchCommands::cMenuSearchCommands(const char *Title, const cEvent* Event, bool DirectCall, cSearchExt* Search)
   :cOsdMenu(Title)
{
#if VDRVERSNUM >= 10728
  SetMenuCategory(mcCommand);
#endif

   directCall = DirectCall;
   SetHasHotkeys();
   LoadCommands();

   search = Search;
   event = Event;
   Add(new cOsdItem(hk(tr("Create searchtimer"))));
   Add(new cOsdItem(hk(tr("List searchtimers"))));
   Add(new cOsdItem(hk(tr("Search for repeats"))));
   //Add(new cOsdItem(hk(tr("Search in EPG"))));
   Add(new cOsdItem(hk(tr("List recordings of this broadcast"))));
   Add(new cOsdItem(hk(tr("Mark as 'already recorded'?"))));
   Add(new cOsdItem(hk(tr("Activate/deactivate automatic switch?"))));
   Add(new cOsdItem(hk(tr("Ignore this broadcast"))));
   for (cCommand *command = commands.First(); command; command = commands.Next(command))
      Add(new cOsdItem(hk(command->Title())));
   if (event)
   {
     cString szTitle = cString::sprintf("%s: %s", tr("EPG Commands"), event->Title());
     SetTitle(szTitle);
   }
   SetHelp(NULL, NULL, NULL, tr("Search"));
}

cMenuSearchCommands::~cMenuSearchCommands()
{
}

void cMenuSearchCommands::LoadCommands()
{
  const char* szLanguageCode = I18nLanguageCode(I18nCurrentLanguage());

   char *pstrSearchToken, *pptr;
   char *pstrSearch=strdup(szLanguageCode);
   pstrSearchToken=strtok_r(pstrSearch, ",", &pptr);
   bool bLoaded = false;
   while(pstrSearchToken) // may contain multiple code, e.g. 'ger,deu'
   {
     cString cmdFile = cString::sprintf("%s-%s.conf", ADDDIR(CONFIGDIR, "epgsearchcmds"), pstrSearchToken);
      if (access(cmdFile, F_OK) == 0)
      {
         commands.Load(cmdFile, true);
         bLoaded = true;
         break;
      }
      pstrSearchToken=strtok_r(NULL, ",", &pptr);
   }
   if (!bLoaded)
      commands.Load(AddDirectory(CONFIGDIR, "epgsearchcmds.conf"), true);
   free(pstrSearch);
}

eOSState cMenuSearchCommands::Switch(void)
{
   cChannel *channel = Channels.GetByChannelID(event->ChannelID(), true, true);
   if (channel && cDevice::PrimaryDevice()->SwitchChannel(channel, true))
      return osEnd;
   else
   {
      Skins.Message(mtInfo, trVDR("Can't switch channel!")); 
     return osContinue;
   }
}

eOSState cMenuSearchCommands::ExtendedSearch(void)
{
   return AddSubMenu(new cMenuEPGSearchExt());
}

eOSState cMenuSearchCommands::Record(void)
{
   if (!event) return osContinue;
#if REELVDR
   eTimerMatch timerMatch = tmNone;
#else
   int timerMatch = tmNone;
#endif
   cTimer* timer = Timers.GetMatch(event, &timerMatch);
   if (timerMatch == tmFull)
   {
      if (EPGSearchConfig.useVDRTimerEditMenu)
         return AddSubMenu(new cMenuEditTimer(timer));
      else
         return AddSubMenu(new cMenuMyEditTimer(timer, false, event, timer->Channel(), NULL));
   }

   timer = new cTimer(event);
   PrepareTimerFile(event, timer);
   cTimer *t = Timers.GetTimer(timer);

   if (EPGSearchConfig.onePressTimerCreation == 0 || t || (!t && event->StartTime() - (Setup.MarginStart+2) * 60 < time(NULL))) 
   {
      if (t)
      {
         delete timer;
         timer = t;
      }
      if (EPGSearchConfig.useVDRTimerEditMenu)
         return AddSubMenu(new cMenuEditTimer(timer, !t));
      else
         return AddSubMenu(new cMenuMyEditTimer(timer, !t, event));
   }
   else
   {
      string fullaux = "";
      string aux = "";
      if (event)
      {
         int bstart = event->StartTime() - timer->StartTime();
         int bstop = timer->StopTime() - event->EndTime();
         int checkmode = DefTimerCheckModes.GetMode(timer->Channel());
         aux = UpdateAuxValue(aux, "channel", NumToString(timer->Channel()->Number()) + " - " + CHANNELNAME(timer->Channel()));
         aux = UpdateAuxValue(aux, "update", checkmode);
         aux = UpdateAuxValue(aux, "eventid", event->EventID());
         aux = UpdateAuxValue(aux, "bstart", bstart);
         aux = UpdateAuxValue(aux, "bstop", bstop);
         fullaux = UpdateAuxValue(fullaux, "epgsearch", aux);
      }
#ifdef USE_PINPLUGIN
      aux = "";
      aux = UpdateAuxValue(aux, "protected", timer->FskProtection() ? "yes" : "no");
      fullaux = UpdateAuxValue(fullaux, "pin-plugin", aux);
#endif

      SetAux(timer, fullaux);
      if(!Timers.Add(timer))
            Skins.Message(mtError, trVDR("Could not add timer"));
      timer->Matches();
      Timers.SetModified();

      LogFile.iSysLog("timer %s added (active)", *timer->ToDescr());
      return osBack;
   }
   return osContinue;
}

eOSState cMenuSearchCommands::MarkAsRecorded(void)
{
   if (!event) return osContinue;
   if (!Interface->Confirm(tr("Mark as 'already recorded'?")))
      return osContinue;
   cTimer* dummyTimer = new cTimer(event);
   cMutexLock RecsDoneLock(&RecsDone);
   RecsDone.Add(new cRecDone(dummyTimer, event, search));
   RecsDone.Save();
   delete dummyTimer;
   return osBack;
}

eOSState cMenuSearchCommands::AddToSwitchList(void)
{
   if (!event) return osContinue;

   time_t now = time(NULL);
   if (now >= event->StartTime())
   {
      Skins.Message(mtError, tr("Already running!"));
      return osBack;
   }
   cSwitchTimer* switchTimer = SwitchTimers.InSwitchList(event);
   if (!switchTimer)
   {
      if (!Interface->Confirm(tr("Add to switch list?")))
	     return osContinue;
      cMutexLock SwitchTimersLock(&SwitchTimers);
      SwitchTimers.Add(new cSwitchTimer(event));
      SwitchTimers.Save();
      cSwitchTimerThread::Init(); // asure the thread is running
   }
   else
   {
      if (!Interface->Confirm(tr("Delete from switch list?")))
	     return osContinue;
      cMutexLock SwitchTimersLock(&SwitchTimers);
      SwitchTimers.Del(switchTimer);
      SwitchTimers.Save();
      if (SwitchTimers.Count() == 0)
	     cSwitchTimerThread::Exit();
   }
   return osBack;
}

eOSState cMenuSearchCommands::CreateSearchTimer(void)
{
   if (!event) return osContinue;

   cSearchExt* pNew = new cSearchExt;
   strcpy(pNew->search, event->Title());
   return AddSubMenu(new cMenuEditSearchExt(pNew, true, false, true /*open search timer list menu*/));
}

eOSState cMenuSearchCommands::CreateBlacklist(void)
{
   if (!event) return osContinue;

   cBlacklist* pNew = new cBlacklist;
   strcpy(pNew->search, event->Title());
   return AddSubMenu(new cMenuBlacklistEdit(pNew, true));
}

eOSState cMenuSearchCommands::Execute(void)
{
   int current = Current();
   if (current <= 7)
   {
      if (current == 0)
         return CreateSearchTimer();
      if (current == 1)
         return ExtendedSearch(); 
      if (current == 2)
         return AddSubMenu(new cMenuSearchResultsForQuery(event->Title(), true));
//      if (current == 3)
//         return AddSubMenu(new cMenuQuickSearch(new cSearchExt));
      if (current == 3)
         return AddSubMenu(new cMenuSearchResultsForRecs(event->Title()));
      if (current == 4)
         return MarkAsRecorded();
      if (current == 5)
         return AddToSwitchList();
      if (current == 6)
         return CreateBlacklist(); 
   }

   cCommand *command = commands.Get(current-7);
   if (command) {
      cString buffer;
      bool confirmed = true;
      if (command->Confirm()) {
	buffer = cString::sprintf("%s?", command->Title());
	confirmed = Interface->Confirm(buffer);
      }
      if (confirmed) {
	buffer = cString::sprintf("%s...", command->Title());
	Skins.Message(mtStatus, buffer);

	buffer = cString::sprintf("'%s' %ld %ld %d '%s' '%s'",
                  EscapeString(event->Title()).c_str(), 
                  event->StartTime(), 
                  event->EndTime(), 
                  ChannelNrFromEvent(event), 
                  EscapeString(Channels.GetByChannelID(event->ChannelID(), true, true)->Name()).c_str(), 
                  EscapeString(event->ShortText()?event->ShortText():"").c_str());
         const char *Result = command->Execute(buffer);
	Skins.Message(mtStatus, NULL);
	if (Result)
	  return AddSubMenu(new cMenuText(command->Title(), Result, fontFix));
	return osBack;
      }
   }
   return osContinue;
}

eOSState cMenuSearchCommands::ProcessKey(eKeys Key)
{
   bool hadSubmenu = HasSubMenu();
   eOSState state = cOsdMenu::ProcessKey(Key);

   // jump back to calling menu, if a command was called directly with key '1' .. '9'
   if (directCall && hadSubmenu && !HasSubMenu())
      return osBack;

   if (state == osUnknown) {
      switch (Key) {
         case kGreen:
         case kYellow: return osContinue;
         case kBlue: return AddSubMenu(new cMenuQuickSearch(new cSearchExt));

         case k1...k9:
         case kOk:
            return Execute();
         default:   break;
      }
   }
   return state;
}