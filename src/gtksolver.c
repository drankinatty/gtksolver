#include <glib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkkeysyms.h>  /* for GDK key values */
#include "mtrx_t.h"

#define PACKAGE "gtksolver"
#define VERSION "0.0.1"
#define IMGDIR  "img"
#define LOGO    "gtksolver.png"
#define ICON    "gtksolver.ico"
#define LICENSE "LICENSE"
#define SITE    "https://www.rankinlawfirm.com"

#define APPNAME "GtkSolver"
#define APPSTR "Linear System Solver"
// #define APPSTR "Linear System of Equations Solver"

/* default window width and height */
#define WWIDTH  640
#define WHEIGHT 450

/* set default font string */
#define FONTSTR "DejaVu Sans Mono 8"

#if defined (_WIN32) || defined (_WIN64) || defined (_WINNT)
 #define HAVEMSWIN 1
#endif

/* size_t format specifier for older MinGW compiler versions */
#if defined (_WIN32)
#define SZTFMT "u"
#elif defined (_WIN64)
#define SZTFMT "lu"
#else
#define SZTFMT "zu"
#endif

typedef struct app {
    GtkWidget   *window,        /* main window */
                *toolbar,       /* main toolbar */
                *text_view,     /* text view */
                *btnsolv,       /* solve button */
                *menusolv,      /* solve menu entry */
                *toolsolv;      /* solve toolbar entry */
    GtkTextTag  *tagblue,       /* Deep sky blue tag */
                *tagroyal,      /* Royal blue tag */
                *tagred;        /* Red tag */
    gboolean    showtoolbar,    /* show toolbar */
                bottombuttons;  /* display bottom buttons */
} app_t;

/* set sensitivity of solver widgets */
void setsolvsensitive (gpointer data, gboolean state)
{
    app_t *inst = data;
    gtk_widget_set_sensitive (inst->btnsolv, state);
    gtk_widget_set_sensitive (inst->menusolv, state);
    gtk_widget_set_sensitive (inst->toolsolv, state);
}

/* Solve... button callback */
void btnsolv_activate (GtkWidget *widget, gpointer data)
{
    app_t *inst = data;
    gint line;
    size_t i;
    gchar *buf, c, last = 0, prevlast = 0;
    gboolean havevalue = FALSE;
    GtkTextIter start, end, lastiter, previter;
    // GtkTextBuffer *buffer = GTK_TEXT_BUFFER(data);
    GtkTextBuffer *buffer;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(inst->text_view));

    /* get start and end iters for buffer */
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);
    lastiter = start;

    /* advance start iter to beginning of first value
     * (updated to handle -0.4 or -.4)
     */
    do {
        c = gtk_text_iter_get_char (&start);
        if (isdigit (c)) {
            // if (last == '.' || last == '-' || last == '+')
            if (last == '.' || last == '-' || last == '+') {
                if (last == '.' && (prevlast == '-' || prevlast == '+'))
                    start = previter;
                else
                    start = lastiter;
            }
            havevalue = TRUE;
            break;
        }
        prevlast = last ? last : c;
        last = c;
        previter = lastiter;
        lastiter = start;
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
    // gtk_widget_set_sensitive (inst->btnsolv, FALSE);
    setsolvsensitive (data, FALSE);

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
    // gtk_widget_set_sensitive (inst->btnsolv, FALSE);
    setsolvsensitive (data, FALSE);

    if (widget || data) {}
}

/* Exit... button callback */
void btnexit_activate (GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();

    if (widget || data) {}
}

/* create tag table for text colors */
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

/** set intro help on initial window display and help menu/toolbar */
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
    // gtk_widget_set_sensitive (inst->btnsolv, FALSE);
    setsolvsensitive (data, FALSE);
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
    // app_t *inst = data;

    /* set [Solve...] button(s) sensitivity */
    // gtk_widget_set_sensitive (inst->btnsolv, TRUE);
    setsolvsensitive (data, TRUE);

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

/*
 * menu callback functions
 *
 *  _File menu
 */
void menu_file_clear_activate (GtkMenuItem *menuitem, gpointer data)
{
    btnclear_activate (NULL, data);
    if (menuitem || data) {}
}
void menu_file_solv_activate (GtkMenuItem *menuitem, gpointer data)
{
    btnsolv_activate (NULL, data);
    if (menuitem || data) {}
}
void menu_file_quit_activate (GtkMenuItem *menuitem, gpointer data)
{
    gtk_main_quit ();
    if (menuitem || data) {}
}

