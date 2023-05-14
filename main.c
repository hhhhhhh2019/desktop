#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <xcb/xcb.h>
#include <app_icon.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <stdlib.h>


#define ROOT_CSS "/com/Cher/Desktop/css/root_window.css"


GError* error = NULL;
GtkBuilder* builder;

GtkCssProvider* css_provider;

GtkWidget* root_window;
GtkWidget* grid;

GtkWidget* panel_window;
char panel_is_button_press = 0;
int panel_last_x, panel_last_y;
char panel_open = 0;
double panel_swipe_length = 0; // without sqrt
char panel_swipe_dir = 0; // 0 - left, 1 - up, 2 - right, 3 - down


GResource* widgets_resource;


int rows = 0;
int cols = 0;
int max_cols = 5;

char panel_expose(GtkWidget*, cairo_t*, gpointer);

void set_window_strut(GtkWidget*);

void update_icons();
void icon_press(GtkWidget*, GdkEvent*, gpointer);

void panel_motion_notify (GtkWidget*, GdkEventMotion, gpointer);
void panel_button_press  (GtkWidget*, GdkEventButton, gpointer);
void panel_button_release(GtkWidget*, GdkEventButton, gpointer);
void on_swipe(GtkGestureSwipe*, gdouble, gdouble, gpointer);


int main(int argc, char** argv) {
	gtk_init(&argc, &argv);

	widgets_resource = g_resource_load("res/widgets.gresource", &error);

	if (!widgets_resource) {
		g_warning("%s", error->message);
		g_free(error);
		return 1;
	}

	g_resources_register(widgets_resource);


	css_provider = gtk_css_provider_new();
	gtk_style_context_add_provider_for_screen(gdk_display_get_default_screen(gdk_display_get_default()), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	gtk_css_provider_load_from_resource(GTK_CSS_PROVIDER(css_provider), ROOT_CSS);

	builder = gtk_builder_new();

	if (!gtk_builder_add_from_resource(builder, "/com/Cher/Desktop/ui/root_window.ui", &error)) {
		g_warning("%s", error->message);
		g_free(error);
		return 1;
	}

	root_window  = GTK_WIDGET(gtk_builder_get_object(builder, "root_window"));
	grid         = GTK_WIDGET(gtk_builder_get_object(builder, "grid"));

	panel_window = GTK_WIDGET(gtk_builder_get_object(builder, "panel_window"));
	gtk_window_move(GTK_WINDOW(panel_window), 0,-1240);

	GdkScreen *screen = gtk_widget_get_screen(panel_window);
	GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
	gtk_widget_set_visual(panel_window, visual);

	gtk_window_set_decorated(GTK_WINDOW(panel_window), 0);
	g_signal_connect(panel_window, "draw", G_CALLBACK(panel_expose), NULL);

	gtk_builder_connect_signals(builder, NULL);

	//update_icons();

	gtk_widget_show_all(root_window);
	gtk_widget_show_all(panel_window);
	set_window_strut(root_window);
	set_window_strut(panel_window);
	gtk_main();
	return 0;
}



void set_window_strut(GtkWidget* window) {
	GdkWindow* dwin = gtk_widget_get_window(gtk_widget_get_toplevel(window));

	GdkAtom atom_cardinal      = gdk_atom_intern("CARDINAL", 0);
	GdkAtom atom_strut         = gdk_atom_intern("_NET_WM_STRUT", 0);
	GdkAtom atom_strut_partial = gdk_atom_intern("_NET_WM_STRUT_PARTIAL", 0);

	long vals[12] = {0,0,40,0, 0,0, 0,0, 0,40, 0,0};
	gdk_property_change(dwin, atom_strut, atom_cardinal, 32, GDK_PROP_MODE_REPLACE, (guchar*)vals, 4);
	gdk_property_change(dwin, atom_strut_partial, atom_cardinal, 32, GDK_PROP_MODE_REPLACE, (guchar*)vals, 12);
}



void update_icons() {
	GList* children = gtk_container_get_children(GTK_CONTAINER(grid));

	for (int i = 0; i < g_list_length(children); i++)
		gtk_widget_destroy(g_list_nth_data(children, i));

	for (int i = 0; i < max_cols-cols; i++)
		gtk_grid_insert_column(GTK_GRID(grid), i);

	cols = max_cols;

	DIR* dirp;
	struct dirent* entry;

	int id = 0;

	dirp = opendir("/usr/share/applications");
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) {
			AppIcon icon = appicon_load_from_file(entry->d_name);

			if (icon.filename == NULL)
				continue;

			char* filename = malloc(strlen(icon.filename)+1);
			strcpy(filename, icon.filename);

			int x = id % cols;
			int y = id / cols;

			if (x == 0) {
				gtk_grid_insert_row(GTK_GRID(grid), y);
				rows++;
			}

			GtkButton* button = appicon_create_icon(icon);
			g_signal_connect_data(G_OBJECT(button), "button-press-event", G_CALLBACK(icon_press), filename, (GClosureNotify)free, 0);

			gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(button), x,y, 1,1);

			id++;
		}
	}
	closedir(dirp);
}



void icon_press(GtkWidget* self, GdkEvent* event, gpointer data) {
	char* exec = malloc(strlen("gtk-launch ") + strlen((char*)data));
	strcpy(exec, "gtk-launch ");
	strcat(exec, data);
	system(exec);
	free(exec);
}



char panel_expose(GtkWidget* self, cairo_t* cr, gpointer) {
	cairo_save(cr);

	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.9);

	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);

	cairo_restore(cr);

	return 0;
}


void panel_motion_notify(GtkWidget* self, GdkEventMotion event, gpointer) {
	if (panel_is_button_press == 0)
		return;

	int vx = event.x_root - panel_last_x;
	int vy = event.y_root - panel_last_y;
	panel_last_x = event.x_root;
	panel_last_y = event.y_root;

	int x,y;
	gtk_window_get_position(GTK_WINDOW(panel_window), &x,&y);

	if (vy > 0 && y == 0) {}
	else if (y > 0)
		gtk_window_move(GTK_WINDOW(panel_window), 0,0);
	else
		gtk_window_move(GTK_WINDOW(panel_window), 0,y+vy);

	panel_swipe_length = (double)(vx*vx) + (double)(vy*vy);

	if (abs(vx) > abs(vy)) {
		if (vx > 0)
			panel_swipe_dir = 2;
		else
			panel_swipe_dir = 0;
	} else {
		if (vy > 0)
			panel_swipe_dir = 3;
		else
			panel_swipe_dir = 1;
	}
}

void panel_button_press(GtkWidget*, GdkEventButton event, gpointer) {
	panel_is_button_press = 1;

	panel_last_x = event.x_root;
	panel_last_y = event.y_root;

	panel_swipe_length = 0;
}

void panel_button_release(GtkWidget*, GdkEventButton, gpointer) {
	panel_is_button_press = 0;

	int x,y;
	gtk_window_get_position(GTK_WINDOW(panel_window), &x,&y);

	if (panel_open == 0) {
		if ((panel_swipe_length > 400 || y > -800) && panel_swipe_dir == 3) {
			gtk_window_move(GTK_WINDOW(panel_window), 0,0);
			panel_open = 1;
		} else {
			gtk_window_move(GTK_WINDOW(panel_window), 0,-1240);
		}
	} else {
		if ((panel_swipe_length > 400  || y < -400) && panel_swipe_dir == 1) {
			gtk_window_move(GTK_WINDOW(panel_window), 0,-1240);
			panel_open = 0;
		} else {
			gtk_window_move(GTK_WINDOW(panel_window), 0,0);
		}
	}
}
