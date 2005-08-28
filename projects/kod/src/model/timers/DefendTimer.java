/* $Id: DefendTimer.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * DefendTimer.java
 * 
 * Defend for a while.
 */

package src.model.timers;

import src.model.Character;
import src.model.CharacterSpot;
import src.model.Playfield;
import src.utils.Constants;
import src.utils.TimerCallbackObject;

public class DefendTimer implements TimerCallbackObject
{
	private Character defender;
	private CharacterSpot cs;

	public DefendTimer(CharacterSpot cs, Character defender)
	{
		this.defender = defender;
		this.cs = cs;
	}

    public void ready()
    {
    	Playfield pf = Playfield.getInstance();

    	synchronized (pf) {
	    	pf.getGame().defendStarting(defender);

	    	defender.defend(cs);
    	}

    	try {
            Thread.sleep(Constants.DEFENDING_TIME);
        } catch (InterruptedException e) {
            this.error(e);
        }

        synchronized (pf) {
			defender.setDefending(false);
			defender.defend(null);

			pf.getGame().defendFinished(defender);
    	}
    }

    public void error(Exception e)
    {
    	System.err.println("Problem in DefendTimer: " + e.getMessage());
    }
}
