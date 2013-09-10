package pm.monkey.xmp4j.android;

import pm.monkey.xmp4j.AudioSink;
import android.media.AudioTrack;

public class AudioTrackSink extends AudioTrack implements AudioSink {

    private int bufferSize;

    public AudioTrackSink(int streamType, int sampleRateInHz,
            int channelConfig, int audioFormat, int bufferSizeInBytes,
            int mode, int sessionId) throws IllegalArgumentException {
        super(streamType, sampleRateInHz, channelConfig, audioFormat,
                bufferSizeInBytes, mode, sessionId);

        bufferSize = bufferSizeInBytes;
    }

    public AudioTrackSink(int streamType, int sampleRateInHz,
            int channelConfig, int audioFormat, int bufferSizeInBytes, int mode)
            throws IllegalArgumentException {
        super(streamType, sampleRateInHz, channelConfig, audioFormat,
                bufferSizeInBytes, mode);

        bufferSize = bufferSizeInBytes;
    }

    public int getBufferSize() {
        return bufferSize;
    }

}