/*
 * _Help menu functions
 */
void help_about (gpointer data)
{
    app_t *app = data;
    gchar *buf, *logofn, *license;
    gsize fsize;
    GdkPixbuf *logo = NULL;

    static const gchar *const authors[] = {
            "David C. Rankin, J.D,P.E. <drankinatty@gmail.com>",
            NULL
    };

    static const gchar copyright[] = \
            "Copyright \xc2\xa9 2019 David C. Rankin";

    static const gchar comments[] = APPSTR;

    /* create pixbuf from logofn to pass to show_about_dialog */
    // if ((logofn = g_strdup_printf ("%s/%s", app->imgdir, LOGO))) {
    if ((logofn = g_strdup_printf ("%s/%s", IMGDIR, LOGO))) {
        // g_print ("logofn : '%s'\n", logofn);
        logo = create_pixbuf_from_file (logofn);
        g_free (logofn);
    }

/* TODO; get system and user directories based on OS and distribution */
// #ifndef HAVEMSWIN
//     /* Archlinux */
//     license = g_strdup_printf ("%s/%s/%s", "/usr/share/licenses", CFGDIR, LICENSE);
//     if (g_file_test (license, G_FILE_TEST_EXISTS))
//         goto gotnixlic;
//
//     /* openSuSE */
//     g_free (license);
//     license = NULL;
//     license = g_strdup_printf ("%s/%s/%s", "/usr/share/doc/packages", CFGDIR, LICENSE);
//     if (g_file_test (license, G_FILE_TEST_EXISTS))
//         goto gotnixlic;
//
//     /* generic */
//     g_free (license);
//     license = NULL;
//     license = g_strdup_printf ("%s/%s", app->sysdatadir, LICENSE);
//
//     gotnixlic:;
// #else
//     /* win32/win64 */
//     license = g_strdup_printf ("%s\\%s", app->sysdatadir, LICENSE);
// #endif

    license = g_strdup (LICENSE);

    /* check if license read into buf */
//     if (g_file_get_contents (license, &buf, &fsize, NULL)) {
    if (g_file_get_contents (LICENSE, &buf, &fsize, NULL)) {

        if (logo)   /* show logo */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VERSION,
                                "website", SITE,
                                "program-name", APPNAME,
                                "logo", logo,
                                "license", buf,
                                NULL);
        else    /* show stock GTK_STOCK_EDIT */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VERSION,
                                "website", SITE,
                                "program-name", APPNAME,
                                "logo-icon-name", GTK_STOCK_EDIT,
                                "license", buf,
                                NULL);
        g_free (buf);
    }
    else {
#ifdef DEBUG
        g_warning ("error: load of license file %s failed.\n", LICENSE);
#endif
        if (logo)   /* show logo */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VERSION,
                                "website", SITE,
                                "program-name", APPNAME,
                                "logo", logo,
                                NULL);
        else    /* show stock GTK_STOCK_EDIT */
            gtk_show_about_dialog (GTK_WINDOW (app->window),
                                "authors", authors,
                                "comments", comments,
                                "copyright", copyright,
                                "version", VERSION,
                                "website", SITE,
                                "program-name", APPNAME,
                                "logo-icon-name", GTK_STOCK_EDIT,
                                NULL);
    }

    if (logo)
        g_object_unref (logo);
    if (license)
        g_free (license);
}

/*
 *  _Help menu
 */
void menu_help_help_activate (GtkMenuItem *menuitem, gpointer data)
{
    btnhelp_activate (NULL, data);
    if (menuitem || data) {}
}
void menu_help_about_activate (GtkMenuItem *menuitem, gpointer data)
{
    help_about (data);
    if (menuitem || data) {}
}

