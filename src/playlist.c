/*
 * Copyright (c) 2014-2016 gnome-mpv
 *
 * This file is part of GNOME MPV.
 *
 * GNOME MPV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNOME MPV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNOME MPV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "playlist.h"
#include "common.h"
#include "mpv.h"
#include "control_box.h"
#include "playlist_widget.h"

void playlist_row_handler(	GtkTreeView *tree_view,
				GtkTreePath *path,
				GtkTreeViewColumn *column,
				gpointer data )
{
	gmpv_handle *ctx = data;
	gint *indices = gtk_tree_path_get_indices(path);

	if(indices)
	{
		gint64 index = indices[0];

		mpv_set_property(	ctx->mpv_ctx,
					"playlist-pos",
					MPV_FORMAT_INT64,
					&index );
	}
}

void playlist_row_inserted_handler(	GtkTreeModel *tree_model,
					GtkTreePath *path,
					GtkTreeIter *iter,
					gpointer data )
{
	gmpv_handle *ctx = data;

	ctx->playlist_move_dest = gtk_tree_path_get_indices(path)[0];
}

void playlist_row_deleted_handler(	GtkTreeModel *tree_model,
					GtkTreePath *path,
					gpointer data )
{
	gmpv_handle *ctx = data;
	const gchar *cmd[] = {"playlist_move", NULL, NULL, NULL};
	gchar *src_str;
	gchar *dest_str;
	gint src;
	gint dest;

	src = gtk_tree_path_get_indices(path)[0];
	dest = ctx->playlist_move_dest;

	if(dest >= 0)
	{
		src_str = g_strdup_printf("%d", (src > dest)?--src:src);
		dest_str = g_strdup_printf("%d", dest);
		ctx->playlist_move_dest = -1;

		cmd[1] = src_str;
		cmd[2] = dest_str;

		mpv_command(ctx->mpv_ctx, cmd);

		g_free(src_str);
		g_free(dest_str);
	}
}

void playlist_remove_current_entry(gmpv_handle *ctx)
{
	const gchar *cmd[] = {"playlist_remove", NULL, NULL};
	PlaylistWidget *playlist;
	GtkTreePath *path;

	playlist = PLAYLIST_WIDGET(ctx->gui->playlist);

	gtk_tree_view_get_cursor
		(	GTK_TREE_VIEW(playlist->tree_view),
			&path,
			NULL );

	if(path)
	{
		gint index;
		gchar *index_str;

		index = gtk_tree_path_get_indices(path)[0];
		index_str = g_strdup_printf("%d", index);
		cmd[1] = index_str;

		g_signal_handlers_block_matched
			(	playlist->list_store,
				G_SIGNAL_MATCH_DATA,
				0,
				0,
				NULL,
				NULL,
				ctx );

		playlist_widget_remove(playlist, index);

		if(ctx->loaded)
		{
			mpv_check_error(mpv_command(ctx->mpv_ctx, cmd));
		}

		if(playlist_widget_empty(playlist))
		{
			control_box_set_enabled
				(CONTROL_BOX(ctx->gui->control_box), FALSE);
		}

		g_signal_handlers_unblock_matched
			(	playlist->list_store,
				G_SIGNAL_MATCH_DATA,
				0,
				0,
				NULL,
				NULL,
				ctx );

		g_free(index_str);
	}
}

void playlist_reset(gmpv_handle *ctx)
{
	PlaylistWidget *playlist = PLAYLIST_WIDGET(ctx->gui->playlist);

	playlist_widget_set_indicator_pos(playlist, 0);
}
