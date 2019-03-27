#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "mtrx_t.h"

#define PACKAGE "gtksolver"
#define VERSION "0.0.1"
#define IMGDIR  "img"
#define LOGO    "gtksolver.png"
#define ICON    "gtksolver.ico"

/* size_t format specifier for older MinGW compiler versions */
#if defined (_WIN32)
#define SZTFMT "u"
#elif defined (_WIN64)
#define SZTFMT "lu"
#else
#define SZTFMT "zu"
#endif

typedef struct app {
    GtkWidget   *text_view,     /* text view */
                *btnsolv;       /* solve button */
    GtkTextTag  *tagblue,       /* Deep sky blue tag */
                *tagroyal,      /* Royal blue tag */
                *tagred;        /* Red tag */
} app_t;

/* Solve... button callback */
void btnsolv_activate (GtkWidget *widget, gpointer data)
{
    app_t *inst = data;
    gint line;
    size_t i;
    gchar *buf, c;
    gboolean havevalue = FALSE;
    GtkTextIter start, end;
    // GtkTextBuffer *buffer = GTK_TEXT_BUFFER(data);
    GtkTextBuffer *buffer;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(inst->text_view));

    /* get start and end iters for buffer */
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);

    /* advance start iter to beginning of first value */
    do {
        c = gtk_text_iter_get_char (&start);
        if (c == '.' || c == '-' || c == '+' || isdigit (c)) {
            havevalue = TRUE;
            break;
        }
    } while (gtk_text_iter_forward_char (&start) &&
            !gtk_text_iter_equal (&start, &end));

    /* suppress incremental update of text_view */
    gtk_text_buffer_begin_user_action (buffer);

    /* get end iter for ouput */
    gtk_text_buffer_get_end_iter (buffer, &end);

    /* save current line to apply tag when done */
    line = gtk_text_iter_get_line (&end);

    if (havevalue) {
        /* get text from buffer */
        buf = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

        /* fill matrix from values in buffer */
        mtrx_t *m = mtrx_read_alloc_buf (buf);

        /* solve system by gauss-jordan elimintation will full-pivoting */
        mtrx_solv_gaussj (m->mtrx, m->rows);    /* m->mtrx now contains
                                                 * inverse + solution vector
                                                 */

        /* output formatted solution vector */
        gtk_text_buffer_insert (buffer, &end, "\n\nSolution Vector:\n\n", -1);
        for (i = 0; i < m->rows; i++) { /* output formatted solution vector */
            gchar *x;
            x = g_strdup_printf (" x[%3" SZTFMT "] : % 11.7f\n",
                                    i, m->mtrx[i][m->rows]);
            gtk_text_buffer_get_end_iter (buffer, &end);
            gtk_text_buffer_insert (buffer, &end, x, -1);
            g_free(x);
        }
        g_free (buf);
    }
    else
        /* output format error - no system of equations found */
        gtk_text_buffer_insert (buffer, &end,
            "\n\n => ERROR: No Numeric Value Found In Buffer\n", -1);

    /* get iters around solution vector title (lines +2 & 3 from save) */
    gtk_text_buffer_get_iter_at_line (buffer, &start, line+2);
    gtk_text_buffer_get_iter_at_line (buffer, &end, line+3);
    /* color output blue/red based on havevalue */
    gtk_text_buffer_apply_tag (buffer,
                                havevalue ? inst->tagblue: inst->tagred,
                                &start, &end);

    /* restore normal text_view behavior */
    gtk_text_buffer_end_user_action (buffer);

    /* set buffer modified false, set [Solve...] sensitivity */
    gtk_text_buffer_set_modified (buffer, FALSE);
    gtk_widget_set_sensitive (inst->btnsolv, FALSE);

    if(widget) {}
}

/* Clear... button callback */
void btnclear_activate (GtkWidget *widget, gpointer data)
{
    app_t *inst = data;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(inst->text_view));

    gtk_text_buffer_set_text (buffer, "", -1);

    /* set buffer modified false, set [Solve...] sensitivity */
    gtk_text_buffer_set_modified (buffer, FALSE);
    gtk_widget_set_sensitive (inst->btnsolv, FALSE);

    if (widget || data) {}
}

/* Exit... button callback */
void btnexit_activate (GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();

    if (widget || data) {}
}

