[![Build Status](https://monkeybymonkey.ci.cloudbees.com/buildStatus/icon?job=xmp4j)](https://monkeybymonkey.ci.cloudbees.com/job/xmp4j/)

# xmp4j

Java wrapper for libxmp, largely based on http://sourceforge.net/p/xmp/xmp-android by 
Claudio Matsuoka et. al.

libxmp is copyright Claudio Matsuoka et. al. See jni/libxmp/docs/CREDITS for a 
complete list of authors.

## Building

`mvn clean package`. Requires `$ANDROID_NDK_HOME`, `$JDK_WIN` and `$JDK_MAC`
(Mac only) to be set. The `$JDK_*` variables should point to respective 
JDK location, e.g. `$JAVA_HOME`.

## Usage (Android)

    AudioSink sink = new AudioTrackSink(
        AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_OUT_STEREO, 
        AudioFormat.ENCODING_PCM_16BIT, 
        AudioTrack.getMinBufferSize(
            44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT
        ), AudioTrack.MODE_STREAM);
        
    player = new Player(sink);

    InputStream stream = getResources().openRawResource(R.raw.dance);
    player.play(stream); // calls stream.close() after reading

    player.pause(); // pauses playback
    player.resume(); // resumes playback
    player.stop(); // stops playback
    player.dispose(); // disposes native resources

Using the player object after `dispose()` has been called will produce 
unspecified behaviour.

## AudioSink

Implement the AudioSink interface to provide a custom output. AudioTrackSink 
provides an implementation based on Android's AudioTrack.


## LICENSE

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version. This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
