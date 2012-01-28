/*
 * panel-compatibility.c: panel backwards compatibility support
 *
 * Copyright (C) 2003 Sun Microsystems, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 *	Mark McLoughlin <mark@skynet.ie>
 */

#include <config.h>

#include "panel-compatibility.h"

#include "panel-profile.h"
#include "mate-panel-applets-manager.h"

void
panel_compatiblity_migrate_settings_menu_button (MateConfClient *client,
						 const char  *id)
{
	const char *key;

	panel_profile_remove_from_list (PANEL_MATECONF_OBJECTS, id);

	key = panel_mateconf_full_key (PANEL_MATECONF_OBJECTS, id,
				    "launcher_location");
	mateconf_client_set_string (client, key,
				 "mate-control-center.desktop", NULL);

	key = panel_mateconf_full_key (PANEL_MATECONF_OBJECTS, id,
				    "object_type");
	mateconf_client_set_string (client, key, "launcher-object", NULL);

	panel_profile_add_to_list (PANEL_MATECONF_OBJECTS, id);
}

gchar *
panel_compatibility_get_applet_iid (const gchar *id)
{
	MateConfClient *client = panel_mateconf_get_client ();
	MatePanelAppletInfo *info;
	const char *key;
	gchar *applet_iid;
	gboolean needs_migration;
	const char *iid;

	/*
	 * There are two compatibility steps here:
	 *
	 * 1) we need to migrate from bonobo_iid to applet_iid if there's no
	 *    value in the applet_iid key. Always.
	 *
	 * 2) we need to try to migrate the iid to a new iid. We can't assume
	 *    that the fact that the applet_iid key was used mean anything
	 *    since the value there could well be a bonobo iid.
	 *    The reason we really have to try to migrate first is this case:
	 *    if an applet was added with the bonobo iid but gets ported later
	 *    to dbus, then the reference to the bonobo iid will only be valid
	 *    as an old reference.
	 *    And if migration fails, we just use the iid as it is.
	 */

	needs_migration = FALSE;

	key = panel_mateconf_full_key (PANEL_MATECONF_APPLETS, id, "applet_iid");
	applet_iid = mateconf_client_get_string (client, key, NULL);

	if (!applet_iid || !applet_iid[0]) {
		needs_migration = TRUE;

		key = panel_mateconf_full_key (PANEL_MATECONF_APPLETS, id, "bonobo_iid");
		applet_iid = mateconf_client_get_string (client, key, NULL);

		if (!applet_iid || !applet_iid[0])
			return NULL;
	}

	info = mate_panel_applets_manager_get_applet_info_from_old_id (applet_iid);
	if (!info)
		info = mate_panel_applets_manager_get_applet_info (applet_iid);

	if (!info)
		return NULL;

	iid = mate_panel_applet_info_get_iid (info);

	/* migrate if the iid in the configuration is different than the real
	 * iid that will get used */
	if (!g_str_equal (iid, applet_iid))
		needs_migration = TRUE;

	g_free (applet_iid);

	if (needs_migration) {
		key = panel_mateconf_full_key (PANEL_MATECONF_APPLETS, id, "applet_iid");
		mateconf_client_set_string (client, key, iid, NULL);
	}

	return g_strdup (iid);
}
