package pm.monkey.xmp4j;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import pm.monkey.xmp4j.exceptions.DepackException;
import pm.monkey.xmp4j.exceptions.LoadException;
import pm.monkey.xmp4j.exceptions.UnsupportedFormatException;
import pm.monkey.xmp4j.utils.LibraryLoader;

public class PlaybackEngine {

    /* sample format flags */
    public static final int XMP_FORMAT_8BIT         = (1 << 0);             /* Mix to 8-bit instead of 16 */
    public static final int XMP_FORMAT_UNSIGNED     = (1 << 1);             /* Mix to unsigned samples */
    public static final int XMP_FORMAT_MONO         = (1 << 2);             /* Mix to mono instead of stereo */

    /* mixer paramters for xmp_set_player() */
    public static final int XMP_PLAYER_AMP          = 0;                    /* Amplification factor */
    public static final int XMP_PLAYER_MIX          = 1;                    /* Stereo mixing */
    public static final int XMP_PLAYER_INTERP       = 2;                    /* Interpolation type */
    public static final int XMP_PLAYER_DSP          = 3;                    /* DSP effect flags */
    public static final int XMP_PLAYER_FLAGS        = 4;                    /* Player flags */
    public static final int XMP_PLAYER_CFLAGS       = 5;                    /* Player flags for current module */
    public static final int XMP_PLAYER_SMPCTL       = 6;                    /* Sample control flags */
    public static final int XMP_PLAYER_VOLUME       = 7;                    /* Player module volume */
    public static final int XMP_PLAYER_STATE        = 8;                    /* Internal player state */
    public static final int XMP_PLAYER_SFX_VOLUME   = 9;                    /* Player SFX volume */

    /* interpolation types */
    public static final int XMP_INTERP_NEAREST      = 0;                    /* Nearest neighbor */
    public static final int XMP_INTERP_LINEAR       = 1;                    /* Linear (default) */
    public static final int XMP_INTERP_SPLINE       = 2;                    /* Cubic spline */

    /* dsp effect types */
    public static final int XMP_DSP_LOWPASS         = (1 << 0);             /* Lowpass filter effect */
    public static final int XMP_DSP_ALL             = XMP_DSP_LOWPASS;

    /* player state */
    public static final int XMP_STATE_UNLOADED      = 0;                    /* Context created */
    public static final int XMP_STATE_LOADED        = 1;                    /* Module loaded */
    public static final int XMP_STATE_PLAYING       = 2;                    /* Module playing */

    /* player flags */
    public static final int XMP_FLAGS_VBLANK        = (1 << 0);             /* Use vblank timing */
    public static final int XMP_FLAGS_FX9BUG        = (1 << 1);             /* Emulate FX9 bug */
    public static final int XMP_FLAGS_FIXLOOP       = (1 << 2);             /* Emulate sample loop bug */

    /* sample flags */
    public static final int XMP_SMPCTL_SKIP         = (1 << 0);             /* Don't load samples */

    /* limits */
    public static final int XMP_MAX_KEYS            = 121;                  /* Number of valid keys */
    public static final int XMP_MAX_ENV_POINTS      = 32;                   /* Max number of envelope points */
    public static final int XMP_MAX_MOD_LENGTH      = 256;                  /* Max number of patterns in module */
    public static final int XMP_MAX_CHANNELS        = 64;                   /* Max number of channels in module */
    public static final int XMP_MAX_SRATE           = 49170;                /* max sampling rate (Hz) */
    public static final int XMP_MIN_SRATE           = 4000;                 /* min sampling rate (Hz) */
    public static final int XMP_MIN_BPM             = 20;                   /* min BPM */

    /* frame rate = (50 * bpm / 125) Hz
       frame size = (sampling rate * channels * size) / frame rate */
    public static final int XMP_MAX_FRAMESIZE       = (5 * XMP_MAX_SRATE * 2 / XMP_MIN_BPM);

    /* error codes */
    public static final int XMP_END                 = 1;
    public static final int XMP_ERROR_INTERNAL      = 2;                    /* Internal error */
    public static final int XMP_ERROR_FORMAT        = 3;                    /* Unsupported module format */
    public static final int XMP_ERROR_LOAD          = 4;                    /* Error loading file */
    public static final int XMP_ERROR_DEPACK        = 5;                    /* Error depacking file */
    public static final int XMP_ERROR_SYSTEM        = 6;                    /* System error */
    public static final int XMP_ERROR_INVALID       = 7;                    /* Invalid parameter */
    public static final int XMP_ERROR_STATE         = 8;                    /* Invalid player state */

    private native synchronized void init();
    private native synchronized void deinit();
    public native synchronized void loadModule(String name)
            throws UnsupportedFormatException,
                   LoadException,
                   DepackException;
    public native synchronized void loadModuleFromMemory(byte[] buffer)
            throws UnsupportedFormatException,
                   LoadException,
                   DepackException;
    public native synchronized int releaseModule();
    public native synchronized void startPlayer(int start, int rate, int flags);
    public native synchronized int endPlayer();
    public native synchronized int playFrame();
    public native synchronized int getBuffer(short buffer[]);
    public native synchronized int nextPosition();
    public native synchronized int prevPosition();
    public native synchronized int setPosition(int n);
    public native synchronized int stopModule();
    public native synchronized int restartModule();
    public native synchronized int seek(int time);
    public native synchronized int time();
    public native synchronized int mute(int chn, int status);
    public native synchronized void getInfo(int[] values);
    public native synchronized void setPlayer(int parm, int val);
    public native synchronized int getLoopCount();
    public native synchronized void getModVars(int[] vars);
    public native synchronized static String getVersion();
    public native synchronized String getModName();
    public native synchronized String getModType();
    public native synchronized String[] getFormats();
    public native synchronized String[] getInstruments();
    public native synchronized void getChannelData(int[] volumes, int[] finalvols, int[] pans, int[] instruments, int[] keys, int[] periods);
    public native synchronized void getPatternRow(int pat, int row, byte[] rowNotes, byte[] rowInstruments);
    public native synchronized void getSampleData(boolean trigger, int ins, int key, int period, int chn, int width, byte[] buffer);

    public void loadModule(InputStream stream)
            throws IOException,
                   UnsupportedFormatException,
                   LoadException,
                   DepackException {

        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        int nRead;
        byte[] data = new byte[16384];

        while ((nRead = stream.read(data, 0, data.length)) != -1) {
            buffer.write(data, 0, nRead);
        }

        buffer.flush();

        byte[] bytes = buffer.toByteArray();
        buffer.close();

        loadModuleFromMemory(bytes);
    }

    public void unmuteAllChannels() {
        for (int i = 0; i < XMP_MAX_CHANNELS; i++) {
            mute(i, 0);
        }
    }

    static {
        LibraryLoader.loadLibrary("xmp4j");
    }

    protected PlaybackEngine() {
        init();
    }

    public void dispose() {
        deinit();
    }

    private static PlaybackEngine instance = new PlaybackEngine();

    public static PlaybackEngine getInstance() {
        return instance;
    }
}