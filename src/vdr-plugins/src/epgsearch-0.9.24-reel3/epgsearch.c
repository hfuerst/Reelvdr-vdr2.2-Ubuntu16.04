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

/*
 * epgsearch.c: A plugin for the Video Disk Recorder
 *
 */

#include <vector>

#include <vdr/epg.h>
#include <vdr/menu.h>
#include <vdr/plugin.h>
#include <vdr/status.h>

#include <getopt.h>

#include "menu_event.h"
#include "menu_main.h"
#include "menu_whatson.h"
#include "epgsearchsetup.h"
#include "epgsearchcfg.h"
#include "epgsearchext.h"
#include "epgsearchcats.h"
#include "menu_searchresults.h"
#include "searchtimer_thread.h"
#include "switchtimer_thread.h"
#include "menu_myedittimer.h"
#include "menu_epgtimers.h"
#include "epgsearchtools.h"
#include "rcfile.h"
#include "changrp.h"
#include "recstatus.h"
#include "timerstatus.h"
#include "recdone.h"
#include "blacklist.h"
#include "menu_search.h"
#include "templatefile.h"
#include "menu_conflictcheck.h"
#include "epgsearch.h"
#include "conflictcheck_thread.h"
#include "uservars.h"
#include "mail.h"
#include "svdrpclient.h"
#include "noannounce.h"
#include "menu_deftimercheckmethod.h"
#include "timerdone.h"
#include "epgsearchservices.h"
#include "menu_quicksearch.h"
#include "menu_announcelist.h"
#include "confdloader.h"
#ifdef USEMYSQL
#include "epgsearchmysql.h"
#endif
#include <langinfo.h>

static const char VERSION[]        = "0.9.24-reel2";
static const char DESCRIPTION[]    = trNOOP("search the EPG for repeats and more");

// globals
char *ConfigDir = NULL;
bool reloadMenuConf = false;
int updateForced = 0;
bool isUTF8 = false;

// LogFile declaration and statics
cLogFile LogFile;
char *cLogFile::LogFileName = NULL;
int cLogFile::loglevellimit = 0;

// external SVDRPCommand
const char *cSVDRPClient::SVDRPSendCmd = "svdrpsend.pl";

cPluginEpgsearch::cPluginEpgsearch(void)
{
   showConflicts = false;
   showAnnounces = false;
   gl_recStatusMonitor = NULL;
   gl_timerStatusMonitor = NULL;
}

cPluginEpgsearch::~cPluginEpgsearch()
{
   delete gl_recStatusMonitor;
   delete gl_timerStatusMonitor;
   cSearchTimerThread::Exit();
   cSwitchTimerThread::Exit();
   cConflictCheckThread::Exit();
   if (ConfigDir) free(ConfigDir);
}

const char* cPluginEpgsearch::Version(void) 
{ 
   return VERSION; 
}

const char* cPluginEpgsearch::Description(void) 
{ 
   return tr(DESCRIPTION); 
}

const char *cPluginEpgsearch::CommandLineHelp(void)
{
   return "  -f file,  --svdrpsendcmd=file  the path to svdrpsend.pl for external\n"
      "                           SVDRP communitcation (default is internal\n"
      "                           communication)\n"
      "  -c path,  --config=path  to specify a specific config dir for epgsearch\n"
      "  -l file,  --logfile=file to specify a specific logfile for epgsearch\n"
      "  -v n,  --verbose=n       verbose level for logfile\n"
      "  -r,  --reloadmenuconf    reload epgsearchmenu.conf with plugin call\n"
      "  -m file,  --mailcmd=file path to an alternative mail script for mail\n"
      "                           notification\n";

}

const char *cPluginEpgsearch::MainMenuEntry(void)
{
   if (EPGSearchConfig.hidemenu)
      return NULL;
   if (isempty(EPGSearchConfig.mainmenuentry))
      return tr("TV-Guide (EPG)");
   else
      return EPGSearchConfig.mainmenuentry;   
}