/* create menubar and return pointer to caller */
GtkWidget *create_menubar (gpointer data, GtkAccelGroup *mainaccel)
{
    app_t *inst = data;

    GtkWidget   *menubar,       /* menu container   */
                *fileMenu,      /* file menu        */
                *fileMi,
                *clearMi,
                *solvMi,
                *sep,
                *quitMi;

    GtkWidget   *helpMenu,      /* help menu        */
                *helpMi,
                *helpI,
                *aboutMi;

    /* create menubar, menus and submenus */
    menubar             = gtk_menu_bar_new ();
        fileMenu        = gtk_menu_new ();
        helpMenu        = gtk_menu_new ();

    /* define file menu */
    fileMi = gtk_menu_item_new_with_mnemonic ("_File");
    sep = gtk_separator_menu_item_new ();
    clearMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLEAR,
                                                   NULL);
    solvMi   = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES,
                                                   NULL);
    gtk_menu_item_set_label (GTK_MENU_ITEM (solvMi), "_Solve");
    // gtk_menu_item_set_label (GTK_MENU_ITEM (solvMi), "_Solve System");
    quitMi   = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);

    inst->menusolv = solvMi;    /* set widget in struct */

    /* create entries under 'File' then add to menubar */
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (fileMi), fileMenu);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), clearMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), solvMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu),
                           gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (fileMenu), quitMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), fileMi);

    gtk_widget_add_accelerator (clearMi, "activate", mainaccel,
                                GDK_KEY_e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (solvMi, "activate", mainaccel,
                                GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (quitMi, "activate", mainaccel,
                                GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    /*define help menu */
    helpMi = gtk_menu_item_new_with_mnemonic ("_Help");
    // gtk_menu_item_set_right_justified ((GtkMenuItem *)helpMi, TRUE);
    sep = gtk_separator_menu_item_new ();
    helpI = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP,
                                                  NULL);
    aboutMi = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT,
                                                  NULL);

    /* create entries under 'Help' then add to menubar */
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (helpMi), helpMenu);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpMenu), sep);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpMenu), helpI);
    gtk_menu_shell_append (GTK_MENU_SHELL (helpMenu), aboutMi);
    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), helpMi);

    gtk_widget_add_accelerator (helpI, "activate", mainaccel,
                                GDK_KEY_h, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (aboutMi, "activate", mainaccel,
                                GDK_KEY_a, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    /* File Menu */
    g_signal_connect (G_OBJECT (clearMi), "activate",       /* file Clear */
                      G_CALLBACK (menu_file_clear_activate), data);

    g_signal_connect (G_OBJECT (solvMi), "activate",        /* file Solv  */
                      G_CALLBACK (menu_file_solv_activate), data);

    g_signal_connect (G_OBJECT (quitMi), "activate",        /* file Quit  */
                      G_CALLBACK (menu_file_quit_activate), data);

    /* Help Menu */
    g_signal_connect (G_OBJECT (helpI), "activate",         /* help Info  */
                      G_CALLBACK (menu_help_help_activate), data);

    g_signal_connect (G_OBJECT (aboutMi), "activate",       /* help About */
                      G_CALLBACK (menu_help_about_activate), data);

    gtk_widget_show_all (menubar);

    return menubar;
}

GtkWidget *create_toolbar (GtkAccelGroup *mainaccel, gpointer data)
{
    app_t *inst = data;

    GtkWidget *toolbar;

    GtkToolItem *clear,         /* clear, solv, quit, help */
                *solv,
                *quit,
                *help;

    /* create toolbar, set overflow arrow on */
    toolbar = gtk_toolbar_new ();
    gtk_toolbar_set_show_arrow (GTK_TOOLBAR(toolbar), TRUE);

    /* toolbar styles
     *   GTK_TOOLBAR_ICONS,
     *   GTK_TOOLBAR_TEXT,
     *   GTK_TOOLBAR_BOTH,
     *   GTK_TOOLBAR_BOTH_HORIZ  (text only shown when is_important set)
     */
    // gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    // gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_TEXT);
    // gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH_HORIZ);

    clear = gtk_tool_button_new_from_stock(GTK_STOCK_CLEAR);
    gtk_tool_item_set_is_important (clear, TRUE);
    // gtk_tool_item_set_homogeneous (clear, FALSE);
    gtk_tool_item_set_homogeneous (clear, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), clear, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(clear), "Clear Content ");

    solv = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
    gtk_tool_item_set_is_important (solv, TRUE);
    // gtk_tool_item_set_homogeneous (solv, FALSE);
    gtk_tool_item_set_homogeneous (solv, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), solv, -1);
    gtk_tool_button_set_label (GTK_TOOL_BUTTON(solv), "Solve");
    gtk_widget_set_tooltip_text (GTK_WIDGET(solv), "Solve Shown System ");

    inst->toolsolv = GTK_WIDGET(solv);  /* set inst struct member */

    quit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_tool_item_set_is_important (quit, TRUE);
    // gtk_tool_item_set_homogeneous (quit, FALSE);
    gtk_tool_item_set_homogeneous (quit, TRUE);
    // gtk_toolbar_insert(GTK_TOOLBAR(toolbar), quit, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(quit), "Quit ");

    help = gtk_tool_button_new_from_stock(GTK_STOCK_HELP);
    gtk_tool_item_set_is_important (help, TRUE);
    // gtk_tool_item_set_homogeneous (help, FALSE);
    gtk_tool_item_set_homogeneous (help, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), help, -1);
    gtk_widget_set_tooltip_text (GTK_WIDGET(help), "Display Initial Help ");

    /* test putting quit after help */
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), quit, -1);

    /* File Menu */
    g_signal_connect (G_OBJECT (clear), "clicked",          /* file Clear */
                      G_CALLBACK (menu_file_clear_activate), data);

    g_signal_connect (G_OBJECT (solv), "clicked",           /* file Solv  */
                      G_CALLBACK (menu_file_solv_activate), data);

    g_signal_connect (G_OBJECT (quit), "clicked",           /* file Quit  */
                      G_CALLBACK (menu_file_quit_activate), data);

    /* Help Menu */
    g_signal_connect (G_OBJECT (help), "clicked",           /* help Info  */
                      G_CALLBACK (menu_help_help_activate), data);

    gtk_widget_show_all (toolbar);

    return toolbar;

    if (mainaccel) {} /* stub to prevent [-Wunused-parameter] */
}

