/* $Id: StartGame.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * StartGame.java
 * 
 * Starts the game.
 */

package src.model.timers;

import src.model.Playfield;
import src.utils.Constants;
import src.utils.TimerCallbackObject;

public class StartGame implements TimerCallbackObject
{
	private int counts;

	public StartGame()
	{
		this.counts = 0;
	}
	
	public void ready()
	{
    	Playfield pf = Playfield.getInstance();

    	synchronized (pf) {
    		counts++;
			if (counts >= Constants.GAME_DELAY)
				pf.startGame();
			else
				pf.getGame().countDown(Constants.GAME_DELAY - counts);
    	}
	}

	public void error(Exception e)
	{
    	System.err.println("Problem in StartGame timer: " + e.getMessage());
	}
}
