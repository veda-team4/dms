#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <cstring>
#include "global.h"
#include "utils.h"

// === AES 대칭키 ===
static const unsigned char key[33] = "abcdefghijklmnopqrstuvwxyz012345";

static void start_feed(GstElement *appsrc, guint, gpointer) {
    static GstClockTime timestamp = 0;
    cv::Mat frame;
    {
      std::lock_guard<std::mutex> lock(rtspFrameMutex);
      if (rtspFrame.empty()) return;
      rtspFrame.copyTo(frame);
    }

    cv::resize(frame, frame, cv::Size(320, 240));
    cv::cvtColor(frame, frame, cv::COLOR_BGR2YUV_I420);

    int size = frame.total() * frame.elemSize();
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, size, NULL);
    gst_buffer_fill(buffer, 0, frame.data, size);

    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);
    timestamp += GST_BUFFER_DURATION(buffer);

    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);
}
static void media_unprepared(GstRTSPMedia *, gpointer) {
    writeLog("[RTSP] Client disconnected");
}
static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer) {
    GstElement *element = gst_rtsp_media_get_element(media);
    GstElement *appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");
    g_signal_connect(appsrc, "need-data", G_CALLBACK(start_feed), NULL);
    g_signal_connect(media, "unprepared", G_CALLBACK(media_unprepared), NULL);
    gst_object_unref(appsrc);
}

static GMainLoop *rtsp_loop = nullptr;
void startRtsp() {
  // int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  // sockaddr_in addr{};
  // addr.sin_family = AF_INET;
  // addr.sin_port = htons(9000);
  // addr.sin_addr.s_addr = INADDR_ANY;

  // bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
  // listen(server_fd, 1);

  GstRTSPServer *server = gst_rtsp_server_new();
  gst_rtsp_server_set_service(server, "8554");

  GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
  GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_shared(factory, FALSE);

  gst_rtsp_media_factory_set_launch(factory,
      "( appsrc name=mysrc is-live=true block=true format=time "
      "caps=video/x-raw,format=I420,width=320,height=240,framerate=30/1 "
      "! x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast "
      "! rtph264pay name=pay0 pt=96 )");

  gst_rtsp_mount_points_add_factory(mounts, "/stream", factory);
  g_object_unref(mounts);

  g_signal_connect(factory, "media-configure", G_CALLBACK(media_configure), NULL);

  gst_rtsp_server_attach(server, NULL);
  writeLog("RTSP server started at rtsp://<IP>:8554/stream");

  rtsp_loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(rtsp_loop);
}

void stopRtsp() {
  if (rtsp_loop) {
    g_main_loop_quit(rtsp_loop);
    g_main_loop_unref(rtsp_loop);
    rtsp_loop = nullptr;
  }
}