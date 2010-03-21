#include <gst/gst.h>
#include <stdbool.h>

#include <stdio.h>     /* standard I/O functions                         */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <sys/types.h> /* various type definitions, like pid_t           */
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <string.h>
#include <time.h>

enum enumMode {ModeLinear, ModeLinearRepeating, ModeRandom};

static GMainLoop *mloop;
static GstElement *pipeline;
int m_currenttrack = -1;
guint durationtimer = 0;
enumMode m_currentmode;
GPtrArray* items;

static void selectTrack(int track);

static void print(const char* msg) {
    puts(msg);
    fflush(stdout);
}

/* first, here is the signal handler */
static void catch_int(int )
{
    /* re-set the signal handler again to catch_int, for next time */
    signal(SIGINT, catch_int);
	signal(SIGTERM, catch_int);
    g_main_loop_quit (mloop);
}

static void play() {
    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
    GstState state;
    gst_element_get_state(pipeline, &state, 0, 0);
    print("state playing");
}

static void pausemedia() {
    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
    GstState state;
    gst_element_get_state(pipeline, &state, 0, 0);
    print("state paused");
}

static void stop() {
    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_READY);
    GstState state;
    gst_element_get_state(pipeline, &state, 0, 0);
    print("state stopped");
}

static bool next() {
    if (!items->len) return false;

    if (m_currentmode == ModeLinear) {
        if (m_currenttrack+1>=(int)items->len) return false;
        m_currenttrack+=1;
    } else if (m_currentmode == ModeLinearRepeating) {
        m_currenttrack+=1;
        if (m_currenttrack>=(int)items->len) m_currenttrack = 0;
    } else if (m_currentmode == ModeRandom) {
        m_currenttrack = rand() % items->len;
    }
    selectTrack(m_currenttrack);
    return true;
}

static bool prev() {
    if (!items->len) return false;

    if (m_currentmode == ModeLinear) {
        if (m_currenttrack-1<0) return false;
        m_currenttrack-=1;
    } else if (m_currentmode == ModeLinearRepeating) {
        m_currenttrack-=1;
        if (m_currenttrack<0) m_currenttrack = (int)items->len-1;
    } else if (m_currentmode == ModeRandom) {
        m_currenttrack = rand() % items->len;
    }
    selectTrack(m_currenttrack);
    return true;
}

static void freeelement(gpointer , gpointer)
{
    //g_free(data);
}

static void clear() {
    stop();
    g_ptr_array_foreach(items, freeelement, 0);
    g_ptr_array_free (items, true);
    items = g_ptr_array_new();
}

static void queue(GString* entry) {
    // fix absolute paths (add file:// protocol)
    if (entry->len && entry->str[0] == '/') {
        g_string_prepend(entry, "file://");
    }
    g_print("queued %s", entry->str);
    g_ptr_array_add (items, entry);
}

static void selectTrack(int track) {
    if (track>=(int)items->len || track<0) return;
    // stop duration timer if active
    if (durationtimer) {
        g_source_remove(durationtimer);
        durationtimer = 0;
    }

    m_currenttrack = track;
    GString* uri = (GString*)g_ptr_array_index (items, track);
    if (!uri->len) return;
    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_READY);
    GstState state;
    gst_element_get_state(pipeline, &state, 0, 0);
    g_object_set (G_OBJECT (pipeline), "uri", uri->str, NULL);
    g_print("active %i\n", track);
    play();
	
    GstFormat fmt = GST_FORMAT_TIME;
    long int len;
    if (gst_element_query_duration (pipeline, &fmt, &len))
        g_print("total %ld\n",len/1000000);
}

static void volume(gdouble volume) {
    //get: g_object_get (G_OBJECT(playbin), "volume", &vol, NULL);
    gdouble vol = volume;
    g_object_set (G_OBJECT(pipeline), "volume", vol, NULL);
}

static void position(int position) {
    if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                           GST_SEEK_TYPE_SET, 1000000*(gint64)position,
                           GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
        g_print ("seek_failed\n");
    }

}

static void getposition() {
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 pos;

    if (gst_element_query_position (pipeline, &fmt, &pos)) {
        g_print("current %ld\n", pos/1000000);
    } else
        g_print("current 0\n");
}