bool cPluginEpgsearch::ProcessArgs(int argc, char *argv[])
{
   // Implement command line argument processing here if applicable.


   if (argc==5 && !strcmp(argv[0], "timermenu"))
   { // yes, this is an ugly hack!
      argv[1] = (char*) new cMenuMyEditTimer((cTimer*) argv[2], (bool) argv[3], (const cEvent*) argv[4]);
      return true;
   }

   if (argc==8 && !strcmp(argv[0], "searchepg"))
   {
      cSearchExt* SearchExt = new cSearchExt;
      strcpy(SearchExt->search,argv[2]);
      if (atoi(argv[3]) > 0)
      {
         SearchExt->useChannel = true;
         SearchExt->channelMin = Channels.GetByNumber(atoi(argv[3]));
         SearchExt->channelMax = Channels.GetByNumber(atoi(argv[3]));
      }
      SearchExt->mode = atoi(argv[4]);
      SearchExt->useTitle = atoi(argv[5]);
      SearchExt->useSubtitle = atoi(argv[6]);
      SearchExt->useDescription = atoi(argv[7]);
      argv[1] = (char*) new cMenuSearchResultsForSearch(SearchExt, cTemplFile::GetTemplateByName("MenuSearchResults"));
      return true;
   }

   static const struct option long_options[] = {
      { "svdrpsendcmd",   required_argument, NULL, 'f' },
      { "config",         required_argument, NULL, 'c' },
      { "logfile",        required_argument, NULL, 'l' },
      { "verbose",        required_argument, NULL, 'v' },
      { "mailcmd",        required_argument, NULL, 'm' },
      { "reloadmenuconf", no_argument, NULL, 'r' },
      { NULL }
   };

   int c=0,i=0;

   while ((c = getopt_long(argc, argv, "f:c:l:v:m:r", long_options, &i)) != -1) {
      switch (c) {

         case 'f': 
            cSVDRPClient::SVDRPSendCmd = optarg;
            EPGSearchConfig.useExternalSVDRP = 1;
            break;
         case 'c': 
            ConfigDir = strdup(optarg);
            break;
         case 'l': 
            cLogFile::LogFileName = optarg;
            break;
         case 'v': 
            cLogFile::loglevellimit = atoi(optarg);
            break;
         case 'r': 
            reloadMenuConf = true;
            break;
         case 'm': 
            cMailNotifier::MailCmd = optarg;
            break;
         default:  return false;
      }
   }

   if (EPGSearchConfig.useExternalSVDRP && access(cSVDRPClient::SVDRPSendCmd, F_OK) != 0)
   {
      LogFile.eSysLog("ERROR - can't find SVDRPSend script: '%s'", cSVDRPClient::SVDRPSendCmd);
      cSVDRPClient::SVDRPSendCmd = NULL;
   }

   return true;
}