static void settagtable (gpointer data)
{
    app_t *inst = data;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(inst->text_view));

    /* tags to highlight button actions in intro
     *   X11 Color - Deep sky blue - "#00BFFF"
     */
    inst->tagblue = gtk_text_buffer_create_tag (buffer, "blue_foreground",
                            "foreground", "#00BFFF", NULL);
    /*   X11 Color - Royal blue - "#4169E1" */
    inst->tagroyal = gtk_text_buffer_create_tag (buffer, "royal_foreground",
                            "foreground", "#4169E1", NULL);
    /*   X11 Color - Red */
    inst->tagred = gtk_text_buffer_create_tag (buffer, "red_foreground",
                            "foreground", "red", NULL);
}

static void intro (gpointer data)
{
    app_t *inst = data;
    GtkTextIter iter, end;
    GtkTextTag  *tagblue = inst->tagblue;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(inst->text_view));
    // GtkTextTag *tag/*, *tagg*/;

    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
    // gtk_text_buffer_insert (buffer, &iter, "Linear System of Equations Solver\n\n", -1);
    gtk_text_iter_forward_lines (&iter, 1);
    gtk_text_buffer_insert (buffer, &iter, "\nClick [Clear] and enter Coefficient Matrix with Constant Vector as Last Column\n\n", -1);
    gtk_text_iter_forward_lines (&iter, 1);
    gtk_text_buffer_insert (buffer, &iter, "Example (3 x 4):\n\n"
                                " 3.0  2.0  -4.0   3.0\n"
                                " 2.0  3.0   3.0  15.0\n"
                                " 5.0 -3.0   1.0  14.0\n\nor\n\n"
                                "3,2,-4,3\n"
                                "2,3,3,15\n"
                                "5,-3,1,14\n\n", -1);
    gtk_text_iter_forward_lines (&iter, 1);
    gtk_text_buffer_insert (buffer, &iter, "Then click [Solve...]\n\n"
                                "Solution Vector:\n\n"
                                " x[  0] :   3.0000000\n"
                                " x[  1] :   1.0000000\n"
                                " x[  2] :   2.0000000\n", -1);

    /* highlight [Clear...] */
    gtk_text_buffer_get_iter_at_line_offset (buffer, &iter, 1, 6);
    gtk_text_buffer_get_iter_at_line_offset (buffer, &end, 1, 13);
    gtk_text_buffer_apply_tag (buffer, tagblue, &iter, &end);

    /* highlight [Solve...] */
    gtk_text_buffer_get_iter_at_line_offset (buffer, &iter, 15, 11);
    gtk_text_buffer_get_iter_at_line_offset (buffer, &end, 15, 21);
    gtk_text_buffer_apply_tag (buffer, tagblue, &iter, &end);

    gtk_text_buffer_set_modified (buffer, FALSE);
    gtk_widget_set_sensitive (inst->btnsolv, FALSE);
}

/* Help... button callback */
void btnhelp_activate (GtkWidget *widget, gpointer data)
{
    app_t *inst = data;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(inst->text_view));

    gtk_text_buffer_set_text (buffer, "", -1);
    intro (data);

    if (widget || data) {}
}

/** on_buffer_changed fires before on_mark_set with all changes.
 *  (even undo) and fires before gtk_text_buffer_get_modified()
 *  reflects change.
 */
void on_buffer_changed (GtkTextBuffer *buffer, gpointer data)
{
    app_t *inst = data;
    // gboolean modified = gtk_text_buffer_get_modified (buffer);

    /* set [Solve...] button sensitivity */
    // gtk_widget_set_sensitive (inst->btnsolv, modified);
    gtk_widget_set_sensitive (inst->btnsolv, TRUE);

    if (buffer) {}
}

/** creates a new pixbuf from filename.
 *  you are responsible for calling g_object_unref() on
 *  the pixbuf when done.
 */
GdkPixbuf *create_pixbuf_from_file (const gchar *filename)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file (filename, &error);

    if (!pixbuf) {
        g_warning (error->message); /* log to terminal window */
        g_error_free (error);
    }

    return pixbuf;
}