static gboolean getduration(gpointer) {
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 pos, len;

    if (gst_element_query_position (pipeline, &fmt, &pos)) {
        g_print("current %ld\n",pos/1000000);
    } else
        g_print("current 0\n");
    if (gst_element_query_duration (pipeline, &fmt, &len)) {
        g_print("total %ld\n",len/1000000);
    } else
        g_print("total 0\n");
    return false;
}
static gboolean bus_call (GstBus *, GstMessage *msg, gpointer)
{
    switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_DURATION:
    {
        durationtimer = g_timeout_add(300, getduration, 0);
        break;
    }
    case GST_MESSAGE_EOS:
    {
        if (!next()) stop();
        break;
    }
    case GST_MESSAGE_ERROR:
    {
        gchar *debug;
        GError *err;

        gst_message_parse_error (msg, &err, &debug);
        g_free (debug);

        if (m_currenttrack<(int)items->len || m_currenttrack>=0) {
            GString* uri = (GString*)g_ptr_array_index (items, m_currenttrack);
            g_error ("GST_MESSAGE_ERROR %s %s", err->message, uri->str);
        } else
            g_error ("GST_MESSAGE_ERROR %s", err->message);
        g_error_free (err);

        //g_main_loop_quit (loop);
        break;
    }
    default:
        break;
    }

    return true;
}

static gboolean readinput (GIOChannel *source, GIOCondition, gpointer) {
    GError* error = 0;
    char *line = 0;
    gsize len;
    gchar* var, *val;

    /* Read the data into our buffer */
    //if (!(condition & G_IO_IN)) return true;
    while (g_io_channel_read_line (source, &line, &len, NULL, &error) == G_IO_STATUS_NORMAL)
    {
        if (len==1) continue;
        line[len-1] = 0;
		val = strchr(line,' ');
        if (val) {
			var = strtok(line," ");
			val += 1; // Ignore whitespace
			//g_print("split: %s:%s\n", var, val);
            if (!strcmp(var,"queue")) {
                queue(g_string_new(val));
            } else if (!strcmp(var,"mode")) {
                if (!strcmp(val,"linear")) m_currentmode = ModeLinear;
                else if (!strcmp(val,"repeat")) m_currentmode = ModeLinearRepeating;
                else if (!strcmp(val,"random")) m_currentmode = ModeRandom;
                else fprintf(stderr,"unknown_mode %s\n",val);
            } else if (!strcmp(var,"select")) {
                selectTrack(atoi(val));
            } else if (!strcmp(var,"volume")) {
                volume(atof(val));
            } else if (!strcmp(var,"position")) {
                position(atoi(val));
            } else
                fprintf(stderr,"unknown_option %s\n",var);
        } else if (!strcmp(line,"clear")) {
            clear();
        } else if (!strcmp(line,"play")) {
            play();
        } else if (!strcmp(line,"pause")) {
            pausemedia();
        } else if (!strcmp(line,"stop")) {
            stop();
        } else if (!strcmp(line,"next")) {
            next();
        } else if (!strcmp(line,"prev")) {
            prev();
        } else if (!strcmp(line,"getposition")) {
            getposition();
        } else if (!strcmp(line,"quit")) {
            g_main_loop_quit (mloop);
        } else
            fprintf(stderr,"unknown_option %s\n",line);
        if (line) g_free(line);
        break;
    }
    return true;
}

int main (int argc, char *argv[])
{
    gst_init (&argc, &argv);
    signal(SIGINT, catch_int);
	signal(SIGTERM, catch_int);
    srand(time(0));

    /* für Daten auf stdin einen Event-Handler einrichten  */
    GIOChannel* gio_read = g_io_channel_unix_new (fileno(stdin));
    g_io_channel_set_encoding (gio_read, NULL, NULL);
    g_io_add_watch (gio_read, G_IO_IN, readinput, 0);

    /* Hauptschleife und GStreamer playbin */
    pipeline = gst_element_factory_make ("playbin2", "player");

    /* GStreamer communication bus */
    {
        GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
        gst_bus_add_signal_watch(bus);
        g_signal_connect(bus, "message", G_CALLBACK(bus_call), 0);
        //gst_bus_add_watch_full (bus, bus_call, NULL);
        gst_object_unref (bus);
    }

    items = g_ptr_array_new();

    /* Print status information */
/*    queue(g_string_new("file:///media/roomserver/daten/Musik/VA-incoming/01_psyko_punkz_-_after_mf.mp3"));
    queue(g_string_new("file:///media/roomserver/daten/Musik/VA-incoming/02-the_raiders-a_feeling_(tuneboy_remix).mp3"));*/
    g_print("version %s\n", gst_version_string());

    mloop = g_main_loop_new (NULL, false);
    g_main_loop_run (mloop);

    /* Clean */
    clear();
    g_ptr_array_free (items, true);
    g_io_channel_unref(gio_read);
    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));

    print("exit");

    return 0;
}
