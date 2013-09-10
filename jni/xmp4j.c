/* Simple interface adaptor for jni */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <jni.h>
#include "xmp.h"

static xmp_context ctx = NULL;
static struct xmp_module_info mi;
static struct xmp_frame_info fi;

static int _playing = 0;
static int _cur_vol[XMP_MAX_CHANNELS];
static int _hold_vol[XMP_MAX_CHANNELS];
static int _pan[XMP_MAX_CHANNELS];
static int _ins[XMP_MAX_CHANNELS];
static int _key[XMP_MAX_CHANNELS];
static int _period[XMP_MAX_CHANNELS];
static int _finalvol[XMP_MAX_CHANNELS];
static int _last_key[XMP_MAX_CHANNELS];
static int _pos[XMP_MAX_CHANNELS];
static int _decay = 4;

#define MAX_BUFFER_SIZE 256
static char _buffer[MAX_BUFFER_SIZE];

const char *exceptionFromError(int error)
{

    switch (error) {
        case -XMP_ERROR_INTERNAL:
            return "java/lang/RuntimeException";
        case -XMP_ERROR_FORMAT:
            return "pm/monkey/xmp4j/exceptions/UnsupportedFormat";
        case -XMP_ERROR_LOAD:
            return "pm/monkey/xmp4j/exceptions/LoadException";
        case -XMP_ERROR_DEPACK:
            return "pm/monkey/xmp4j/exceptions/DepackException";
        case -XMP_ERROR_SYSTEM:
            return "pm/monkey/xmp4j/exceptions/SystemException";
        case -XMP_ERROR_INVALID:
            return "java/lang/IllegalArgumentException";
        case -XMP_ERROR_STATE:
            return "java/lang/IllegalStateException";
        default:
            return "java/lang/NoClassDefFoundError";
    }
}

jint throwException(JNIEnv *env, const char *class, const char *message)
{
    jclass exClass;

    exClass = (*env)->FindClass(env, class);

    if (exClass == NULL) {
        return throwException(env, "java/lang/NoClassDefFoundError", NULL);
    } else {
        return (*env)->ThrowNew(env, exClass, message);
    }
}

jint throwSystemException(JNIEnv *env, int error)
{
    return throwException(env, exceptionFromError(-XMP_ERROR_SYSTEM), strerror(error));
}

