/*
 * test-tee-gst.c
 *
 *  Created on: Feb 11, 2021
 *      Author: sham
 */

#include <gst/gst.h>
#include <glib.h>

typedef struct _UserData {GMainLoop *loop; GstElement *element;} UserData;

static gboolean print_field (GQuark field, const GValue * value, gpointer pfx) {
  gchar *str = gst_value_serialize (value);

  g_print ("%s  %15s: %s\n", (gchar *) pfx, g_quark_to_string (field), str);
  g_free (str);
  return TRUE;
}


static void print_caps (const GstCaps * caps, const gchar * pfx) {
  guint i;

  g_return_if_fail (caps != NULL);

  if (gst_caps_is_any (caps)) {
    g_print ("%sANY\n", pfx);
    return;
  }
  if (gst_caps_is_empty (caps)) {
    g_print ("%sEMPTY\n", pfx);
    return;
  }

  for (i = 0; i < gst_caps_get_size (caps); i++) {
    GstStructure *structure = gst_caps_get_structure (caps, i);

    g_print ("%s%s\n", pfx, gst_structure_get_name (structure));
    gst_structure_foreach (structure, print_field, (gpointer) pfx);
  }
}


static void print_pad_capabilities (GstElement *element, gchar *pad_name) {
  GstPad *pad = NULL;
  GstCaps *caps = NULL;
  /* Retrieve pad */
  pad = gst_element_get_static_pad (element, pad_name);
  if (!pad) {
    g_printerr ("Could not retrieve pad '%s'\n", pad_name);
    return;
  }

  /* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
  caps = gst_pad_get_current_caps (pad);
  if (!caps)
    caps = gst_pad_query_caps (pad, NULL);

  /* Print and free */
  g_print ("Caps for the %s pad:\n", pad_name);
  print_caps (caps, "      ");
  gst_caps_unref (caps);
  gst_object_unref (pad);
}


static gboolean bus_call (GstBus     *bus,
          GstMessage *msg,
		  UserData    *data)
{
  GMainLoop *loop = (*data).loop;
  GstElement *element = (*data).element;

//  guint64 running_time, stream_time;
//
//  GstState old_state, new_state, pending_state;
//  print_pad_capabilities (element, "sink");
//  gst_message_parse_qos(msg, NULL, &running_time, &stream_time, NULL, NULL);
//  gint64 current = -1;
//  g_print("%" G_GUINT64_FORMAT "\n", msg->timestamp);
//  g_print("%" G_GUINT64_FORMAT "\n", running_time);
//  g_print("%" G_GUINT64_FORMAT "\n", stream_time);
//
//  if (!gst_element_query_position (element, GST_FORMAT_TIME, &current)) {
//	g_printerr ("Could not query current position.\n");
//  }
//
//  g_print ("Position %" GST_TIME_FORMAT "/ %d \n",
//              GST_TIME_ARGS (current), GST_MESSAGE_TYPE (msg));
  gint64 current = -1;



  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_ELEMENT:
    	if (!gst_element_query_position (element, GST_FORMAT_TIME, &current)) {
    		g_printerr ("Could not query current position.\n");
    	  }
    	print_pad_capabilities (element, "sink");
		g_print ("Position %" GST_TIME_FORMAT "/ %d \n",
    	              GST_TIME_ARGS (current), GST_MESSAGE_TYPE (msg));
    	break;
    case GST_MESSAGE_LATENCY:
    	break;
    case GST_MESSAGE_STREAM_START:
    	break;
    case GST_MESSAGE_DURATION_CHANGED:
    	break;
    case GST_MESSAGE_HAVE_CONTEXT:
    	break;
//	case GST_MESSAGE_STATE_CHANGED:
//	  /* We are only interested in state-changed messages from the pipeline */
//		gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
//		g_print ("\nPipeline state changed from %s to %s:\n",
//		gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
//		/* Print the current capabilities of the sink element */
//	  break;
    default:
      break;
  }

  return TRUE;
}

static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (decoder, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}



//static void
//on_pad_added (GstElement *element,
//              GstPad     *pad,
//              gpointer    data)
//{
//  GstPad *sinkpad;
//  GstElement *decoder = (GstElement *) data;
//
//  /* We can now link this pad with the vorbis-decoder sink pad */
//  g_print ("Dynamic pad created, linking demuxer/decoder\n");
//
//  sinkpad = gst_element_get_static_pad (decoder, "sink");
//
//  gst_pad_link (pad, sinkpad);
//
//  gst_object_unref (sinkpad);
//}
/* Shows the CURRENT capabilities of the requested pad in the given element */

