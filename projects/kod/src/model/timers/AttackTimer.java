/* $Id: AttackTimer.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * AttackTimer.java
 * 
 * Hit after a while, when pressing button.
 */

package src.model.timers;

import src.model.Character;
import src.model.CharacterSpot;
import src.model.Playfield;
import src.utils.TimerCallbackObject;

public class AttackTimer implements TimerCallbackObject
{
	private Character attacker;
	private CharacterSpot cs;

    public AttackTimer(Character attacker, CharacterSpot cs)
    {
		this.attacker = attacker;
		this.cs = cs;
    }

	public void ready()
    {
		Playfield pf = Playfield.getInstance();

		synchronized (pf) {
	        if (cs != null && cs.getCharacter() != null) {
			    Character c = cs.getCharacter();
			    Character attacked = attacker.attack(c);

			    pf.checkAllDead();

			    pf.getGame().attackFinished(attacker, attacked, attacked.isDead());
			} else {
				pf.getGame().attackFinished(attacker, null, false);
			}

	        attacker.setAttacking(false);
		}
    }

    public void error(Exception e)
    {
    	System.err.println("Problem in AttackTimer: " + e.getMessage());
    }
}