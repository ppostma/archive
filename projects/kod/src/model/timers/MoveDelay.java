/* $Id: MoveDelay.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * MoveDelay.java
 * 
 * Adds the move delay.
 */

package src.model.timers;

import src.model.Character;
import src.utils.TimerCallbackObject;

public class MoveDelay implements TimerCallbackObject
{
	private Character ch;

	public MoveDelay(Character ch)
    {
    	this.ch = ch;
    }

	public void ready()
	{
		ch.timerSetMovable();
	}

	public void error(Exception e)
	{
		System.err.println("Problem in MoveDelay timer: " + e.getMessage());
	}
}
