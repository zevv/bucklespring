/*
 * An icon in the system tray, with a menu for the things worth reaching for
 * without a terminal: pause, volume, which audio device, and quit.
 *
 * The icon is a StatusNotifierItem, put up through libayatana-appindicator.
 * That is what KDE shows natively and what the AppIndicator extension shows
 * under GNOME Shell; where nothing is listening the item simply never
 * appears, which is why tray_init() settling for a display is as far as the
 * checking can usefully go.
 *
 * Everything here runs on the main thread, which owns the GTK loop, and
 * reaches the rest of the program only through the buckle_* calls in
 * buckle.h, each of which takes the audio lock for itself.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <gtk/gtk.h>
#include <libayatana-appindicator/app-indicator.h>

#include "buckle.h"

#define VOLUME_STEP 10
#define DEFAULT_MUTE_KEYCODE 0x46 /* Scroll Lock */

static GtkWidget *item_mute;
static GtkWidget *item_volume;

/*
 * The volume levels, as radio items in a submenu.  A real slider is not on
 * offer: a StatusNotifierItem menu travels over dbusmenu, which carries item
 * properties and not widgets, so whatever is built here has to be made of
 * labels and check marks.  Nor can a client keep the menu open across a
 * click; that is the panel's business.  So the levels are one click each
 * rather than a click per step, with the two ends to push for finer moves,
 * and a scroll over the icon for when the menu is in the way entirely.
 */

#define VOLUME_LEVELS 11

static GtkWidget *level_item[VOLUME_LEVELS];
static int syncing = 0;


/*
 * Two presses of the mute key inside two seconds toggle the sound, so the
 * same sequence does both halves of pause and resume.  Name the key only
 * when it is the one the label can be sure of.
 */

static const char *mute_sequence(void)
{
	return buckle_mute_keycode() == DEFAULT_MUTE_KEYCODE
		? "ScrollLock \303\227 2"
		: "mute key \303\227 2";
}


static void sync_menu(void)
{
	char label[128];

	g_snprintf(label, sizeof label, "%s  (%s)",
			buckle_muted() ? "Resume" : "Pause", mute_sequence());
	gtk_menu_item_set_label(GTK_MENU_ITEM(item_mute), label);

	g_snprintf(label, sizeof label, "Volume: %d%%", buckle_gain());
	gtk_menu_item_set_label(GTK_MENU_ITEM(item_volume), label);

	/* Tick the level nearest the gain, which the keyboard and the scroll
	 * wheel can have moved off a step.  Setting it emits "toggled", hence
	 * the guard. */

	syncing = 1;
	{
		int nearest = (buckle_gain() + VOLUME_STEP / 2) / VOLUME_STEP;

		if(nearest < 0) { nearest = 0; }
		if(nearest > VOLUME_LEVELS - 1) { nearest = VOLUME_LEVELS - 1; }
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(level_item[nearest]), TRUE);
	}
	syncing = 0;
}


/* The mute key works whether or not anyone is looking at the menu, so the
 * label is worth re-reading rather than only writing when it is clicked. */

static gboolean on_tick(gpointer data)
{
	(void)data;
	sync_menu();
	return G_SOURCE_CONTINUE;
}


static void on_mute(GtkMenuItem *item, gpointer data)
{
	(void)item;
	(void)data;
	buckle_set_muted(!buckle_muted());
	sync_menu();
}


static void on_volume(GtkMenuItem *item, gpointer data)
{
	(void)item;
	buckle_set_gain(buckle_gain() + GPOINTER_TO_INT(data));
	sync_menu();
}


static void on_level(GtkCheckMenuItem *item, gpointer data)
{
	if(syncing || !gtk_check_menu_item_get_active(item)) {
		return;
	}
	buckle_set_gain(GPOINTER_TO_INT(data));
	sync_menu();
}


/* A wheel over the icon, which is the nearest thing to a slider that a tray
 * icon has, and does not involve opening the menu at all. */

static void on_scroll(AppIndicator *indicator, gint delta,
		GdkScrollDirection direction, gpointer data)
{
	int step;

	(void)indicator;
	(void)data;

	if(delta < 1) {
		delta = 1;
	}

	switch(direction) {
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			step = VOLUME_STEP;
			break;
		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT:
			step = -VOLUME_STEP;
			break;
		default:
			return;
	}

	buckle_set_gain(buckle_gain() + step * delta);
	sync_menu();
}


static void on_device(GtkCheckMenuItem *item, gpointer data)
{
	const char *name = data;

	/* Picking the one already in use, or the echo of our own call to
	 * gtk_check_menu_item_set_active() below, is not worth a device
	 * change; and only the item being switched on means anything. */

	if(!gtk_check_menu_item_get_active(item)) {
		return;
	}
	if(g_strcmp0(name, buckle_audio_device()) == 0) {
		return;
	}

	if(buckle_set_audio_device(name) < 0) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, 0,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
				"Could not switch to the audio device \"%s\".", name);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}


