/* $Id: EnergyRegeneration.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * EnergyRegeneration.java
 * 
 * Energy regeneration for a character.
 */

package src.model.timers;

import src.model.Character;
import src.model.Playfield;
import src.utils.Constants;
import src.utils.TimerCallbackObject;

public class EnergyRegeneration implements TimerCallbackObject
{
    private Character character;

    public EnergyRegeneration(Character ch)
    {
        this.character = ch;
    }

    public void ready()
    {
    	Playfield pf = Playfield.getInstance();

        if (character != null) {
        	synchronized (pf) {
        		character.setEnergy(character.getEnergy() + Constants.ENERGY_MOVEMENT_INCR);

        		pf.getGame().updateViewAttributes(character);
        	}
        }
    }

    public void error(Exception e)
    {
    	System.err.println("Problem in EnergyRegeneration timer: " + e.getMessage());
    }
}
