#include "openbox/actions.h"
#include "openbox/actions_list.h"
#include "openbox/actions_parser.h"
#include "openbox/actions_value.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/event.h"
#include "openbox/focus_cycle.h"
#include "openbox/openbox.h"
#include "gettext.h"
#include "obt/keyboard.h"

typedef struct {
    gboolean linear;
    gboolean dock_windows;
    gboolean desktop_windows;
    gboolean only_hilite_windows;
    gboolean all_desktops;
    gboolean forward;
    gboolean bar;
    gboolean raise;
    ObFocusCyclePopupMode dialog_mode;
    ObActionsList *actions;


    /* options for after we're done */
    gboolean cancel; /* did the user cancel or not */
    guint state;     /* keyboard state when finished */
} Options;

static gpointer setup_func(GHashTable *config,
                           ObActionsIPreFunc *pre,
                           ObActionsIInputFunc *in,
                           ObActionsICancelFunc *c,
                           ObActionsIPostFunc *post);
static gpointer setup_forward_func(GHashTable *config,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *in,
                                   ObActionsICancelFunc *c,
                                   ObActionsIPostFunc *post);
static gpointer setup_backward_func(GHashTable *config,
                                    ObActionsIPreFunc *pre,
                                    ObActionsIInputFunc *in,
                                    ObActionsICancelFunc *c,
                                    ObActionsIPostFunc *post);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);
static void     i_post_func(gpointer options);

void action_cyclewindows_startup(void)
{
    actions_register_i("NextWindow", setup_forward_func, free_func, run_func);
    actions_register_i("PreviousWindow", setup_backward_func, free_func,
                       run_func);
}

static gpointer setup_func(GHashTable *config,
                           ObActionsIPreFunc *pre,
                           ObActionsIInputFunc *input,
                           ObActionsICancelFunc *cancel,
                           ObActionsIPostFunc *post)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->bar = TRUE;
    o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_LIST;

    v = g_hash_table_lookup(config, "linear");
    if (v && actions_value_is_string(v))
        o->linear = actions_value_bool(v);
    v = g_hash_table_lookup(config, "dialog");
    if (v && actions_value_is_string(v)) {
        const gchar *s = actions_value_string(v);
        if (g_strcasecmp(s, "none") == 0)
            o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_NONE;
        else if (g_strcasecmp(s, "icons") == 0)
            o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_ICONS;
    }
    v = g_hash_table_lookup(config, "bar");
    if (v && actions_value_is_string(v))
        o->bar = actions_value_bool(v);
    v = g_hash_table_lookup(config, "raise");
    if (v && actions_value_is_string(v))
        o->raise = actions_value_bool(v);
    v = g_hash_table_lookup(config, "panels");
    if (v && actions_value_is_string(v))
        o->dock_windows = actions_value_bool(v);
    v = g_hash_table_lookup(config, "hilite");
    if (v && actions_value_is_string(v))
        o->only_hilite_windows = actions_value_bool(v);
    v = g_hash_table_lookup(config, "desktop");
    if (v && actions_value_is_string(v))
        o->desktop_windows = actions_value_bool(v);
    v = g_hash_table_lookup(config, "allDesktops");
    if (v && actions_value_is_string(v))
        o->all_desktops = actions_value_bool(v);

    v = g_hash_table_lookup(config, "finalactions");
    if (v && actions_value_is_actions_list(v)) {
        o->actions = actions_value_actions_list(v);
        actions_list_ref(o->actions);
    }
    else {
        ObActionsParser *p = actions_parser_new();
        o->actions = actions_parser_read_string(p,
                                                "focus\n"
                                                "raise\n"
                                                "unshade\n");
        actions_parser_unref(p);
    }

    *input = i_input_func;
    *cancel = i_cancel_func;
    *post = i_post_func;
    return o;
}

static gpointer setup_forward_func(GHashTable *config,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post)
{
    Options *o = setup_func(config, pre, input, cancel, post);
    o->forward = TRUE;
    return o;
}

static gpointer setup_backward_func(GHashTable *config,
                                    ObActionsIPreFunc *pre,
                                    ObActionsIInputFunc *input,
                                    ObActionsICancelFunc *cancel,
                                    ObActionsIPostFunc *post)
{
    Options *o = setup_func(config, pre, input, cancel, post);
    o->forward = FALSE;
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    actions_list_unref(o->actions);
    g_slice_free(Options, o);
}

static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    struct _ObClient *ft;

    ft = focus_cycle(o->forward,
                     o->all_desktops,
                     !o->only_hilite_windows,
                     o->dock_windows,
                     o->desktop_windows,
                     o->linear,
                     TRUE,
                     o->bar,
                     o->dialog_mode,
                     FALSE, FALSE);

    stacking_restore();
    if (o->raise && ft) stacking_temp_raise(CLIENT_AS_WINDOW(ft));

    return TRUE;
}

static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used)
{
    Options *o = options;
    guint mods;

    mods = obt_keyboard_only_modmasks(e->xkey.state);
    if (e->type == KeyRelease) {
        /* remove from the state the mask of the modifier key being
           released, if it is a modifier key being released that is */
        mods &= ~obt_keyboard_keyevent_to_modmask(e);
    }

    if (e->type == KeyPress) {
        KeySym sym = obt_keyboard_keypress_to_keysym(e);

        /* Escape cancels no matter what */
        if (sym == XK_Escape) {
            o->cancel = TRUE;
            o->state = e->xkey.state;
            return FALSE;
        }

        /* There were no modifiers and they pressed enter */
        else if ((sym == XK_Return || sym == XK_KP_Enter) && !initial_state) {
            o->cancel = FALSE;
            o->state = e->xkey.state;
            return FALSE;
        }
    }
    /* They released the modifiers */
    else if (e->type == KeyRelease && initial_state && !(mods & initial_state))
    {
        o->cancel = FALSE;
        o->state = e->xkey.state;
        return FALSE;
    }

    return TRUE;
}

static void i_cancel_func(gpointer options)
{
    Options *o = options;
    o->cancel = TRUE;
    o->state = 0;
}

static void i_post_func(gpointer options)
{
    Options *o = options;
    struct _ObClient *ft;

    ft = focus_cycle(o->forward,
                     o->all_desktops,
                     !o->only_hilite_windows,
                     o->dock_windows,
                     o->desktop_windows,
                     o->linear,
                     TRUE,
                     o->bar,
                     o->dialog_mode,
                     TRUE, o->cancel);

    if (ft)
        actions_run_acts(o->actions, OB_USER_ACTION_KEYBOARD_KEY,
                         o->state, -1, -1, 0, OB_FRAME_CONTEXT_NONE, ft);

    stacking_restore();
}