int main(int   argc, char *argv[]) {

	GstElement *pipeline, *filesrc, *flvdemux, *h264parse, *videorate, *capsfilter, *nvh264dec, *cudadownload, *jpegenc, *multifilesink;
	GstStateChangeReturn ret;
	GstCaps *filtercaps;
	gst_init(&argc, &argv);

	pipeline = gst_pipeline_new ("audio-player");
	filesrc   = gst_element_factory_make ("filesrc",       "src");
	flvdemux   = gst_element_factory_make ("flvdemux",       "demux");
	h264parse   = gst_element_factory_make ("h264parse",       "h264parse");
	nvh264dec   = gst_element_factory_make ("nvh264dec",       "nvh264dec");
	cudadownload   = gst_element_factory_make ("cudadownload",       "cudadownload");
	videorate   = gst_element_factory_make ("videorate",       "videorate");
	capsfilter   = gst_element_factory_make ("capsfilter",       "capsfilter");
	jpegenc   = gst_element_factory_make ("jpegenc",       "jpegenc");
	multifilesink  = gst_element_factory_make ("multifilesink",      "multifilesink");

	if (!pipeline || !filesrc || !multifilesink) {
	    g_printerr ("One element could not be created. Exiting.\n");
	    return -1;
	  }


	filtercaps = gst_caps_new_simple ("video/x-raw",
            "framerate", GST_TYPE_FRACTION, 1, 5,
            NULL);

	g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
	g_object_set (G_OBJECT (multifilesink), "location", "aaa5_%d.jpg", NULL);
	g_object_set (G_OBJECT (capsfilter), "caps", filtercaps, NULL);
	g_object_set (G_OBJECT (multifilesink), "post-messages", TRUE, NULL);

	GstBus *bus;
	GMainLoop *loop;

	loop = g_main_loop_new (NULL, FALSE);


	gst_bin_add_many (GST_BIN (pipeline),
			filesrc, flvdemux, h264parse, nvh264dec, cudadownload, videorate, capsfilter, jpegenc, multifilesink, NULL);


	if (gst_element_link (filesrc, flvdemux) != TRUE) {
		g_printerr ("Elements could not be linked1.\n");
		gst_object_unref (pipeline);
		return -1;
	}

	if (gst_element_link_many (h264parse, nvh264dec, cudadownload, videorate, capsfilter, jpegenc, multifilesink, NULL) != TRUE) {
		g_printerr ("Elements could not be linked2.\n");
		gst_object_unref (pipeline);
		return -1;
	}

	g_signal_connect(flvdemux, "pad-added", G_CALLBACK (on_pad_added), h264parse);

	g_print ("In NULL state:\n");
//	print_pad_capabilities (flvdemux, "video");
//	gst_element_link (filesrc, "pad-added", G_CALLBACK (on_pad_added), filesink);
//	gst_element_link (filesrc, filesink);



	g_print ("Now playing: %s\n", argv[1]);
	ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr ("Unable to set the pipeline to the playing state (check the bus for error messages).\n");
	}

	g_print ("Running...\n");
//	g_main_loop_run (loop);
	UserData user_data = {loop, jpegenc};
	/* we add a message handler */
	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	gst_bus_add_signal_watch (bus);
	g_signal_connect (bus, "message", G_CALLBACK (bus_call), &user_data);
//	bus_watch_id = gst_bus_add_watch (bus, bus_call, user_data);
	g_main_loop_run (loop);
//	GstMessage *msg;
//	gboolean terminate = FALSE;
//	bus = gst_element_get_bus (pipeline);
//	bus_watch_id = gst_bus_add_watch (bus, bus_call, NULL);
//	  do {
//	    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ANY);
//
//	    /* Parse message */
//	    if (msg != NULL) {
//	      GError *err;
//	      gchar *debug_info;
//
//	      switch (GST_MESSAGE_TYPE (msg)) {
//	        case GST_MESSAGE_ERROR:
//	          gst_message_parse_error (msg, &err, &debug_info);
//	          g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
//	          g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
//	          g_clear_error (&err);
//	          g_free (debug_info);
//	          terminate = TRUE;
//	          break;
//	        case GST_MESSAGE_EOS:
//	          g_print ("End-Of-Stream reached.\n");
//	          terminate = TRUE;
//	          break;
//	        case GST_MESSAGE_STATE_CHANGED:
//	          /* We are only interested in state-changed messages from the pipeline */
//	          if (GST_MESSAGE_SRC (msg) == GST_OBJECT (pipeline)) {
//	            GstState old_state, new_state, pending_state;
//	            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
//	            g_print ("\nPipeline state changed from %s to %s:\n",
//				gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
//	            /* Print the current capabilities of the sink element */
//	            print_pad_capabilities (flvdemux, "video");
//	          }
//	          break;
//	        default:
//	          /* We should not reach here because we only asked for ERRORs, EOS and STATE_CHANGED */
//	          g_printerr ("Unexpected message received.\n");
//	          break;
//	      }
//	      gst_message_unref (msg);
//	    }
//	  } while (!terminate);
//	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	g_print ("Running...\n");


	g_print ("Returned, stopping playback\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);

	g_print ("Deleting pipeline\n");
	gst_object_unref (GST_OBJECT (pipeline));
	g_main_loop_unref (loop);

	return 0;
}