bool cPluginEpgsearch::Service(const char *Id, void *Data)
{
#if VDRVERSNUM >= 10716
    if (strcmp(Id, "MenuMainHook-V1.0") == 0)
    {
        MenuMainHook_Data_V1_0 *mmh = (MenuMainHook_Data_V1_0 *)Data;
        switch(mmh->Function) 
        {
            case osSchedule: mmh->pResultMenu = (cOsdMenu*) MainMenuAction(); break;
                                              //RC: call cMenuSearchMain() direct?
#ifdef REELVDR
            case osActiveEvent : {
                //Do what is done in cMenuSearchMain::ShowSummary() in menu_main.c

                printf("cPluginEpgsearch::Service() osActiveEvent: \n");

                cChannel *currChannel = Channels.GetByNumber(cDevice::CurrentChannel());
                cSchedulesLock schedulesLock;

                const cSchedules *schedules = cSchedules::Schedules(schedulesLock);
                if (schedules) {

                    const cSchedule *Schedule = schedules->GetSchedule(currChannel);
                    if (Schedule){

                        const cEvent *currEvent = Schedule->GetPresentEvent();
                        if (currEvent){

                            // static since 'eventObject' is passed by reference
                            static cEventObjects eventObjects;
                            eventObjects.Clear();
                            eventObjects.Add(currEvent);

                            mmh->pResultMenu = new cMenuEventSearch(currEvent, eventObjects, SurfModeTime);
                        }// currEvent

                    }//Schedule

                }//schedules

            }//case osActiveEvent
                break;
#endif /*REELVDR*/

            case osTimers  : mmh->pResultMenu = new cMenuEpgTimers(); break;
            case osSearchtimers : 
                             mmh->pResultMenu = new cMenuEPGSearchExt(); break;
            default     : return false;
        }
        return true;
    }
    else
#endif

#ifdef USEMYSQL
    if(strcmp(Id, "Avahi MySQL sync") == 0)
    {
        if(Setup.ReelboxModeTemp == eModeClient) // Client only
        {
            SearchExts.SyncData();
            return true;
        }
        else
            return false;
    }
    else if(strcmp(Id, "Avahi MySQL enable") == 0)
    {
        if(Setup.ReelboxModeTemp == eModeClient) // Client only
        {
            SearchExts.Load(); // Clear before load DB
            SearchExts.LoadDB();
            return true;
        }
        else
            return false;
    }
    else if(strcmp(Id, "Avahi MySQL disable") == 0)
    {
        if(Setup.ReelboxModeTemp == eModeClient) // Client only
        {
            SearchExts.Load(); // Clear
            SearchExts.LastEventID = -1;
            return true;
        }
        else
            return false;
    }
    else if(strcmp(Id, "MultiRoom disable") == 0)
    {
	/** If reelvdr WAS in client mode discard timers list
         *            and load timer list from its own epgsearch.conf
         *   If reelvdr WAS in server mode save timers list to file
	 */
	
        // present mode
	if (Setup.ReelboxModeTemp == eModeStandalone) 
        {
            // previous mode
            switch(Setup.ReelboxMode) 
            {
                 case eModeClient: // clear and load older list because we are in standalone mode
                 {
		      SearchExts.LastEventID = -1;    
                       
                      // clears list and loads from file
		      SearchExts.Load(AddDirectory(CONFIGDIR, "epgsearch.conf"));
                 }
                 break;

                 case eModeServer: // save the list to the conf file
                 {
                     /* save timers.conf */
                     cString vdrConfigDir = cPlugin::ConfigDirectory();
                     // gives '.../vdr/plugins', so to get .../vdr/timers.conf
                     // add "/../timers.conf"
                     cString timersFile = AddDirectory(vdrConfigDir, "/../timers.conf");
                     Timers.Save(timersFile);
                     isyslog("(%s,%d) Multiroom disabled: timers list saved to '%s'",
                             __FILE__, __LINE__, *timersFile);

		     SearchExts.SetFilename(AddDirectory(CONFIGDIR, "epgsearch.conf"));
		     SearchExts.Save(); 
                 }
                 break;

                case eModeHotel: // fall through

                default:
                     isyslog("epgsearch (%s:%d): unhandled ReelboxMode %d", __FILE__, __LINE__, Setup.ReelboxMode);
            }
        }
         else 
            isyslog("epgsearch (%s:%d): multiroom disabled, still ReelboxModeTemp != eModeStandalone", __FILE__, __LINE__);
        
       return true;
    }
    else if(strcmp(Id, "MultiRoom enable") == 0)
    {
        if(Setup.ReelboxModeTemp == eModeClient) // Client only
        {
            SearchExts.Load(); // Clear
            SearchExts.LoadDB();
        }
        else if(Setup.ReelboxModeTemp == eModeServer) // Server only
        {
            SearchExts.SetFilename(NULL);
            // Add TimerSearchs to Database
            cTimerSearchMysql *TimerSearchMysql = new cTimerSearchMysql(MYSQLREELUSER, MYSQLREELPWD, "vdr");
            if(TimerSearchMysql->SetServer("localhost"))
            {
                cMutexLock SearchExtsLock(&SearchExts);
                cSearchExt *searchext = (cSearchExt*)SearchExts.First();
                while (searchext)
                {
                    TimerSearchMysql->InsertTimerSearch(searchext);
                    searchext = (cSearchExt *)searchext->Next();
                }
            }
            delete TimerSearchMysql;
        }
        return true;
    }
#endif //USEMYSQL
   if (strcmp(Id, "MainMenuHooksPatch-v1.0::osSchedule") == 0 && EPGSearchConfig.ReplaceOrgSchedule!=0) 
   {
      if (Data == NULL)
         return true;
      cOsdMenu **menu = (cOsdMenu**) Data;
      if (menu)
         *menu = (cOsdMenu*) MainMenuAction();
      return true;
   }
 
   if (strcmp(Id, "Epgsearch-search-v1.0") == 0) {
      if (Data == NULL)
         return true;
      cSearchExt* SearchExt = new cSearchExt;
      
      Epgsearch_search_v1_0* searchData = (Epgsearch_search_v1_0*) Data;
      searchData->pResultMenu = NULL;
      strcpy(SearchExt->search,searchData->query);
      if (searchData->channelNr > 0)
      {
         SearchExt->useChannel = true;
         SearchExt->channelMin = Channels.GetByNumber(searchData->channelNr);
         SearchExt->channelMax = Channels.GetByNumber(searchData->channelNr);
      }
      SearchExt->mode = searchData->mode;
      SearchExt->useTitle = searchData->useTitle;
      SearchExt->useSubtitle = searchData->useSubTitle;
      SearchExt->useDescription = searchData->useDescription;
      searchData->pResultMenu = new cMenuSearchResultsForSearch(SearchExt, cTemplFile::GetTemplateByName("MenuSearchResults"));

      return true;
   }
  if (strcmp(Id, "Epgsearch-exttimers-v1.0") == 0) {
     if (Data == NULL)
        return true;
      Epgsearch_exttimers_v1_0* serviceData = (Epgsearch_exttimers_v1_0*) Data;
      serviceData->pTimersMenu = new cMenuEpgTimers();

      return true;
  }
   if (strcmp(Id, "Epgsearch-exttimeredit-v1.0") == 0) {
      if (Data == NULL)
         return true;
      
      Epgsearch_exttimeredit_v1_0* serviceData = (Epgsearch_exttimeredit_v1_0*) Data;
      serviceData->pTimerMenu = new cMenuMyEditTimer(serviceData->timer, serviceData->bNew, serviceData->event);

      return true;
   }
   if (strcmp(Id, "Epgsearch-updatesearchtimers-v1.0") == 0) {
      if (Data == NULL)
         return true;
      else
      {
         Epgsearch_updatesearchtimers_v1_0* serviceData = (Epgsearch_updatesearchtimers_v1_0*) Data;
         updateForced = serviceData->showMessage?2:1;
      }
      return true;
   }
   if (strcmp(Id, "Epgsearch-osdmessage-v1.0") == 0) {
      if (Data == NULL)
         return true;
      else
      {
         Epgsearch_osdmessage_v1_0* serviceData = (Epgsearch_osdmessage_v1_0*) Data;
         Skins.Message(serviceData->type, serviceData->message,5);
      }
      return true;
   }
   if (strcmp(Id, "Epgsearch-searchmenu-v1.0") == 0) {
      if (Data == NULL)
         return true;
      EpgSearchMenu_v1_0* serviceData = (EpgSearchMenu_v1_0*) Data;
      serviceData->Menu = new cMenuEPGSearchExt();

      return true;
   }
  if (strcmp(Id, "Epgsearch-searchmenu-v1.1") == 0) {
     if (Data == NULL)
        return true;
      Epgsearch_searchmenu_v1_1* serviceData = (Epgsearch_searchmenu_v1_1*) Data;
      serviceData->pSearchMenu = new cMenuEPGSearchExt();
      if(serviceData->text)
            ((cMenuEPGSearchExt *)serviceData->pSearchMenu)->ShowNew(serviceData->text);
      return true;
  }
   if (strcmp(Id, "Epgsearch-conflictmenu-v1.0") == 0) {
      if (Data == NULL)
         return true;
      
      EpgSearchMenu_v1_0* serviceData = (EpgSearchMenu_v1_0*) Data;
      serviceData->Menu = new cMenuConflictCheck();

      return true;
   }
   if (strcmp(Id, "Epgsearch-lastconflictinfo-v1.0") == 0) {
      if (Data == NULL)
         return true;
      
      Epgsearch_lastconflictinfo_v1_0* serviceData = (Epgsearch_lastconflictinfo_v1_0*) Data;
      serviceData->nextConflict = cConflictCheckThread::m_cacheNextConflict;
      serviceData->relevantConflicts = cConflictCheckThread::m_cacheRelevantConflicts;
      serviceData->totalConflicts = cConflictCheckThread::m_cacheTotalConflicts;

      return true;
   }
   if (strcmp(Id, "Epgsearch-searchresults-v1.0") == 0) {
      if (Data == NULL)
         return true;
      cSearchExt* SearchExt = new cSearchExt;
      
      Epgsearch_searchresults_v1_0* searchData = (Epgsearch_searchresults_v1_0*) Data;
      searchData->pResultList = NULL;
      strcpy(SearchExt->search,searchData->query);
      if (searchData->channelNr > 0)
      {
         SearchExt->useChannel = true;
         SearchExt->channelMin = Channels.GetByNumber(searchData->channelNr);
         SearchExt->channelMax = Channels.GetByNumber(searchData->channelNr);
      }
      SearchExt->mode = searchData->mode;
      SearchExt->useTitle = searchData->useTitle;
      SearchExt->useSubtitle = searchData->useSubTitle;
      SearchExt->useDescription = searchData->useDescription;

      cSearchResults* results = SearchExt->Run();
      // transfer to result list
      if (results)
      {
         results->SortBy(CompareEventTime);
         searchData->pResultList = new cList<Epgsearch_searchresults_v1_0::cServiceSearchResult>;
         cSearchResult *result = results->First();
         while (result) 
         {
            searchData->pResultList->Add(new Epgsearch_searchresults_v1_0::cServiceSearchResult(result->event));
            result = results->Next(result);
         }
      }
      return true;
   }
   if (strcmp(Id, "Epgsearch-switchtimer-v1.0") == 0) {
      if (Data == NULL)
         return true;
      else
      {
         Epgsearch_switchtimer_v1_0* serviceData = (Epgsearch_switchtimer_v1_0*) Data;
         if (!serviceData->event)
            return false;
         switch(serviceData->mode){
            case 0: {// query existance
               cSwitchTimer *lTimer = SwitchTimers.InSwitchList(serviceData->event);
               if (lTimer) {
                  serviceData->switchMinsBefore = lTimer->switchMinsBefore;
                  serviceData->announceOnly     = lTimer->announceOnly;
               } // if
               serviceData->success=lTimer!=NULL;
               break;
            } // 0
            case 1: { // add/modify
               cSwitchTimer *lTimer = SwitchTimers.InSwitchList(serviceData->event);
               if (lTimer) {
                  lTimer->switchMinsBefore = serviceData->switchMinsBefore;
                  lTimer->announceOnly     = serviceData->announceOnly;
               } else {
                  cMutexLock SwitchTimersLock(&SwitchTimers);	
                  SwitchTimers.Add(new cSwitchTimer(serviceData->event,serviceData->switchMinsBefore,serviceData->announceOnly));  
                  SwitchTimers.Save(); 
                  cSwitchTimerThread::Init();
               } // if
               serviceData->success=true;
               break;
            } // 1
            case 2: {// delete
               cSwitchTimer *lTimer = SwitchTimers.InSwitchList(serviceData->event);
               serviceData->success=lTimer!=NULL;
               if (lTimer) { 
                  cMutexLock SwitchTimersLock(&SwitchTimers);	
                  SwitchTimers.Del(lTimer);
                  SwitchTimers.Save(); 
               } // if
               break;
            } // 2
            default: 
               serviceData->success=false;
               break;
         } // switch
      } // if
      return true;
   } // if
   if (strcmp(Id, "Epgsearch-quicksearch-v1.0") == 0) {
      if (Data == NULL)
         return true;
      
      EpgSearchMenu_v1_0* serviceData = (EpgSearchMenu_v1_0*) Data;
      serviceData->Menu = new cMenuQuickSearch(new cSearchExt);

      return true;
   }
   if (strcmp(Id, "Epgsearch-services-v1.0") == 0) {
      if (Data == NULL)
         return true;
      Epgsearch_services_v1_0* serviceData = (Epgsearch_services_v1_0*) Data;
      std::auto_ptr<cEpgsearchServiceHandler> autoHandler(new cEpgsearchServiceHandler);
      serviceData->handler = autoHandler;
      return true;
   }
   if (strcmp(Id, "Epgsearch-services-v1.1") == 0) {
      if (Data == NULL)
         return true;
      Epgsearch_services_v1_1* serviceData = (Epgsearch_services_v1_1*) Data;
      std::auto_ptr<cEpgsearchServiceHandler> autoHandler(new cEpgsearchServiceHandler);
      serviceData->handler = autoHandler;
      return true;
   }
   if (strcmp(Id, "Epgsearch-services-v1.2") == 0) {
      if (Data == NULL)
         return true;
      Epgsearch_services_v1_2* serviceData = (Epgsearch_services_v1_2*) Data;
      std::auto_ptr<cEpgsearchServiceHandler> autoHandler(new cEpgsearchServiceHandler);
      serviceData->handler = autoHandler;
      return true;
   }
   return false;
}

