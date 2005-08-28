/* $Id: Sword.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Sword.java
 * 
 * Defines the class Sword, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings. It has a limited working time which is defined by the variable duration.
 */

package src.model;

import src.utils.Constants;
 
public class Sword extends Attribute 
{
	/**
	 * Constructor.
	 */
    public Sword() 
    {
		health = Constants.SWORD_HEALTH;
		energy = Constants.SWORD_ENERGY;
		strength = Constants.SWORD_STRENGTH;
		intelligence = Constants.SWORD_INTELLIGENCE;
		speed = Constants.SWORD_SPEED;
		armor = Constants.SWORD_ARMOR;
		duration = Constants.SWORD_DURATION;
    }

    /**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_SWORD;
	}
}
