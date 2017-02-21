//
// Created by panda on 2017/2/21.
//

#include <jni.h>
#include <libavcodec/avcodec.h>

JNIEXPORT jstring JNICALL
Java_cn_panfr_pandalive_live_LiveHelper_helloFromFFmpeg(JNIEnv *env, jclass type) {
    // TODO
    char info[10000] = {0};
    sprintf(info, "%s\n", avcodec_configuration());
    return (*env)->NewStringUTF(env, info);
}