static void activate (app_t *inst)
{
    GtkWidget   *window,
                *scrolled_window,
                *vbox,
                *hbox;

    GtkWidget   *btnclear,
                *btnexit,
                *btnhelp;

    GtkWidget   *toplabel;

    gchar *iconfile;
    GtkTextBuffer *buffer;
    PangoFontDescription *font_desc;

    /* Create a window with a title, and a default size (win, w, h) */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Linear System Solver");
    gtk_window_set_default_size (GTK_WINDOW (window), 650, 550);

    /* create icon filename and set icon */
    if ((iconfile = g_strdup_printf ("%s/%s", IMGDIR, ICON))) {
        GdkPixbuf *pixbuf = create_pixbuf_from_file (iconfile);
        if (pixbuf) {
            gtk_window_set_icon(GTK_WINDOW(window), pixbuf);
            g_object_unref (pixbuf);
        }
        g_free (iconfile);
    }

    /* create vbox and add as main container in window */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    /* top label for title */
    toplabel = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (toplabel),
        "<b>Matrix Solver for Linear System of Equations</b>");
    gtk_misc_set_padding (GTK_MISC(toplabel), 0, 10);
    gtk_box_pack_start (GTK_BOX (vbox), toplabel, FALSE, FALSE, 0);
    gtk_widget_show (toplabel);

    /* Create the scrolled window. NULL is passed for both parameters to
     * size horizontal/vertical adjustments automatically. Setting the
     * scrollbar policy to automatic show scrollbars  when needed.
     */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 5);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    /* The text buffer represents for textview */
    buffer = gtk_text_buffer_new (NULL);

    /* Create text_view to display editable text buffer.
     * Line wrapping is set to break lines in between words.
     */
    inst->text_view = gtk_text_view_new_with_buffer (buffer);
    // gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (inst->text_view), GTK_WRAP_WORD);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (inst->text_view), GTK_WRAP_NONE);

    /* Set default font throughout the widget */
    font_desc = pango_font_description_from_string ("DejaVu Sans Mono 8");
    gtk_widget_modify_font (inst->text_view, font_desc);
    pango_font_description_free (font_desc);

    /* Change left margin throughout the widget */
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (inst->text_view), 10);

    /* Initialize tag table for text_view buffer */
    settagtable (inst);

    /* Add text_view to scrolled_window */
    gtk_container_add (GTK_CONTAINER (scrolled_window), inst->text_view);

    /* Create hbox to hold horizontal row of buttons at bottom of window */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_set_spacing (GTK_BOX (hbox), 5);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

    /* Pack the hbox at the end of the vbox */
    gtk_box_pack_end (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    /* define the buttons and pack into hbox */
    inst->btnsolv = gtk_button_new_with_mnemonic ("_Solve...");
    gtk_widget_set_size_request (inst->btnsolv, 80, 24);
    gtk_box_pack_start (GTK_BOX (hbox), inst->btnsolv, FALSE, FALSE, 0);

    btnclear = gtk_button_new_with_mnemonic ("_Clear");
    gtk_widget_set_size_request (btnclear, 80, 24);
    gtk_box_pack_start (GTK_BOX (hbox), btnclear, FALSE, FALSE, 0);

    btnexit = gtk_button_new_with_mnemonic ("_Exit");
    gtk_widget_set_size_request (btnexit, 80, 24);
    gtk_box_pack_end (GTK_BOX (hbox), btnexit, FALSE, FALSE, 0);

    btnhelp = gtk_button_new_with_mnemonic ("_Help");
    gtk_widget_set_size_request (btnhelp, 80, 24);
    gtk_box_pack_end (GTK_BOX (hbox), btnhelp, FALSE, FALSE, 0);

    /* connect window close callback */
    g_signal_connect (window, "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    /* connect textview callbacks */
    g_signal_connect (buffer, "changed",
                      G_CALLBACK (on_buffer_changed), inst);

    /* connect button callbacks */
    g_signal_connect (inst->btnsolv, "clicked",
                      G_CALLBACK (btnsolv_activate), inst);

    g_signal_connect (btnclear, "clicked",
                      G_CALLBACK (btnclear_activate), inst);

    g_signal_connect (btnhelp, "clicked",
                      G_CALLBACK (btnhelp_activate), inst);

    g_signal_connect (btnexit, "clicked",
                      G_CALLBACK (btnexit_activate), inst);

    /* Show intro text */
    intro (inst);

    gtk_widget_show_all (window);
}



int main (int argc, char **argv)
{
    app_t inst = { .text_view = NULL };
    gtk_init(&argc, &argv);

    activate (&inst);

    gtk_main();

    return 0;
}