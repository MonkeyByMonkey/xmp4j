package pm.monkey.xmp4j;

import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import pm.monkey.xmp4j.exceptions.DepackException;
import pm.monkey.xmp4j.exceptions.LoadException;
import pm.monkey.xmp4j.exceptions.UnsupportedFormatException;

public class Player {

    private AudioSink sink;

    private PlaybackEngine engine;

    private ExecutorService service;

    private Future<?> playback;

    private volatile boolean isPaused;
    private ReentrantLock pauseLock;
    private Condition unpaused;

    public Player(AudioSink sink) {
        this.sink = sink;
        engine = PlaybackEngine.getInstance();
        service = Executors.newSingleThreadExecutor();

        playback = null;

        isPaused = false;
        pauseLock = new ReentrantLock();
        unpaused = pauseLock.newCondition();
    }

    private <T> void play(final T loadable, final Loader<T> loader) {
        stop();

        playback = service.submit(new Callable<Void>() {

            public Void call()
                    throws UnsupportedFormatException,
                           LoadException,
                           DepackException,
                           IOException {

                isPaused = false;

                loader.load(loadable);
                engine.unmuteAllChannels();

                sink.flush();
                engine.startPlayer(0, sink.getSampleRate(), 0);

                engine.setPlayer(PlaybackEngine.XMP_PLAYER_INTERP, PlaybackEngine.XMP_INTERP_SPLINE);
                engine.setPlayer(PlaybackEngine.XMP_PLAYER_DSP, 0);
                engine.setPlayer(PlaybackEngine.XMP_PLAYER_MIX, 80);

                short[] buffer = new short[sink.getBufferSize()];

                Thread thread = Thread.currentThread();

                while (!thread.isInterrupted() && engine.playFrame() == 0) {
                    int size = engine.getBuffer(buffer);
                    sink.write(buffer, 0, size);

                    pauseLock.lock();
                    try {
                        while (isPaused) {
                            sink.pause();
                            unpaused.await();
                        }

                        sink.play();
                    } catch (InterruptedException e) {
                        thread.interrupt();
                    } finally {
                        pauseLock.unlock();
                    }
                }

                isPaused = false;
                engine.stopModule();
                engine.endPlayer();
                engine.releaseModule();
                sink.stop();

                return null;
            }
        });
    }

    public synchronized void play(final String fileName) {
        play(fileName, new Loader<String>() {

            public void load(String loadable)
                    throws UnsupportedFormatException,
                           LoadException,
                           DepackException {

                engine.loadModule(fileName);
            }

        });

    }

    public synchronized void play(final InputStream stream) {
        play(stream, new Loader<InputStream>() {

            public void load(InputStream loadable)
                    throws UnsupportedFormatException,
                           LoadException,
                           DepackException,
                           IOException {

                engine.loadModule(stream);
            }

        });

    }

    public void pause() {
        pauseLock.lock();
        try {
            isPaused = true;
        } finally {
            pauseLock.unlock();
        }
    }


    public void resume() {
        pauseLock.lock();
        try {
            isPaused = false;
            unpaused.signalAll();
        } finally {
            pauseLock.unlock();
        }
    }

    public synchronized void stop() {
        if (playback != null) {
            playback.cancel(true);
        }
    }

    public synchronized void dispose() {
        service.shutdownNow();

        if (engine != null) {
            engine.dispose();
            engine = null;
        }
    }

    private interface Loader<T> {
        abstract void load(T loadable)
                throws LoadException,
                       DepackException,
                       UnsupportedFormatException,
                       IOException;
    }

}
