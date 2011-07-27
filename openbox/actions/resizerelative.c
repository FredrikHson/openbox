#include "openbox/actions.h"
#include "openbox/actions_value.h"
#include "openbox/client.h"
#include "openbox/screen.h"
#include "openbox/frame.h"
#include "openbox/config.h"

typedef struct {
    gint left;
    gint left_denom;
    gint right;
    gint right_denom;
    gint top;
    gint top_denom;
    gint bottom;
    gint bottom_denom;
} Options;

static gpointer setup_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_resizerelative_startup(void)
{
    actions_register("ResizeRelative", setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "left");
    if (v && actions_value_is_string(v))
        actions_value_fraction(v, &o->left, &o->left_denom);
    v = g_hash_table_lookup(config, "right");
    if (v && actions_value_is_string(v))
        actions_value_fraction(v, &o->right, &o->right_denom);
    v = g_hash_table_lookup(config, "top");
    if (v && actions_value_is_string(v))
        actions_value_fraction(v, &o->top, &o->top_denom);
    v = g_hash_table_lookup(config, "bottom");
    if (v && actions_value_is_string(v))
        actions_value_fraction(v, &o->bottom, &o->bottom_denom);

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        ObClient *c = data->client;
        gint x, y, ow, xoff, nw, oh, yoff, nh, lw, lh;
        gint left = o->left, right = o->right, top = o->top, bottom = o->bottom;

        if (o->left_denom)
            left = (left * c->area.width / c->size_inc.width) / o->left_denom;
        if (o->right_denom)
            right = (right * c->area.width / c->size_inc.width) / o->right_denom;
        if (o->top_denom)
            top = (top * c->area.height / c->size_inc.height) / o->top_denom;
        if (o->bottom_denom)
            bottom = (bottom * c->area.height / c->size_inc.height) / o->bottom_denom;

        x = c->area.x;
        y = c->area.y;
        ow = c->area.width;
        xoff = -left * c->size_inc.width;
        nw = ow + right * c->size_inc.width
            + left * c->size_inc.width;
        oh = c->area.height;
        yoff = -top * c->size_inc.height;
        nh = oh + bottom * c->size_inc.height
            + top * c->size_inc.height;

        client_try_configure(c, &x, &y, &nw, &nh, &lw, &lh, TRUE);
        xoff = xoff == 0 ? 0 :
            (xoff < 0 ? MAX(xoff, ow-nw) : MIN(xoff, ow-nw));
        yoff = yoff == 0 ? 0 :
            (yoff < 0 ? MAX(yoff, oh-nh) : MIN(yoff, oh-nh));

        actions_client_move(data, TRUE);
        client_move_resize(c, x + xoff, y + yoff, nw, nh);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
