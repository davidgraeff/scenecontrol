#include <stdbool.h>

#include <stdio.h>     /* standard I/O functions                         */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <sys/types.h> /* various type definitions, like pid_t           */
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <sys/prctl.h> /* to get informed when parent dies               */
#include <string.h>
#include <time.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <glib.h>
#include <locale.h>

/* PA: http://bazaar.launchpad.net/~indicator-applet-developers/indicator-sound/trunk/annotate/head%3A/src/pulse-manager.c*/
typedef struct {
    gchar* name;
    gchar* description;
    gchar* icon_name;
    gint index;
    gint device_index;
    pa_cvolume volume;
    pa_channel_map channel_map;
    gboolean mute;
    gboolean active_port;
    pa_volume_t base_volume;
} sink_info;

// Until we find a satisfactory default sink this index should remain < 0
static gint DEFAULT_SINK_INDEX = -1;
static gboolean pa_server_available = FALSE;
// PA related
static pa_context *pulse_context = NULL;
static pa_glib_mainloop *pa_main_loop = NULL;
static void context_state_callback(pa_context *c, void *userdata);
static void pulse_sink_info_callback(pa_context *c, const pa_sink_info *sink_info, int eol, void *userdata);
static void pulse_sink_input_info_callback(pa_context *c, const pa_sink_input_info *info, int eol, void *userdata);
static void pulse_server_info_callback(pa_context *c, const pa_server_info *info, void *userdata);
static void update_sink_info(pa_context *c, const pa_sink_info *info, int eol, void *userdata);
static void destroy_sink_info(void *value);
static void reconnect_to_pulse();
static void output_volume(sink_info *value);

// This is where we'll store the output device list
static GHashTable *sink_hash = NULL;

static void gather_pulse_information(pa_context *c, void *userdata)
{
    pa_operation *operation;
    if (!(operation = pa_context_get_server_info(c, pulse_server_info_callback, userdata)))
    {
        g_warning("pa_context_get_server_info failed");
        if (!(operation = pa_context_get_sink_info_list(c, pulse_sink_info_callback, NULL)))
        {
            g_warning("pa_context_get_sink_info_list() failed - cannot fetch server or sink info - leaving . . .");
            return;
        }
    }
    pa_operation_unref(operation);
    return;
}

/**
On Service startup this callback will be called multiple times resulting our sinks_hash container to be filled with the
available sinks.
For now this callback it assumes it only used at startup. It may be necessary to use if sinks become available after startup.
Major candidate for refactoring.
**/
static void pulse_sink_info_callback(pa_context *, const pa_sink_info *sink, int eol, void *)
{
    if (eol > 0) {
    }
    else {
        sink_info *value;
        value = g_new0(sink_info, 1);
        value->index = value->device_index = sink->index;
        value->name = g_strdup(sink->name);
        value->description = g_strdup(sink->description);
        value->icon_name = g_strdup(pa_proplist_gets(sink->proplist, PA_PROP_DEVICE_ICON_NAME));
        value->active_port = (sink->active_port != NULL);
        value->mute = !!sink->mute;
        value->volume = sink->volume;
        value->base_volume = sink->base_volume;
        value->channel_map = sink->channel_map;
        g_hash_table_insert(sink_hash, value->name, value);
        output_volume(value);
    }
}

static void pulse_default_sink_info_callback(pa_context *c, const pa_sink_info *info, int eol, void *)
{
    if (eol > 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;
        //g_warning("Default Sink info callback failure");
        return;
    }
    else {
        DEFAULT_SINK_INDEX = info->index;
        GList *keys = g_hash_table_get_keys(sink_hash);
        gint position =  g_list_index(keys, info->name);
        // Only update sink-list if the index is not in our already fetched list.
        if (position < 0)
        {
            pa_operation_unref(pa_context_get_sink_info_list(c, pulse_sink_info_callback, NULL));
        }
        else
        {
//             update_pa_state(TRUE, determine_sink_availability(), default_sink_is_muted(), get_default_sink_volume());
        }
    }
}

static void pulse_sink_input_info_callback(pa_context *c, const pa_sink_input_info *info, int eol, void *) {
    if (eol > 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;
        return;
    }
    else {
        if (info == NULL)
        {
            return;
        }
    }
}

