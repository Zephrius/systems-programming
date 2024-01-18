// Jackson Burns
// 10-30-2023
// Lab 6 "Plugin Lab"
// Description: This program is able to load in shared object files as plugins. It continually prompts the
// user for a command and if the user enters load and a file path for a shared object file it will load
// the plugin, if the user enters unload and the name of the plugin then it will unload that plugin, if 
// the user enters list or plugins then it will list the currently loaded plugins. If the user enters
// quit then the program will stop prompting the user for commands and unload all the currently loaded
// plugins and free the vectors used to store the plugins and their respective handles. If the user enters
// any command that isn't one of the above then it will go through all loaded plugins and try to run the 
// command with them. If the command belongs to one of the plugins then it will run that command and
// stop trying to run the command on the rest of the plugins.

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "vector.h"
#include <string.h>

typedef struct Plugin
{
	char name[65];
	int (*init)(void);
	void (*fini)(void);
	int (*cmd)(char *str);
} Plugin;


int main ()
{
	// Input is initialized to be the combined max size of cmd and path
	char input[4354];
	char cmd[257];
	char path[4097];
	Vector *plugins = vector_new();
	Vector *handles = vector_new();

	while (1)
	{
		// Prompt user for a command
		printf(">");
		fgets(input, sizeof(input), stdin);

		// Quit
		if (strncmp(input, "quit", 4) == 0) 
		{
			break;
		}
		
		// Load
		else if (strncmp(input, "load", 4) == 0)
		{
			// Separate the path from the input
			sscanf(input, "%s %s", cmd, path);
			
			// Open SO
			void *handle = dlopen(path, RTLD_LAZY);
			if (handle == NULL)
			{
				perror("Unable to open given file path.\n");
			}
			
			// Store plugin object from the export struct in given SO
			Plugin *plug = dlsym(handle, "export");

			// Run the plugins init command
			int (*initialize)(void) = plug->init;
			if (initialize == NULL) perror("Unable to run plugin's init command.\n");
			initialize();

			// Add the plugin and the plugin's handle to their respective vectors
			vector_push(plugins, (int64_t)plug);
			vector_push(handles, (int64_t)handle);
		} 

		// Unload
		else if (strncmp(input, "unload", 6) == 0)
		{
			// Separate plugin name from input
			char pname[65];
			sscanf(input, "%s %s", cmd, pname);
			
			// Go through plugin vector to find the given pluging
			for (int i = 0; i < (int)vector_size(plugins); i++)
			{
				// Typecast bc I don't like warnings
				Plugin *pval;
				int64_t val;
				vector_get(plugins, i, &val);
				pval = (Plugin*)val;

				// If current plugin's name matches given name...
				if (!strcmp(pval->name, pname))
				{
					// Call the fini function for that plugin
					void (*done)(void) = pval->fini;
					if (done == NULL) perror("Unable to run plugin's fini command.\n");
					done();

					// We need the matching handle for plugin
					void *hand;
					int64_t hval;
					vector_get(handles, i, &hval);
					hand = (void*)hval;

					// Close SO and remove the plugin and handle from their vectors
					dlclose(hand);
					vector_remove(plugins, i);
					vector_remove(handles, i);
				}
			}
		} 

		// List/Plugins
		else if (strncmp(input, "list", 4) == 0 || strncmp(input, "plugins", 7) == 0)
		{
			int num_plugins = (int)vector_size(plugins);

			// Go through plugin vector and list each name
			for (int i = 0; i < num_plugins; i++)
			{
				// Typecast so no warnings
				int64_t val;
				vector_get(plugins, i, &val);
				Plugin *pval = (Plugin*)val;
				printf("%s\n", pval->name);
			}

			printf("%d plugins loaded.\n", num_plugins);
		}

		// Generic Commands
		else
		{
			sscanf(input, "%s", cmd);
			//printf("Your command is: %s\n", cmd);

			// Go through each plugin to try the given command
			for (int i = 0; i < (int)vector_size(plugins); i++)
			{
				int64_t val;
				vector_get(plugins, i, &val);
				Plugin *plug = (Plugin*)val;

				// If the command is run by current plugin then stop trying
				int (*command)(char *) = plug->cmd;
				if(command(cmd))
				{
					break;
				}
			}
		}
	}

	// If we are here, that means the user has chosen to quit the program.
	// If there are any still loaded plugins we need to unload them here
	for (int i = 0; i < (int)vector_size(plugins); i++)
	{
		Plugin *plug;
		int64_t val;
		vector_get(plugins, i, &val);
		plug = (Plugin*)val;

		void (*done)(void) = plug->fini;
		done();

		void *handle;
		int64_t val2;
		vector_get(handles, i, &val2);
		handle = (void*)val2;
		dlclose(handle);
	}

	// Free the vectors
	vector_free(plugins);
	vector_free(handles);

}

