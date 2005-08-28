/* $Id: PlaySound.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * PlaySound.java
 * 
 * Plays a sound, really.
 */

package src.utils;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;

public class PlaySound
{
    public PlaySound(String file)
    {
        AudioInputStream ain = null;
        Clip clip;
        URL base;

        try {
            base = new URL("file:" + System.getProperty("user.dir") + "/" + Constants.DIRECTORY_SOUNDS_BASE + file);
        } catch (MalformedURLException e) {
            System.err.println(e.getMessage());
            return;
        }

        try {
            ain = AudioSystem.getAudioInputStream(base);
            DataLine.Info info = new DataLine.Info(Clip.class, ain.getFormat());
            clip = (Clip)AudioSystem.getLine(info);
            clip.open(ain);
        } catch (Exception e) {
            System.err.println(e.getMessage());
            return;
        } finally {
            try {
                ain.close();
            } catch (IOException e1) {
                System.err.println(e1.getMessage());
            }
        }

        clip.start();
    }
}