bool cPluginEpgsearch::Initialize(void)
{
   return true;
}

bool cPluginEpgsearch::Start(void)
{
   if (!ConfigDir)
      ConfigDir = strdup(cPlugin::ConfigDirectory("epgsearch"));

   cPlugconfdirVar::dir = cPlugin::ConfigDirectory();

   if (cLogFile::LogFileName)
      LogFile.Open(cLogFile::LogFileName, Version());
   else
      LogFile.Open(AddDirectory(CONFIGDIR, "epgsearch.log"), Version());

   gl_recStatusMonitor = new cRecStatusMonitor;
   gl_timerStatusMonitor = new cTimerStatusMonitor;

   SearchExtCats.Load(AddDirectory(CONFIGDIR, "epgsearchcats.conf"), true);

   LoadMenuTemplates();
   LoadUserVars();
   LoadConfD();

   ChannelGroups.Load(AddDirectory(CONFIGDIR, "epgsearchchangrps.conf"), true);
   Blacklists.Load(AddDirectory(CONFIGDIR, "epgsearchblacklists.conf"));
#ifdef USEMYSQL
   if(Setup.ReelboxModeTemp == eModeServer) // Server only; Clients uses Avahi
       SearchExts.LoadDB();
   else
       SearchExts.Load(AddDirectory(CONFIGDIR, "epgsearch.conf"));
#else
   SearchExts.Load(AddDirectory(CONFIGDIR, "epgsearch.conf"));
#endif
   SearchTemplates.Load(AddDirectory(CONFIGDIR, "epgsearchtemplates.conf"));
   RecsDone.Load(AddDirectory(CONFIGDIR, "epgsearchdone.data"));
   SwitchTimers.Load(AddDirectory(CONFIGDIR, "epgsearchswitchtimers.conf"));
   NoAnnounces.Load(AddDirectory(CONFIGDIR, "noannounce.conf"));
   DefTimerCheckModes.Load(AddDirectory(CONFIGDIR, "deftimerchkmodes.conf"));
   TimersDone.Load(AddDirectory(CONFIGDIR, "timersdone.conf"));

   cSearchTimerThread::Init(this);    
   cSwitchTimerThread::Init();    
   cConflictCheckThread::Init(this);    

   CheckUTF8();

   return true;
}