static void output_volume(sink_info *value) {
    pa_volume_t vol = pa_cvolume_avg(&value->volume);
    uint32_t volume = (vol * 10000) / PA_VOLUME_NORM;
    g_print("pa_sink %s %i %u\n", value->name, value->mute, volume);
}

static void update_sink_info(pa_context *, const pa_sink_info *info, int eol, void *)
{
    if (eol > 0 || !info) {
        return;
    }

    sink_info *value = (sink_info *)g_hash_table_lookup(sink_hash, info->name);
    gboolean output = TRUE;
    if (value)
    {
        value->name = g_strdup(info->name);
        value->description = g_strdup(info->description);
        value->icon_name = g_strdup(pa_proplist_gets(info->proplist, PA_PROP_DEVICE_ICON_NAME));
        value->active_port = (info->active_port != NULL);
        output &= (value->mute != !!info->mute) || (pa_cvolume_equal(&info->volume, &value->volume) == 0);
        value->mute = !!info->mute;
        value->volume = info->volume;
        value->base_volume = info->base_volume;
    }
    else
    {
        value = g_new0(sink_info, 1);
        value->index = info->index;
        value->name = g_strdup(info->name);
        value->description = g_strdup(info->description);
        value->icon_name = g_strdup(pa_proplist_gets(info->proplist, PA_PROP_DEVICE_ICON_NAME));
        value->mute = !!info->mute;
        value->volume = info->volume;
        value->base_volume = info->base_volume;
        g_hash_table_insert(sink_hash, value->name, value);
    }

    // output volume
    if (output) {
        output_volume(value);
    }
}


static void pulse_server_info_callback(pa_context *c, const pa_server_info *info, void *userdata)
{
    pa_operation *operation;
    if (info == NULL)
    {
        pa_server_available = FALSE;
        return;
    }
    pa_server_available = TRUE;
    if (info->default_sink_name != NULL)
    {
        if (!(operation = pa_context_get_sink_info_by_name(c, info->default_sink_name, pulse_default_sink_info_callback, userdata)))
        {
            //g_warning("pa_context_get_sink_info_by_name() failed");
        }
        else {
            pa_operation_unref(operation);
            return;
        }
    }
    if (!(operation = pa_context_get_sink_info_list(c, pulse_sink_info_callback, NULL)))
    {
        //g_warning("pa_context_get_sink_info_list() failed");
        return;
    }
    pa_operation_unref(operation);
}

gboolean find_index_in_hash_table (gpointer /*key*/, gpointer value, gpointer user_data)
{
    sink_info *s = (sink_info *)value;
    return (s->index == GPOINTER_TO_INT(user_data));
}

static void subscribed_events_callback(pa_context *c, enum pa_subscription_event_type t, uint32_t index, void *userdata)
{
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
    case PA_SUBSCRIPTION_EVENT_SINK:
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            g_hash_table_remove(sink_hash, g_hash_table_find(sink_hash, find_index_in_hash_table,GINT_TO_POINTER(index)));
            if ((int)index == DEFAULT_SINK_INDEX) {
                DEFAULT_SINK_INDEX = -1;
            }
        }
        else
        {
            pa_operation_unref(pa_context_get_sink_info_by_index(c, index, update_sink_info, userdata));
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
//         g_debug("PA_SUBSCRIPTION_EVENT_SINK_INPUT event triggered!!");
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
        }
        else
        {
            pa_operation_unref(pa_context_get_sink_input_info(c, index, pulse_sink_input_info_callback, userdata));
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SERVER:
//         g_debug("PA_SUBSCRIPTION_EVENT_SERVER change of some description ???");
        pa_operation *o;
        if (!(o = pa_context_get_server_info(c, pulse_server_info_callback, userdata)))
        {
            g_warning("pa_context_get_server_info() failed");
            return;
        }
        pa_operation_unref(o);
        break;
    }
}


