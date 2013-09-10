package pm.monkey.xmp4j;

public interface AudioSink {

    public int getSampleRate();

    public int getBufferSize();

    public int write(byte[] audioData, int offsetInBytes, int sizeInBytes);

    public int write (short[] audioData, int offsetInShorts, int sizeInShorts);

    public void flush();

    public void play();

    public void pause();

    public void stop();

    public void release();

}
