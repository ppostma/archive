/* $Id: Broomstick.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Broomstick.java
 * 
 * Defines the class Broomstick, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings. It has a limited working time which is defined by the variable duration.
 */

package src.model;

import src.utils.Constants;
import src.utils.Dice;

public class Broomstick extends Attribute 
{
	/**
	 * Constructor.
	 */
    public Broomstick() 
    {
		health = Constants.BROOMSTICK_HEALTH;
		energy = Constants.BROOMSTICK_ENERGY;
		strength = Constants.BROOMSTICK_STRENGTH;
		intelligence = Constants.BROOMSTICK_INTELLIGENCE;
		speed = Constants.BROOMSTICK_SPEED;
		armor = Constants.BROOMSTICK_ARMOR;
		duration = Constants.BROOMSTICK_DURATION;
    }

    /**
     * Use the broomstick. Fails sometimes.
     * 
     * @param ch
     */
    public void use(Character ch)
    {
        if (Dice.doThrowNumber(10) > 0)
            super.use(ch);
        else
            ch.setEnergy(ch.getEnergy() - 50);
    }

    /**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_BROOMSTICK;
	}
}
