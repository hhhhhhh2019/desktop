#include <app_icon.h>
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>


AppIcon appicon_load_from_file(char* filename) {
	/*GKeyFile* file = g_key_file_new();

	if (!g_key_file_load_from_file(file, filename, G_KEY_FILE_NONE, NULL)) {
		return (AppIcon){NULL,NULL,NULL};
	}

	char* name = g_key_file_get_string(file, "Desktop Entry", "Name", NULL);
	char* icon = g_key_file_get_string(file, "Desktop Entry", "Icon", NULL);

	return (AppIcon){filename, name, icon};*/

	GDesktopAppInfo* info = g_desktop_app_info_new(filename);

	if (info == 0 || g_desktop_app_info_get_is_hidden(info) == 1)
		return (AppIcon){NULL};

	char* name = (char*)g_app_info_get_display_name((GAppInfo*)info);
	char* icon = (char*)g_icon_to_string(g_app_info_get_icon((GAppInfo*)info));

	return (AppIcon){filename, name, icon};
}


GtkButton* appicon_create_icon(AppIcon icon) {
	GtkImage* image;

	if (*icon.icon == '/')
		image = GTK_IMAGE(gtk_image_new_from_file(icon.icon));
	else
		image = GTK_IMAGE(gtk_image_new_from_icon_name(icon.icon, 1));

	gtk_image_set_pixel_size(image, 96);
	gtk_widget_show(GTK_WIDGET(image));

	GtkLabel* label = GTK_LABEL(gtk_label_new(icon.label));
	gtk_label_set_lines(label, 2);
	gtk_label_set_line_wrap(label, 1);
	gtk_label_set_xalign(label, 0.5);
	gtk_label_set_justify(label, GTK_JUSTIFY_CENTER);
	gtk_widget_show(GTK_WIDGET(label));

	GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_box_pack_start(box, GTK_WIDGET(image), 0,0,0);
	gtk_box_pack_start(box, GTK_WIDGET(label), 0,0,0);
	gtk_widget_show(GTK_WIDGET(box));

	GtkButton* button = GTK_BUTTON(gtk_button_new());
	gtk_button_set_image(button, GTK_WIDGET(box));
	gtk_button_set_image_position(button, GTK_POS_TOP);
	gtk_widget_show(GTK_WIDGET(button));

	GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(button));
	gtk_style_context_add_class(context, "desktop_icon");

	return button;
}
