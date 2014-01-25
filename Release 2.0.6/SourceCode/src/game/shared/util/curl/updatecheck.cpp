#include "cbase.h"
#ifdef _LINUX
	#include <arpa/inet.h>
#endif
#undef GetCommandLine
#undef ReadConsoleInput
#undef RegCreateKey
#undef RegCreateKeyEx
#undef RegOpenKey
#undef RegOpenKeyEx
#undef RegQueryValue
#undef RegQueryValueEx
#undef RegSetValue
#undef RegSetValueEx
#undef recvfrom

#ifdef CLIENT_DLL
#include "modcom/menupopup.h"
#endif

#include "filesystem.h"
#include "curl.h"

#define REMOTE_URL "http://datacollecter.modularcombatsource.com/latestversion.php"
#define VERSION_FILE "version.txt"

#ifndef CLIENT_DLL
	#define VarArgs UTIL_VarArgs
#endif

struct MemoryStruct
{
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */ 
		Warning("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), ptr, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

unsigned UpdateCheckThread( void *params )
{
	Msg("Checking for updates...\n");

	struct MemoryStruct chunk;
	chunk.memory = (char*)malloc(1);
	chunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL); // In windows, this will init the winsock stuff
	CURL *curl = curl_easy_init(); // get curl handle
	if ( curl )
	{
		curl_easy_setopt(curl, CURLOPT_URL, REMOTE_URL);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // send all data to this function
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk); // we pass our 'chunk' struct to the callback function
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0"); // some servers don't like requests missing a user-agent

		CURLcode res = curl_easy_perform(curl); // run commands
		if ( res != 0 )
			Msg(VarArgs("Update check failed! (error #%i)\n", (int)res));
		else if(chunk.memory)
		{
			int fileLength;
			char *currentVersion = (char*)UTIL_LoadFileForMe( VERSION_FILE, &fileLength );
#ifdef DEBUG
			Msg("Current version is %s\n", currentVersion);
#endif
			if ( chunk.size > 30 )
#ifdef DEBUG
				Warning("Update check failed! Unexpected response from %s:\n%s\n", REMOTE_URL, chunk.memory);
#else
				Msg("Update check failed! (unexpected response)\n");
#endif
			else if ( FStrEq(chunk.memory, currentVersion) )
				Msg("Update check complete, no updates available\n");
			else
			{
#ifdef CLIENT_DLL
				CMenuPopupPanel* pPanel = (CMenuPopupPanel*)(GetMenuPopupPanel()->GetPanel());
				pPanel->ShowMissingContentLabels(false);
				pPanel->SetMessage("Update Available", VarArgs("Modular Combat %s is now available!", chunk.memory));
#endif
				Warning(VarArgs("Update available: version %s is now available!\n", chunk.memory));
			}
			free(chunk.memory);
		}
		else
			Msg("Update check failed! (no data returned)\n");

		curl_easy_cleanup(curl);  // standard cleanup
	}
	curl_global_cleanup();
	return 0;
}

void CheckForUpdates()
{
	CreateSimpleThread( UpdateCheckThread, NULL );
}