void cPluginEpgsearch::CheckUTF8()
{
#if APIVERSNUM >= 10503 
   // Taken from VDR's vdr.c
   char *CodeSet = NULL;
   if (setlocale(LC_CTYPE, ""))
     CodeSet = nl_langinfo(CODESET);
   else {
     char *LangEnv = getenv("LANG"); // last resort in case locale stuff isn't installed
     if (LangEnv) {
       CodeSet = strchr(LangEnv, '.');
       if (CodeSet)
	 CodeSet++; // skip the dot
     }
   }
   
   if (CodeSet && strcasestr(CodeSet, "UTF-8") != 0)
     isUTF8=true;
#endif
}

void cPluginEpgsearch::Stop(void)
{
   cSearchTimerThread::Exit();
   cSwitchTimerThread::Exit();
#ifdef REELVDR
   SearchExts.SetFilename(AddDirectory(CONFIGDIR, "epgsearch.conf"));
   SearchExts.Save();
#endif
}

void cPluginEpgsearch::Housekeeping(void)
{
   // Perform any cleanup or other regular tasks.
}

cOsdObject *cPluginEpgsearch::DoInitialSearch(char* rcFilename)
{
   cRCFile rcFile;
   if (rcFile.Load(rcFilename))
   {
      cSearchExt* SearchExt = new cSearchExt;
      strcpy(SearchExt->search,rcFile.Search);
      if (rcFile.ChannelNr != -1)
      {
         SearchExt->useChannel = true;
         SearchExt->channelMin = Channels.GetByNumber(rcFile.ChannelNr);
         SearchExt->channelMax = Channels.GetByNumber(rcFile.ChannelNr);
      }
      SearchExt->mode = rcFile.SearchMode;
      SearchExt->useTitle = rcFile.UseTitle;
      SearchExt->useSubtitle = rcFile.UseSubtitle;
      SearchExt->useDescription = rcFile.UseDescr;
      remove(rcFilename);
      return new cMenuSearchResultsForSearch(SearchExt, cTemplFile::GetTemplateByName("MenuSearchResults"));
   }
   else
      LogFile.eSysLog("could not load '%s'", rcFilename);
	
   return NULL;
}