/* create scrolled window containing textview with Fixed Font */
GtkWidget *create_scrolled_view (gpointer data)
{
    app_t *inst = data;

    GtkWidget   *scrolled_window;
    GtkTextBuffer *buffer;
    PangoFontDescription *font_desc;

    /* create the scrolled window. NULL is passed for both parameters to
     * size horizontal/vertical adjustments automatically. Setting the
     * scrollbar policy to automatic show scrollbars  when needed.
     */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 5);

    /* The text buffer represents for textview */
    buffer = gtk_text_buffer_new (NULL);

    /* create text_view to display editable text buffer.
     * line wrapping is set to NONE so coefficient matrix lines do not wrap.
     */
    inst->text_view = gtk_text_view_new_with_buffer (buffer);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (inst->text_view), GTK_WRAP_NONE);

    /* set default font throughout the widget */
    font_desc = pango_font_description_from_string (FONTSTR);
    gtk_widget_modify_font (inst->text_view, font_desc);
    pango_font_description_free (font_desc);

    /* change left margin throughout the widget */
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (inst->text_view), 10);

    /* initialize tag table for text_view buffer */
    settagtable (inst);

    /* add text_view to scrolled_window */
    gtk_container_add (GTK_CONTAINER (scrolled_window), inst->text_view);

    /* connect textview callbacks */
    g_signal_connect (buffer, "changed",
                      G_CALLBACK (on_buffer_changed), inst);

    return scrolled_window;
}

GtkWidget *create_bottombtnbar (gpointer data)
{
    app_t *inst = data;

    GtkWidget   *hbox,
                *btnclear,
                *btnexit,
                *btnhelp;

    /* Create hbox to hold horizontal row of buttons at bottom of window */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_set_spacing (GTK_BOX (hbox), 5);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

    /* define the buttons and pack into hbox */
    inst->btnsolv = gtk_button_new_with_mnemonic ("_Solve...");
    gtk_widget_set_size_request (inst->btnsolv, 80, 24);
    gtk_box_pack_start (GTK_BOX (hbox), inst->btnsolv, FALSE, FALSE, 0);

    btnclear = gtk_button_new_with_mnemonic ("_Clear");
    gtk_widget_set_size_request (btnclear, 80, 24);
    gtk_box_pack_start (GTK_BOX (hbox), btnclear, FALSE, FALSE, 0);

    btnexit = gtk_button_new_with_mnemonic ("_Quit");
    gtk_widget_set_size_request (btnexit, 80, 24);
    gtk_box_pack_end (GTK_BOX (hbox), btnexit, FALSE, FALSE, 0);

    btnhelp = gtk_button_new_with_mnemonic ("_Help");
    gtk_widget_set_size_request (btnhelp, 80, 24);
    gtk_box_pack_end (GTK_BOX (hbox), btnhelp, FALSE, FALSE, 0);

    /* connect button callbacks */
    g_signal_connect (inst->btnsolv, "clicked",
                      G_CALLBACK (btnsolv_activate), inst);

    g_signal_connect (btnclear, "clicked",
                      G_CALLBACK (btnclear_activate), inst);

    g_signal_connect (btnhelp, "clicked",
                      G_CALLBACK (btnhelp_activate), inst);

    g_signal_connect (btnexit, "clicked",
                      G_CALLBACK (btnexit_activate), inst);

    gtk_widget_show_all (hbox);

    return hbox;
}

