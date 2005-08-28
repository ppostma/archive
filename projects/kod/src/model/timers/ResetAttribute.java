/* $Id: ResetAttribute.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * ResetAttribute.java
 * 
 * Reset the changes made by the attribute.
 */

package src.model.timers;

import src.model.Attribute;
import src.model.Character;
import src.model.Playfield;
import src.utils.TimerCallbackObject;

public class ResetAttribute implements TimerCallbackObject
{
    private Character character;
    private Attribute attribute;

    public ResetAttribute(Character character, Attribute attribute)
    {
        this.character = character;
        this.attribute = attribute;
    }

    public void ready()
    {
    	Playfield pf = Playfield.getInstance();

    	synchronized (pf) {
	        character.setEnergy(character.getEnergy() - attribute.getEnergy());
	        character.setHealth(character.getHealth() - attribute.getHealth());
	        character.setStrength(character.getStrength() - attribute.getStrength());
	        character.setIntelligence(character.getIntelligence() - attribute.getIntelligence());
	        character.setSpeed(character.getSpeed() - attribute.getSpeed());
	        character.setArmor(character.getArmor() - attribute.getArmor());
	
	        int attributeID = character.getIDbyAttribute(attribute);
	
	        attribute.reset(character);

	        pf.getGame().useAttributeFinished(character, attributeID);
	
	        character.setAttributeUsed(attributeID);
    	}
    }

    public void error(Exception e)
    {
    	System.err.println("Problem in ResetAttribute timer: " + e.getMessage());
    }
}