static void on_about(GtkMenuItem *item, gpointer data)
{
	(void)item;
	(void)data;

	char comments[512];

	g_snprintf(comments, sizeof comments,
		"The sound of an IBM Model M buckling spring keyboard, played "
		"back for every key you press.\n\n"
		"Two presses of %s pause and resume it; the same is on this menu. "
		"The mouse buttons and the scroll wheel click too, and a key held "
		"down clicks for every repeat.\n\n"
		"Run with --help for the rest.",
		buckle_mute_keycode() == DEFAULT_MUTE_KEYCODE
			? "Scroll Lock" : "the mute key");

	gtk_show_about_dialog(NULL,
		"program-name", "Bucklespring",
		"version", PACKAGE_VERSION,
		"comments", comments,
		"copyright", "Copyright \302\251 2016-2025 Ico Doornekamp",
		"license-type", GTK_LICENSE_GPL_2_0,
		"website", PACKAGE_URL,
		"logo-icon-name", "input-keyboard",
		NULL);
}


static void on_quit(GtkMenuItem *item, gpointer data)
{
	(void)item;
	(void)data;
	buckle_quit();
}


static GtkWidget *menu_item(GtkWidget *menu, const char *label, GCallback cb,
		gpointer data)
{
	GtkWidget *item = gtk_menu_item_new_with_label(label);

	if(cb != NULL) {
		g_signal_connect(item, "activate", cb, data);
	}
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	return item;
}


static void menu_separator(GtkWidget *menu)
{
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
}


/* The device names are handed to the menu items and outlive this call, so
 * the list is kept rather than freed. */

static void append_devices(GtkWidget *menu)
{
	GtkWidget *item = gtk_menu_item_new_with_label("Audio device");
	GtkWidget *sub = gtk_menu_new();
	GSList *group = NULL;
	char **list = buckle_audio_devices();
	size_t i;

	for(i = 0; list != NULL && list[i] != NULL; i++) {
		GtkWidget *radio =
			gtk_radio_menu_item_new_with_label(group, list[i]);

		group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(radio));

		if(g_strcmp0(list[i], buckle_audio_device()) == 0) {
			gtk_check_menu_item_set_active(
					GTK_CHECK_MENU_ITEM(radio), TRUE);
		}

		g_signal_connect(radio, "toggled", G_CALLBACK(on_device), list[i]);
		gtk_menu_shell_append(GTK_MENU_SHELL(sub), radio);
	}

	if(i == 0) {
		GtkWidget *none = gtk_menu_item_new_with_label("(none found)");
		gtk_widget_set_sensitive(none, FALSE);
		gtk_menu_shell_append(GTK_MENU_SHELL(sub), none);
	}

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
}


/* Louder and quieter at the two ends, with every tenth in between. */

static GtkWidget *volume_menu(void)
{
	GtkWidget *sub = gtk_menu_new();
	GSList *group = NULL;
	int i;

	menu_item(sub, "+   Louder", G_CALLBACK(on_volume),
			GINT_TO_POINTER(VOLUME_STEP));
	menu_separator(sub);

	for(i = VOLUME_LEVELS - 1; i >= 0; i--) {
		char label[32];
		GtkWidget *radio;

		g_snprintf(label, sizeof label, "%d%%", i * VOLUME_STEP);
		radio = gtk_radio_menu_item_new_with_label(group, label);
		group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(radio));

		g_signal_connect(radio, "toggled", G_CALLBACK(on_level),
				GINT_TO_POINTER(i * VOLUME_STEP));
		gtk_menu_shell_append(GTK_MENU_SHELL(sub), radio);

		level_item[i] = radio;
	}

	menu_separator(sub);
	menu_item(sub, "-   Quieter", G_CALLBACK(on_volume),
			GINT_TO_POINTER(-VOLUME_STEP));

	return sub;
}


int tray_init(void)
{
	AppIndicator *indicator;
	GtkWidget *menu;

	/* Not an error: a console session has no display to put an icon on,
	 * and the program is perfectly usable without one. */

	if(!gtk_init_check(NULL, NULL)) {
		return 0;
	}

	menu = gtk_menu_new();

	item_mute = menu_item(menu, "Pause", G_CALLBACK(on_mute), NULL);

	menu_separator(menu);

	item_volume = gtk_menu_item_new_with_label("Volume");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_volume);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_volume), volume_menu());

	menu_separator(menu);

	append_devices(menu);

	menu_separator(menu);

	menu_item(menu, "About Bucklespring", G_CALLBACK(on_about), NULL);
	menu_item(menu, "Quit", G_CALLBACK(on_quit), NULL);

	gtk_widget_show_all(menu);

	/* app_indicator_new() is marked deprecated with nothing named in its
	 * place: it is the GTK3 binding as a whole that is giving way to a
	 * GLib-only one, and this is still the constructor it offers. */

	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	indicator = app_indicator_new(PACKAGE_NAME, "input-keyboard",
			APP_INDICATOR_CATEGORY_HARDWARE);
	G_GNUC_END_IGNORE_DEPRECATIONS
	app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_title(indicator, "Bucklespring");
	app_indicator_set_menu(indicator, GTK_MENU(menu));
	g_signal_connect(indicator, APP_INDICATOR_SIGNAL_SCROLL_EVENT,
			G_CALLBACK(on_scroll), NULL);

	sync_menu();
	g_timeout_add(500, on_tick, NULL);

	return 1;
}


void tray_run(void)
{
	gtk_main();
}