/** create application main window, returning pointer to
 *  main window on success, NULL otherwise.
 */
GtkWidget *create_window (app_t *inst)
{
    GtkWidget   *window,            /* main window & layout containers */
                *scrolled_window,
                *vbox,
                *hbox;

    GtkWidget   *menubar,
                *toolbar;

    gchar *iconfile;
    GtkAccelGroup *mainaccel;

    /* Create a window with a title, and a default size (win, w, h) */
    if (!(window = gtk_window_new(GTK_WINDOW_TOPLEVEL)))
        return NULL;
    inst->window = window;
    gtk_window_set_title (GTK_WINDOW (window), "Linear System Solver");
    gtk_window_set_default_size (GTK_WINDOW (window), WWIDTH, WHEIGHT);

    /* create icon filename and set icon */
    if ((iconfile = g_strdup_printf ("%s/%s", IMGDIR, ICON))) {
        GdkPixbuf *pixbuf = create_pixbuf_from_file (iconfile);
        if (pixbuf) {
            gtk_window_set_icon(GTK_WINDOW(window), pixbuf);
            g_object_unref (pixbuf);
        }
        g_free (iconfile);
    }

    /* create & attach keyboard accelerator group */
    mainaccel = gtk_accel_group_new ();
    gtk_window_add_accel_group (GTK_WINDOW (window), mainaccel);

    /* create vbox and add as main container in window */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    /* create menubar and menus to add */
    menubar = create_menubar (inst, mainaccel);
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
    gtk_widget_show (menubar);

    /* create toolbar
     * GTK_TOOLBAR_ICONS, GTK_TOOLBAR_TEXT, GTK_TOOLBAR_BOTH, GTK_TOOLBAR_BOTH_HORIZ
     */
    toolbar = create_toolbar (mainaccel, inst);
    inst->toolbar = toolbar;
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_widget_show (toolbar);

    /* Create the scrolled window containning text_view with fixed font */
    if (!(scrolled_window = create_scrolled_view (inst)))
        return NULL;

    /* pack scrolled_window into vbox */
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    /* Create hbox to hold horizontal row of buttons at bottom of window */
    hbox = create_bottombtnbar (inst);
    /* Pack the hbox at the end of the vbox */
    gtk_box_pack_end (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    /* connect window close callback */
    g_signal_connect (window, "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    /* Show intro text */
    intro (inst);

    gtk_widget_show_all (window);

    /* enable/disable toolbar or bottom button bar based on CLI arguments */
    gtk_widget_set_visible (toolbar, inst->showtoolbar);
    gtk_widget_set_visible (hbox, inst->bottombuttons);

    gtk_widget_grab_focus (inst->text_view);    /* set focus on text_view */

    return window;
}

/* check & process command line options */
void init_iface (app_t *inst, int argc, char **argv)
{
    int i = 1;

    /* set default TRUE for default interface widgets */
    inst->showtoolbar = TRUE;

    /* iterate over command line options */
    for (; i < argc; i++) {
        if (*argv[i] == '-') {
            switch (argv[i][1]) {
                case 'b':
                    inst->bottombuttons = TRUE;
                    break;
                case 't':
                    inst->showtoolbar = FALSE;
                    break;
            }
        }
    }
}

int main (int argc, char **argv)
{
    app_t inst = { .text_view = NULL };
    gtk_init (&argc, &argv);

    init_iface (&inst, argc, argv);
    if (!create_window (&inst)) {
        g_error ("failed to create window.\n");
        return 1;
    }

    gtk_main();

    return 0;
}
