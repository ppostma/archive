/* $Id: Lipstick.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Lipstick.java
 * 
 * Defines the class Lipstick, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings. It has a limited working time which is defined by the variable duration.
 */

package src.model;

import src.utils.Constants;

public class Lipstick extends Attribute 
{
	/**
	 * Constructor.
	 */
    public Lipstick() 
    {
		health = Constants.LIPSTICK_HEALTH;
		energy = Constants.LIPSTICK_ENERGY;
		strength = Constants.LIPSTICK_STRENGTH;
		intelligence = Constants.LIPSTICK_INTELLIGENCE;
		speed = Constants.LIPSTICK_SPEED;
		armor = Constants.LIPSTICK_ARMOR;
		duration = Constants.LIPSTICK_DURATION;
    }

    /**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_LIPSTICK;
	}
}