static void context_state_callback(pa_context *c, void *userdata) {
    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_UNCONNECTED:
//         g_debug("unconnected");
        break;
    case PA_CONTEXT_CONNECTING:
//         g_debug("connecting");
        break;
    case PA_CONTEXT_AUTHORIZING:
//         g_debug("authorizing");
        break;
    case PA_CONTEXT_SETTING_NAME:
//         g_debug("context setting name");
        break;
    case PA_CONTEXT_FAILED:
	case PA_CONTEXT_TERMINATED:
		fprintf (stderr, "pa_notready\n");
        pa_server_available = FALSE;
        break;
    case PA_CONTEXT_READY:
        pa_operation *o;
		
		g_print("pa_version %u %u\n", pa_context_get_protocol_version(pulse_context), pa_context_get_server_protocol_version(pulse_context));
		
        pa_context_set_subscribe_callback(c, subscribed_events_callback, userdata);

        if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                       (PA_SUBSCRIPTION_MASK_SINK|
                                        PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                        PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                        PA_SUBSCRIPTION_MASK_CLIENT|
                                        PA_SUBSCRIPTION_MASK_SERVER|
                                        PA_SUBSCRIPTION_MASK_CARD), NULL, NULL))) {
            g_warning("pa_context_subscribe() failed");
            return;
        }
        pa_operation_unref(o);

        gather_pulse_information(c, userdata);

        break;
    }
    fflush( stdout );
}

/*
Refine the resolution of the slider or binary scale it to achieve a more subtle volume control.
Use the base volume stored in the sink struct to calculate actual linear volumes.
*/
void set_sink_volume(const char* sinkname, gdouble percent)
{
    if (pa_server_available == FALSE)
        return;

    sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, sinkname);
    if (!s) {
        fprintf (stderr, "pa_setvolume_error sink_not_available\n");
        return;
    }

    pa_volume_t new_volume = (pa_volume_t) ((percent * PA_VOLUME_NORM) / 100);
    pa_cvolume dev_vol;
    pa_cvolume_set(&dev_vol, s->volume.channels, new_volume);
    s->volume = dev_vol;
    pa_operation_unref(pa_context_set_sink_volume_by_name(pulse_context, sinkname, &dev_vol, NULL, NULL));
    output_volume(s);
}

void set_sink_volume_relative(const char* sinkname, gdouble percent) {
    if (pa_server_available == FALSE)
        return;

    sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, sinkname);
    if (!s) {
        fprintf (stderr, "pa_setvolume_error sink_not_available\n");
        return;
    }

    pa_volume_t v = pa_cvolume_avg(&s->volume);
    pa_volume_t new_volume = (pa_volume_t) ((percent* PA_VOLUME_NORM) / 100) + v;
    pa_cvolume dev_vol;
    pa_cvolume_set(&dev_vol, s->volume.channels, new_volume);
    s->volume = dev_vol;
    pa_operation_unref(pa_context_set_sink_volume_by_name(pulse_context, sinkname, &dev_vol, NULL, NULL));
    output_volume(s);
}

// static gdouble get_sink_volume(const char* sinkname)
// {
//     sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, sinkname);
// 	if (!s) return 0;
//     pa_volume_t vol = pa_cvolume_avg(&s->volume);
//     return ((gdouble) vol * 100) / PA_VOLUME_NORM;
// }

/**
  Mute the sink @sinkname. muted>1: toggle mute
  */
void set_sink_muted(const char* sinkname, int muted)
{
    if (pa_server_available == FALSE)
        return;

    sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, sinkname);
    if (!s) {
        fprintf (stderr, "pa_setmuted_error sink_not_available\n");
        return;
    }

    if (muted>1) muted = !s->mute;
    pa_operation_unref(pa_context_set_sink_mute_by_name(pulse_context, sinkname, muted, NULL, NULL));

}

// static gboolean get_sink_muted(const char* sinkname)
// {
//     sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, sinkname);
// 	if (!s) return FALSE;
//     return s->mute;
// }

static void destroy_sink_info(void *value)
{
    sink_info *sink = (sink_info*)value;
    g_free(sink->name);
    g_free(sink->description);
    g_free(sink->icon_name);
    g_free(sink);
}