/* For ModList */
JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_init(JNIEnv *env, jobject obj)
{
    if (ctx != NULL)
        return;

    ctx = xmp_create_context();
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_deinit(JNIEnv *env, jobject obj)
{
    if (ctx != NULL) {
        xmp_free_context(ctx);
        ctx = NULL;
    }
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_loadModule(JNIEnv *env, jobject obj, jstring name)
{
    const char *filename;
    int res, err;

    filename = (*env)->GetStringUTFChars(env, name, NULL);
    res = xmp_load_module(ctx, (char *)filename);
    err = errno;
    (*env)->ReleaseStringUTFChars(env, name, filename);

    if (res == 0) {
        xmp_get_module_info(ctx, &mi);
    } else if (res == -XMP_ERROR_SYSTEM) {
        throwSystemException(env, err);
    } else {
        throwException(env, exceptionFromError(res), NULL);
    }
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_loadModuleFromMemory(JNIEnv *env, jobject obj, jbyteArray buffer)
{
    int res, err;

    jbyte *bufferPtr = (jbyte *) (*env)->GetByteArrayElements(env, buffer, NULL);
    jsize bufferLength = (*env)->GetArrayLength(env, buffer);

    res = xmp_load_module_from_memory(ctx, (void*)bufferPtr, (long)bufferLength);
    (*env)->ReleaseByteArrayElements(env, buffer, bufferPtr, 0);

    if (res == 0) {
        xmp_get_module_info(ctx, &mi);
    } else if (res == -XMP_ERROR_SYSTEM) {
        throwSystemException(env, err);
    } else {
        throwException(env, exceptionFromError(res), NULL);
    }
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_releaseModule(JNIEnv *env, jobject obj)
{
    xmp_release_module(ctx);
    return 0;
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_startPlayer(JNIEnv *env, jobject obj, jint start, jint rate, jint flags)
{
    int i, res, err;

    for (i = 0; i < XMP_MAX_CHANNELS; i++) {
        _key[i] = -1;
        _last_key[i] = -1;
    }

    _playing = 1;
    res = xmp_start_player(ctx, rate, flags);
    err = errno;

    if (res == -XMP_ERROR_SYSTEM) {
        throwSystemException(env, err);
    } else if (res != 0) {
        throwException(env, exceptionFromError(res), NULL);
    }

}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_endPlayer(JNIEnv *env, jobject obj)
{
    _playing = 0;
    xmp_end_player(ctx);
    return 0;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_playFrame(JNIEnv *env, jobject obj)
{
    int i, ret;

    ret = xmp_play_frame(ctx);
    xmp_get_frame_info(ctx, &fi);

    return ret;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getBuffer(JNIEnv *env, jobject obj, jshortArray buffer)
{
    (*env)->SetShortArrayRegion(env, buffer, 0, fi.buffer_size, fi.buffer);
    return fi.buffer_size / 2;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_nextPosition(JNIEnv *env, jobject obj)
{
    return xmp_next_position(ctx);
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_prevPosition(JNIEnv *env, jobject obj)
{
    return xmp_prev_position(ctx);
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_setPosition(JNIEnv *env, jobject obj, jint n)
{
    return xmp_set_position(ctx, n);
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_stopModule(JNIEnv *env, jobject obj)
{
    xmp_stop_module(ctx);
    return 0;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_restartModule(JNIEnv *env, jobject obj)
{
    xmp_restart_module(ctx);
    return 0;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_seek(JNIEnv *env, jobject obj, jint time)
{
    return xmp_seek_time(ctx, time);
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_time(JNIEnv *env, jobject obj)
{
    return _playing ? fi.time : -1;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_mute(JNIEnv *env, jobject obj, jint chn, jint status)
{
    return xmp_channel_mute(ctx, chn, status);
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getInfo(JNIEnv *env, jobject obj, jintArray values)
{
    int v[7];

    v[0] = fi.pos;
    v[1] = fi.pattern;
    v[2] = fi.row;
    v[3] = fi.num_rows;
    v[4] = fi.frame;
    v[5] = fi.speed;
    v[6] = fi.bpm;

    (*env)->SetIntArrayRegion(env, values, 0, 7, v);
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_setPlayer(JNIEnv *env, jobject obj, jint parm, jint val)
{
    int res = xmp_set_player(ctx, parm, val);

    if (res != 0) {
        throwException(env, exceptionFromError(res), NULL);
    }
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getPlaySpeed(JNIEnv *env, jobject obj)
{
    return fi.speed;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getPlayBpm(JNIEnv *env, jobject obj)
{
    return fi.bpm;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getPlayPos(JNIEnv *env, jobject obj)
{
    return fi.pos;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getPlayPat(JNIEnv *env, jobject obj)
{
    return fi.pattern;
}

JNIEXPORT jint JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getLoopCount(JNIEnv *env, jobject obj)
{
    return fi.loop_count;
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getModVars(JNIEnv *env, jobject obj, jintArray vars)
{
    int v[6];

    v[0] = mi.seq_data[0].duration;
    v[1] = mi.mod->len;
    v[2] = mi.mod->pat;
    v[3] = mi.mod->chn;
    v[4] = mi.mod->ins;
    v[5] = mi.mod->smp;

    (*env)->SetIntArrayRegion(env, vars, 0, 6, v);
}

JNIEXPORT jstring JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getVersion(JNIEnv *env, jobject obj)
{
    char buf[20];
    snprintf(buf, 20, "%d.%d.%d",
        (xmp_vercode & 0x00ff0000) >> 16,
        (xmp_vercode & 0x0000ff00) >> 8,
        (xmp_vercode & 0x000000ff));

    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jobjectArray JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getFormats(JNIEnv *env, jobject obj)
{
    jstring s;
    jclass stringClass;
    jobjectArray stringArray;
    int i, num;
    char **list;
    char buf[80];

    list = xmp_get_format_list();
    for (num = 0; list[num] != NULL; num++);

    stringClass = (*env)->FindClass(env,"java/lang/String");
    if (stringClass == NULL)
        return NULL;

    stringArray = (*env)->NewObjectArray(env, num, stringClass, NULL);
    if (stringArray == NULL)
        return NULL;

    for (i = 0; i < num; i++) {
        s = (*env)->NewStringUTF(env, list[i]);
        (*env)->SetObjectArrayElement(env, stringArray, i, s);
        (*env)->DeleteLocalRef(env, s);
    }

    return stringArray;
}

JNIEXPORT jstring JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getModName(JNIEnv *env, jobject obj)
{
    return (*env)->NewStringUTF(env, mi.mod->name);
}

JNIEXPORT jstring JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getModType(JNIEnv *env, jobject obj)
{
    return (*env)->NewStringUTF(env, mi.mod->type);
}

JNIEXPORT jobjectArray JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getInstruments(JNIEnv *env, jobject obj)
{
    jstring s;
    jclass stringClass;
    jobjectArray stringArray;
    int i;
    char buf[80];

    stringClass = (*env)->FindClass(env,"java/lang/String");
    if (stringClass == NULL)
        return NULL;

    stringArray = (*env)->NewObjectArray(env, mi.mod->ins, stringClass, NULL);
    if (stringArray == NULL)
        return NULL;

    for (i = 0; i < mi.mod->ins; i++) {
        snprintf(buf, 80, "%02X %s", i + 1, mi.mod->xxi[i].name);
        s = (*env)->NewStringUTF(env, buf);
        (*env)->SetObjectArrayElement(env, stringArray, i, s);
        (*env)->DeleteLocalRef(env, s);
    }

    return stringArray;
}

static struct xmp_subinstrument *get_subinstrument(int ins, int key)
{
    if (ins >= 0 && ins < mi.mod->ins) {
        if (mi.mod->xxi[ins].map[key].ins != 0xff) {
            int mapped = mi.mod->xxi[ins].map[key].ins;
            return &mi.mod->xxi[ins].sub[mapped];
        }
    }

    return NULL;
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getChannelData(JNIEnv *env, jobject obj, jintArray vol, jintArray finalvol, jintArray pan, jintArray ins, jintArray key, jintArray period)
{
    struct xmp_subinstrument *sub;
    int chn = mi.mod->chn;
    int i;

    for (i = 0; i < chn; i++) {
                struct xmp_channel_info *ci = &fi.channel_info[i];

        if (ci->event.vol > 0) {
            _hold_vol[i] = ci->event.vol * 0x40 / mi.vol_base;
        }

        _cur_vol[i] -= _decay;
        if (_cur_vol[i] < 0) {
            _cur_vol[i] = 0;
        }

        if (ci->event.note > 0 && ci->event.note <= 0x80) {
            _key[i] = ci->event.note - 1;
            _last_key[i] = _key[i];
            sub = get_subinstrument(ci->instrument, _key[i]);
            if (sub != NULL) {
                _cur_vol[i] = sub->vol * 0x40 / mi.vol_base;
            }
        } else {
            _key[i] = -1;
        }

        if (ci->event.vol > 0) {
            _key[i] = _last_key[i];
            _cur_vol[i] = ci->event.vol * 0x40 / mi.vol_base;
        }

        _ins[i] = (signed char)ci->instrument;
        _finalvol[i] = ci->volume;
        _pan[i] = ci->pan;
        _period[i] = ci->period >> 8;
    }

    (*env)->SetIntArrayRegion(env, vol, 0, chn, _cur_vol);
    (*env)->SetIntArrayRegion(env, finalvol, 0, chn, _finalvol);
    (*env)->SetIntArrayRegion(env, pan, 0, chn, _pan);
    (*env)->SetIntArrayRegion(env, ins, 0, chn, _ins);
    (*env)->SetIntArrayRegion(env, key, 0, chn, _key);
    (*env)->SetIntArrayRegion(env, period, 0, chn, _period);
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getPatternRow(JNIEnv *env, jobject obj, jint pat, jint row, jbyteArray rowNotes, jbyteArray rowInstruments)
{
    struct xmp_pattern *xxp;
    unsigned char row_note[XMP_MAX_CHANNELS];
    unsigned char row_ins[XMP_MAX_CHANNELS];
    int chn;
    int i;

    if (mi.mod == NULL || pat > mi.mod->pat || row > mi.mod->xxp[pat]->rows)
        return;

     xxp = mi.mod->xxp[pat];
    chn = mi.mod->chn;

    for (i = 0; i < chn; i++) {
        struct xmp_track *xxt = mi.mod->xxt[xxp->index[i]];
        struct xmp_event *e = &xxt->event[row];

        row_note[i] = e->note;
        row_ins[i] = e->ins;
    }

    (*env)->SetByteArrayRegion(env, rowNotes, 0, chn, row_note);
    (*env)->SetByteArrayRegion(env, rowInstruments, 0, chn, row_ins);
}

JNIEXPORT void JNICALL
Java_pm_monkey_xmp4j_PlaybackEngine_getSampleData(JNIEnv *env, jobject obj, jboolean trigger, jint ins, jint key, jint period, jint chn, jint width, jbyteArray buffer)
{
    struct xmp_subinstrument *sub;
    struct xmp_sample *xxs;
    int i, pos, transient_size;
    int limit;
    int step, len, lps, lpe;

    if (width > MAX_BUFFER_SIZE) {
        width = MAX_BUFFER_SIZE;
    }

    if (period == 0) {
        goto err;
    }

    if (ins < 0 || ins > mi.mod->ins || key > 0x80) {
        goto err;
    }

    sub = get_subinstrument(ins, key);
    if (sub == NULL || sub->sid < 0 || sub->sid >= mi.mod->smp) {
        goto err;
    }

    xxs = &mi.mod->xxs[sub->sid];
    if (xxs == NULL || xxs->flg & XMP_SAMPLE_SYNTH) {
        goto err;
    }

    pos = _pos[chn];

    /* In case of new keypress, reset sample */
    if (trigger == JNI_TRUE) {
        pos = 0;
    }

    step = (XMP_PERIOD_BASE << 5) / period;
    len = xxs->len << 5;
    lps = xxs->lps << 5;
    lpe = xxs->lpe << 5;

    /* Limit is the buffer size or the remaining transient size */
    if (step == 0) {
        transient_size = 0;
    } else if (xxs->flg & XMP_SAMPLE_LOOP) {
        transient_size = (lps - pos) / step;
    } else {
        transient_size = (len - pos) / step;
    }
    if (transient_size < 0) {
        transient_size = 0;
    }

    limit = width;
    if (limit > transient_size) {
        limit = transient_size;
    }

    if (xxs->flg & XMP_SAMPLE_16BIT) {
        /* transient */
        for (i = 0; i < limit; i++) {
            _buffer[i] = ((short *)&xxs->data)[pos >> 5] >> 8;
            pos += step;
        }

        /* loop */
        if (xxs->flg & XMP_SAMPLE_LOOP) {
            for (i = limit; i < width; i++) {
                _buffer[i] = ((short *)xxs->data)[pos >> 5] >> 8;
                pos += step;
                if (pos >= lpe)
                    pos = lps + pos - lpe;
                if (pos >= lpe)        /* avoid division */
                    pos = lps;
            }
        } else {
            for (i = limit; i < width; i++) {
                _buffer[i] = 0;
            }
        }
    } else {
        /* transient */
        for (i = 0; i < limit; i++) {
            _buffer[i] = xxs->data[pos >> 5];
            pos += step;
        }

        /* loop */
        if (xxs->flg & XMP_SAMPLE_LOOP) {
            for (i = limit; i < width; i++) {
                _buffer[i] = xxs->data[pos >> 5];
                pos += step;
                if (pos >= lpe)
                    pos = lps + pos - lpe;
                if (pos >= lpe)        /* avoid division */
                    pos = lps;
            }
        } else {
            for (i = limit; i < width; i++) {
                _buffer[i] = 0;
            }
        }
    }

    _pos[chn] = pos;

    (*env)->SetByteArrayRegion(env, buffer, 0, width, _buffer);
    return;

    err:
    memset(_buffer, 0, width);
    (*env)->SetByteArrayRegion(env, buffer, 0, width, _buffer);
}
