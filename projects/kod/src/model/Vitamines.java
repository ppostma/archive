/* $Id: Vitamines.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Vitamines.java
 * 
 * Defines the class Vitamines, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings.
 */

package src.model;

import src.utils.Constants;

public class Vitamines extends Attribute 
{
	/**
	 * Constructor.
	 */
    public Vitamines() 
    {
		health = Constants.VITAMINES_HEALTH;
		energy = Constants.VITAMINES_ENERGY;
		strength = Constants.VITAMINES_STRENGTH;
		intelligence = Constants.VITAMINES_INTELLIGENCE;
		speed = Constants.VITAMINES_SPEED;
		armor = Constants.VITAMINES_ARMOR;
		duration = Constants.VITAMINES_DURATION;
    }

    /**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_VITAMINES;
	}
}
