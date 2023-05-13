#ifndef APP_ICON_H
#define APP_ICON_H

#include <gtk/gtk.h>


typedef struct {
	char* filename;
	char* label;
	char* icon;
} AppIcon;


AppIcon appicon_load_from_file(char*);
GtkButton* appicon_create_icon(AppIcon);


#endif // APP_ICON_H