/**
reconnect_to_pulse()
In the event of Pulseaudio flapping in the wind handle gracefully without
memory leaks !
*/
static void reconnect_to_pulse()
{
    // reset
    if (pulse_context != NULL) {
        pa_context_unref(pulse_context);
        pulse_context = NULL;
    }

    if (sink_hash != NULL) {
        g_hash_table_destroy(sink_hash);
        sink_hash = NULL;
    }

    // reconnect
    pulse_context = pa_context_new(pa_glib_mainloop_get_api(pa_main_loop), "roompulsehelper");
    g_assert(pulse_context);
	sink_hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, destroy_sink_info);
    g_assert(sink_hash);
    // Establish event callback registration
    pa_context_set_state_callback(pulse_context, context_state_callback, NULL);
    pa_context_connect(pulse_context, NULL, PA_CONTEXT_NOFAIL, NULL);
}

void close_pulseaudio()
{
    if (pulse_context) {
        pa_context_unref(pulse_context);
        pulse_context = NULL;
    }
    g_hash_table_destroy(sink_hash);
    pa_glib_mainloop_free(pa_main_loop);
    pa_main_loop = NULL;
}

/////////////////////// END PULSEAUDIO ////////////////////
GMainLoop* mloop;

static void catch_int(int)
{
    /* re-set the signal handler again to catch_int, for next time */
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);
    g_main_loop_quit (mloop);
}

static gboolean readinput (GIOChannel *source, GIOCondition, gpointer) {
    GError* error = 0;
    char *line = 0;
    gsize len;
    gchar* var, *val, *before;
    int unknown_option=0;

    /* Read the data into our buffer */
    //if (!(condition & G_IO_IN)) return true;
    while (g_io_channel_read_line (source, &line, &len, NULL, &error) == G_IO_STATUS_NORMAL)
    {
        if (len==1) continue;
        unknown_option = 0;
        line[len-1] = 0;
        val = strchr(line,' ');
        if (val) {
            var = strtok(line," ");
            val += 1; // Ignore whitespace
            if (!strcmp(var,"pa_volume")) {
                before = val;
                val = strchr(before,' ');
                if (val) {
                    var = strtok(before," ");
                    val += 1; // Ignore whitespace
                    set_sink_volume(var, atof(val));
                } else {
                    sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, before);
                    if (!s) continue;
                    output_volume(s);
                }
            } else if (!strcmp(var,"pa_volume_relative")) {
                before = val;
                val = strchr(before,' ');
                if (val) {
                    var = strtok(before," ");
                    val += 1; // Ignore whitespace
                    set_sink_volume_relative(var, atof(val));
                } else {
                    sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, before);
                    if (!s) continue;
                    output_volume(s);
                }
            } else if (!strcmp(var,"pa_mute")) {
                before = val;
                val = strchr(before,' ');
                if (val) {
                    var = strtok(before," ");
                    val += 1; // Ignore whitespace
                    set_sink_muted(var, atoi(val));
                } else {
                    sink_info *s = (sink_info *)g_hash_table_lookup(sink_hash, before);
                    if (!s) continue;
                    output_volume(s);
                }
            } else
                unknown_option=1;
        } else if (!strcmp(line,"quit")) {
            g_main_loop_quit (mloop);
        } else
            unknown_option=1;
        if (line) g_free(line);

        if (unknown_option)
            fprintf(stderr,"unknown_option\n");
        break;
    }
    return true;
}

int main (int argc, char *argv[])
{
    setlocale(LC_ALL, "C");
    signal(SIGINT, catch_int);
    signal(SIGTERM, catch_int);
    signal(SIGHUP, catch_int);
    prctl(PR_SET_PDEATHSIG, SIGHUP); // quit if parent dies
    srand(time(0));

    /* für Daten auf stdin einen Event-Handler einrichten  */
    GIOChannel* gio_read = g_io_channel_unix_new (fileno(stdin));
    g_io_channel_set_encoding (gio_read, NULL, NULL);
    g_io_add_watch (gio_read, G_IO_IN, readinput, 0);

    pa_main_loop = pa_glib_mainloop_new(g_main_context_default());
    mloop = g_main_loop_new (g_main_context_default(), false);
    reconnect_to_pulse();
    g_main_loop_run (mloop);

    close_pulseaudio();

    /* Clean */
    g_io_channel_unref(gio_read);

    g_print("exit\n");

    return 0;
}
