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

// 전역 변수
static GMainLoop *rtsp_loop = nullptr;
static GstRTSPServer *server = nullptr;

// --- 프레임 푸시 콜백 ---
static void start_feed(GstElement *appsrc, guint, gpointer) {
    static GstClockTime timestamp = 0;
    cv::Mat frame;

    {
        std::lock_guard<std::mutex> lock(rtspFrameMutex);
        if (rtspFrame.empty()) return;
        rtspFrame.copyTo(frame);
    }

    // 해상도 및 색공간 변환
    cv::resize(frame, frame, cv::Size(320, 240));
    cv::cvtColor(frame, frame, cv::COLOR_BGR2YUV_I420);

    // 안전한 크기 계산
    int size = frame.step[0] * frame.rows;
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, size, NULL);
    gst_buffer_fill(buffer, 0, frame.data, size);

    // 타임스탬프 설정
    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);
    timestamp += GST_BUFFER_DURATION(buffer);

    // 버퍼 푸시
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);
}

// --- 클라이언트 연결 해제 시 ---
static void media_unprepared(GstRTSPMedia *media, gpointer) {
    writeLog("[RTSP] Client disconnected");

    // appsrc 핸들러 해제 (혹시 중복 등록 방지)
    GstElement *element = gst_rtsp_media_get_element(media);
    if (element) {
        GstElement *appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");
        if (appsrc) {
            g_signal_handlers_disconnect_by_func(appsrc, (gpointer)start_feed, NULL);
            gst_object_unref(appsrc);
        }
    }
}

// --- 클라이언트 연결 시 ---
static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer) {
    writeLog("[RTSP] Client connected");

    GstElement *element = gst_rtsp_media_get_element(media);
    GstElement *appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");

    // need-data 시그널 등록
    g_signal_connect(appsrc, "need-data", G_CALLBACK(start_feed), NULL);

    // 연결 종료 감지
    g_signal_connect(media, "unprepared", G_CALLBACK(media_unprepared), NULL);

    gst_object_unref(appsrc);
}

// --- RTSP 서버 시작 ---
void startRtsp() {
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8554");

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    // 공유 모드: 하나의 파이프라인 유지 → 재접속 시 문제 없음
    gst_rtsp_media_factory_set_shared(factory, TRUE);

    // 파이프라인 정의
    gst_rtsp_media_factory_set_launch(factory,
        "( appsrc name=mysrc is-live=true block=true format=time "
        "caps=video/x-raw,format=I420,width=320,height=240,framerate=30/1 "
        "! x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast key-int-max=30 "
        "! rtph264pay name=pay0 pt=96 )");

    gst_rtsp_mount_points_add_factory(mounts, "/stream", factory);
    g_object_unref(mounts);

    // 클라이언트 연결 콜백
    g_signal_connect(factory, "media-configure", G_CALLBACK(media_configure), NULL);

    gst_rtsp_server_attach(server, NULL);
    writeLog("RTSP server started at rtsp://<IP>:8554/stream");

    rtsp_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(rtsp_loop);
}

// --- RTSP 서버 정지 ---
void stopRtsp() {
    if (rtsp_loop) {
        g_main_loop_quit(rtsp_loop);
        g_main_loop_unref(rtsp_loop);
        rtsp_loop = nullptr;
    }

    if (server) {
        g_object_unref(server);
        server = nullptr;
    }
}