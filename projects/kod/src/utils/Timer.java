/* $Id: Timer.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * Timer.java
 * 
 * Simple timer using the Runnable interface.
 */

package src.utils;

public class Timer implements Runnable
{
    private Thread timerThread;
    private TimerCallbackObject co;
    private int duration;
    private int count;
    private boolean running;

    /**
     * Constructor.
     * 
     * @param duration In milliseconds
     * @param count Execute this timer how many times? 0 for endless execution
     * @param co TimerCallbackObject
     */
    public Timer(int duration, int count, TimerCallbackObject co)
    {
        this.timerThread = new Thread(this);
        this.timerThread.setDaemon(true);
        this.duration = duration;
        this.count = count;
        this.co = co;
        this.running = false;
    }

    /**
     * Start the timer. 
     */
    public void start()
    {
        timerThread.start();
    }

    /**
     * Stop the timer.
     */
    public void stop()
    {
        running = false;
    }

    /**
     * Function that will be executed after a 'start'.
     */
    public void run()
    {
        int done = count;
        running = true;

        do {
            try {
                Thread.sleep(duration);
            } catch (InterruptedException ie) {
                co.error(ie);
            }
            if (running)
                co.ready();
            if (count != 0 && --done < 1)
                running = false;
        } while (running);
    }
}