cOsdObject *cPluginEpgsearch::MainMenuAction(void)
{
   if (reloadMenuConf)
      LoadMenuTemplates();

   if (showConflicts)
   {
      showConflicts = false;
      return new cMenuConflictCheck();
   }

   if (showAnnounces)
   {
      showAnnounces = false;
      return new cMenuAnnounceList(cSearchTimerThread::announceList);
   }

   // Perform the action when selected from the main VDR menu.
   cOsdObject* pMenu = NULL;
   char* rcFilename = strdup(AddDirectory(CONFIGDIR, ".epgsearchrc"));
   if (access(rcFilename, F_OK) == 0)
      pMenu = DoInitialSearch(rcFilename);
   else
      pMenu = new cMenuSearchMain();
    
   free(rcFilename);
   return pMenu;
}

void cPluginEpgsearch::LoadMenuTemplates()
{
   char* templateFilename = strdup(AddDirectory(CONFIGDIR, "epgsearchmenu.conf"));
   if (access(templateFilename, F_OK) == 0)
   {
      cTemplFile templFile;
      if (!templFile.Load(templateFilename))
      {
         templFile.Reset();
         LogFile.eSysLog("could not load '%s'", templateFilename);
      }
   }
   cTemplFile::PrepareDefaultTemplates();
   free(templateFilename);
}

void cPluginEpgsearch::LoadUserVars()
{
   char* userVarFilename = strdup(AddDirectory(CONFIGDIR, "epgsearchuservars.conf"));
   if (access(userVarFilename, F_OK) == 0)
   {
      LogFile.Log(1, "loading %s", userVarFilename);
      cUserVarFile userVarFile;
      if (!userVarFile.Load(userVarFilename))
         LogFile.eSysLog("could not load '%s'", userVarFilename);
      else
         LogFile.Log(2,"loaded uservars from %s", userVarFilename);
   }
   free(userVarFilename);
   UserVars.InitInternalVars();
   UserVars.InitExtEPGVars();
}

void cPluginEpgsearch::LoadConfD()
{
  cConfDLoader D;
  D.Load();
}

cMenuSetupPage *cPluginEpgsearch::SetupMenu(void)
{
   // Return a setup menu in case the plugin supports one.
   return new cMenuEPGSearchSetup;
}

