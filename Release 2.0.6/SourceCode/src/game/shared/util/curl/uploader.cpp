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

#include "filesystem.h"
#include "curl.h"


struct ThreadParams_t
{
	char fileName[FILENAME_MAX];
};

#ifdef WIN32
/* NOTE: to work on Windows with libcurl as a DLL, you MUST also provide
   a read callback with CURLOPT_READFUNCTION. 
   Failing to do so will give you a crash since a DLL may not use the
   variable's memory when passed in to it from an app like this. */ 
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */ 
  return fread(ptr, size, nmemb, (FILE*)stream);
}
#endif

unsigned UploadThread( void *params )
{
	ThreadParams_t *p = (ThreadParams_t*)params;
	
#define HTTP_SERVER "datacollecter.modularcombatsource.com/205"

#ifdef RELEASE

#define HTTP_USERNAME "collection205"
#define HTTP_PASSWORD "collection205"

#else

#define HTTP_USERNAME "collection205dev"
#define HTTP_PASSWORD "collection205dev"

#endif

	Msg("Uploading gameplay data...\n");

	char localpath[512] = { 0 }, remotepath[512] = { 0 };
	filesystem->RelativePathToFullPath(p->fileName, "MOD", localpath, sizeof(localpath));
	Q_snprintf(remotepath, sizeof(remotepath), "http://%s/%s", HTTP_SERVER, p->fileName);

	FILE *fh = fopen(localpath, "rb");
	//struct curl_slist *headerlist=NULL;

	curl_global_init(CURL_GLOBAL_ALL); // In windows, this will init the winsock stuff
	CURL *curl = curl_easy_init(); // get curl handle
	if ( curl )
	{
		//headerlist = curl_slist_append(headerlist, UTIL_VarArgs("RNFR %s", remotefilename));
		//headerlist = curl_slist_append(headerlist, UTIL_VarArgs("RNTO %s", remotefilename2)); // rename file once up there

#ifdef WIN32
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
#endif
		curl_easy_setopt(curl, CURLOPT_URL, remotepath); // specify target
		curl_easy_setopt(curl, CURLOPT_STDERR, NULL);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L); // enable uploading
		curl_easy_setopt(curl, CURLOPT_USERPWD, UTIL_VarArgs("%s:%s", HTTP_USERNAME, HTTP_PASSWORD));
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_READDATA, fh); // specify which file to upload
		CURLcode res = curl_easy_perform(curl); // run commands
		if ( res != 0 )
			Msg(UTIL_VarArgs("Data upload failed! (error #%i)\n", (int)res));

		//curl_slist_free_all (headerlist); // cleanup FTP command list
		curl_easy_cleanup(curl);  // standard cleanup
	}
	fclose(fh); // close the local file
	//g_pFullFileSystem->Close( fh );
	curl_global_cleanup();
	Msg("Data upload complete\n");

	// delete the file that we just uploaded (or failed to upload)
	filesystem->RemoveFile(p->fileName, "MOD");
	delete p; // and then delete the params pointer also
	return 0;
}


void DoFTPUpload(const char *fileName)
{
	ThreadParams_t *p = new ThreadParams_t;
	Q_snprintf(p->fileName, sizeof(p->fileName), fileName);
	CreateSimpleThread( UploadThread, p );
}
