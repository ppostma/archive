/* $Id: KissTimer.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * KissTimer.java
 * 
 * Kiss Timer.
 */

package src.model.timers;

import src.model.Character;
import src.model.Playfield;
import src.model.TeamCharacter;
import src.utils.TimerCallbackObject;

public class KissTimer implements TimerCallbackObject
{
    private Character kisser;
    private Character kissed;

    public KissTimer(Character kisser, Character kissed)
    {
        this.kisser = kisser;
        this.kissed = kissed;
    }

    public void ready()
    {
    	Playfield pf = Playfield.getInstance();

    	synchronized (pf) {
	        /* Do the kiss. */
	        boolean monsterKissed = ((TeamCharacter)kisser).kiss(kissed);

	        boolean kisserFrog, kissedFrog;
	        kisserFrog = ((TeamCharacter)kisser).isFrog();
		    if (kissed instanceof TeamCharacter)
		    	kissedFrog = ((TeamCharacter)kissed).isFrog();
		    else
		    	kissedFrog = false;

		    /* Notify the controller. */
	        pf.getGame().kissFinished(kisser, kissed, kisserFrog, kissedFrog);
	
	        /* Call endGame if the monster was kissed. */
	        if (monsterKissed && !kisser.isDead())
	            pf.endGame(kisser);
	
	        /* Set kissing to false. */
	        kisser.setKissing(false);
    	}
    }

    public void error(Exception e)
    {
    	System.err.println("Problem in KissTimer: " + e.getMessage());
    }
}