bool cPluginEpgsearch::SetupParse(const char *Name, const char *Value)
{
   if	  (!strcasecmp(Name, "IsOrgSchedule")) return (EPGSearchConfig.ReplaceOrgSchedule == false);

   if      (!strcasecmp(Name, "HideMenu"))       EPGSearchConfig.hidemenu         = atoi(Value);
   if      (!strcasecmp(Name, "MainMenuEntry")) strcpy(EPGSearchConfig.mainmenuentry, Value);
   if      (!strcasecmp(Name, "ReplaceOrgSchedule"))  EPGSearchConfig.ReplaceOrgSchedule = atoi(Value);
   if      (!strcasecmp(Name, "StartMenu"))       EPGSearchConfig.StartMenu        = atoi(Value);
   //if      (!strcasecmp(Name, "RedKeyMode"))     EPGSearchConfig.redkeymode       = atoi(Value);
   //if      (!strcasecmp(Name, "BlueKeyMode"))     EPGSearchConfig.bluekeymode       = atoi(Value);
   if      (!strcasecmp(Name, "ShowProgress"))     EPGSearchConfig.showProgress   = atoi(Value);
   if      (!strcasecmp(Name, "ShowChannelNr"))     EPGSearchConfig.showChannelNr = atoi(Value);
   if      (!strcasecmp(Name, "UseOkForSwitch"))     EPGSearchConfig.useOkForSwitch = atoi(Value);
   if      (!strcasecmp(Name, "MaxChannelMenuNow"))     EPGSearchConfig.maxChannelMenuNow = atoi(Value);

   if      (!strcasecmp(Name, "ShowRadioChannels"))     EPGSearchConfig.showRadioChannels = atoi(Value);
   if      (!strcasecmp(Name, "OnePressTimerCreation")) EPGSearchConfig.onePressTimerCreation = atoi(Value);
   if      (!strcasecmp(Name, "ShowFavoritesMenu"))     EPGSearchConfig.showFavoritesMenu = atoi(Value);
   if      (!strcasecmp(Name, "FavoritesMenuTimespan")) EPGSearchConfig.FavoritesMenuTimespan = atoi(Value);
  
   if      (!strcasecmp(Name, "UserMode1Description"))   EPGSearchConfig.ShowModes[showUserMode1].SetDescription(Value);
   if      (!strcasecmp(Name, "UserMode1Time"))   EPGSearchConfig.ShowModes[showUserMode1].SetTime(atoi(Value));
   if      (!strcasecmp(Name, "UserMode1UseIt"))   EPGSearchConfig.ShowModes[showUserMode1].SetUsage(atoi(Value));

   if      (!strcasecmp(Name, "UserMode2Description"))   EPGSearchConfig.ShowModes[showUserMode2].SetDescription(Value);
   if      (!strcasecmp(Name, "UserMode2Time"))   EPGSearchConfig.ShowModes[showUserMode2].SetTime(atoi(Value));
   if      (!strcasecmp(Name, "UserMode2UseIt"))   EPGSearchConfig.ShowModes[showUserMode2].SetUsage(atoi(Value));

   if      (!strcasecmp(Name, "UserMode3Description"))   EPGSearchConfig.ShowModes[showUserMode3].SetDescription(Value);
   if      (!strcasecmp(Name, "UserMode3Time"))   EPGSearchConfig.ShowModes[showUserMode3].SetTime(atoi(Value));
   if      (!strcasecmp(Name, "UserMode3UseIt"))   EPGSearchConfig.ShowModes[showUserMode3].SetUsage(atoi(Value));

   if      (!strcasecmp(Name, "UserMode4Description"))   EPGSearchConfig.ShowModes[showUserMode4].SetDescription(Value);
   if      (!strcasecmp(Name, "UserMode4Time"))   EPGSearchConfig.ShowModes[showUserMode4].SetTime(atoi(Value));
   if      (!strcasecmp(Name, "UserMode4UseIt"))   EPGSearchConfig.ShowModes[showUserMode4].SetUsage(atoi(Value));

   if      (!strcasecmp(Name, "UseSearchTimers"))  EPGSearchConfig.useSearchTimers = atoi(Value);
   if      (!strcasecmp(Name, "UpdateIntervall"))  EPGSearchConfig.UpdateIntervall = atoi(Value);
   if      (!strcasecmp(Name, "SVDRPPort"))  EPGSearchConfig.SVDRPPort = atoi(Value);
   if      (!strcasecmp(Name, "CheckTimerConflicts"))  EPGSearchConfig.checkTimerConflictsAfterUpdate = atoi(Value);
   if      (!strcasecmp(Name, "CheckTimerConflictsPriority"))  EPGSearchConfig.checkMinPriority = atoi(Value);
   if      (!strcasecmp(Name, "CheckTimerConflictsDays"))  EPGSearchConfig.checkMaxDays = atoi(Value);
   if      (!strcasecmp(Name, "CheckConflictsIntervall"))  EPGSearchConfig.conflictCheckIntervall = atoi(Value);
   if      (!strcasecmp(Name, "CheckConflictsWithinLimit"))  EPGSearchConfig.conflictCheckWithinLimit = atoi(Value);
   if      (!strcasecmp(Name, "CheckConflictsIntervall2"))  EPGSearchConfig.conflictCheckIntervall2 = atoi(Value);
   if      (!strcasecmp(Name, "CheckConflictsMinDuration"))  EPGSearchConfig.checkMinDuration = atoi(Value);
   if      (!strcasecmp(Name, "CheckConflictsAfterTimerProg"))  EPGSearchConfig.checkTimerConflAfterTimerProg = atoi(Value);
   if      (!strcasecmp(Name, "CheckConflictsOnRecording"))  EPGSearchConfig.checkTimerConflOnRecording = atoi(Value);
   if      (!strcasecmp(Name, "NoConflMsgWhileReplay"))  EPGSearchConfig.noConflMsgWhileReplay = atoi(Value);
   if      (!strcasecmp(Name, "NoAnnounceWhileReplay"))  EPGSearchConfig.noAnnounceWhileReplay = atoi(Value);
#ifdef REELVDR
   if      (!strcasecmp(Name, "ShowPipOnAnnounce"))  EPGSearchConfig.showPipOnAnnounce = atoi(Value);
#endif
   if      (!strcasecmp(Name, "TimerProgRepeat"))  EPGSearchConfig.TimerProgRepeat = atoi(Value);

   if      (!strcasecmp(Name, "TimeIntervallFRFF"))  EPGSearchConfig.timeShiftValue = atoi(Value);
   if      (!strcasecmp(Name, "ToggleGreenYellow"))  EPGSearchConfig.toggleGreenYellow = atoi(Value);

   if      (!strcasecmp(Name, "DefPriority"))  EPGSearchConfig.DefPriority = atoi(Value);
   if      (!strcasecmp(Name, "DefLifetime"))  EPGSearchConfig.DefLifetime = atoi(Value);
   if      (!strcasecmp(Name, "DefMarginStart"))  EPGSearchConfig.DefMarginStart = atoi(Value);
   if      (!strcasecmp(Name, "DefMarginStop"))  EPGSearchConfig.DefMarginStop = atoi(Value);

   if      (!strcasecmp(Name, "IgnorePayTV"))  EPGSearchConfig.ignorePayTV = atoi(Value);

   if      (!strcasecmp(Name, "DefRecordingDir"))  strcpy(EPGSearchConfig.defrecdir, Value);
   if      (!strcasecmp(Name, "UseVDRTimerEditMenu"))  EPGSearchConfig.useVDRTimerEditMenu = atoi(Value);

   if      (!strcasecmp(Name, "ShowChannelGroups"))  EPGSearchConfig.showChannelGroups = atoi(Value); 
   if      (!strcasecmp(Name, "ShowDaySeparators"))  EPGSearchConfig.showDaySeparators = atoi(Value); 

   if      (!strcasecmp(Name, "ShowEmptyChannels"))  EPGSearchConfig.showEmptyChannels = atoi(Value); 

   if      (!strcasecmp(Name, "DefSearchTemplateID"))  EPGSearchConfig.DefSearchTemplateID = atoi(Value); 

   if      (!strcasecmp(Name, "AddSubtitleToTimerMode"))  EPGSearchConfig.addSubtitleToTimer = (addSubtitleToTimerMode) atoi(Value); 
  
   if      (!strcasecmp(Name, "MailViaScript"))  EPGSearchConfig.mailViaScript = atoi(Value); 
   if      (!strcasecmp(Name, "MailNotificationSearchtimers"))  EPGSearchConfig.sendMailOnSearchtimers = atoi(Value); 
   if      (!strcasecmp(Name, "MailNotificationConflicts"))  EPGSearchConfig.sendMailOnConflicts = atoi(Value); 
   if      (!strcasecmp(Name, "MailAddress")) strcpy(EPGSearchConfig.MailAddress, Value);
   if      (!strcasecmp(Name, "MailAddressTo")) strcpy(EPGSearchConfig.MailAddressTo, Value);

   if (isempty(EPGSearchConfig.MailAddressTo))
      strcpy(EPGSearchConfig.MailAddressTo, EPGSearchConfig.MailAddress);

   if      (!strcasecmp(Name, "MailServer")) strcpy(EPGSearchConfig.MailServer, Value);
   if      (!strcasecmp(Name, "MailUseAuth")) EPGSearchConfig.MailUseAuth = atoi(Value);
   if      (!strcasecmp(Name, "MailAuthUser")) strcpy(EPGSearchConfig.MailAuthUser, Value);
   if      (!strcasecmp(Name, "MailAuthPass")) strcpy(EPGSearchConfig.MailAuthPass, Value);

   if      (!strcasecmp(Name, "LastMailConflicts")) strcpy(EPGSearchConfig.LastMailConflicts, Value);

   return true;
}

VDRPLUGINCREATOR(cPluginEpgsearch); // Don't touch this!
