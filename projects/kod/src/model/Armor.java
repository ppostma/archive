/* $Id: Armor.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Armor.java
 * 
 * Defines the class Armor, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings. It has a limited working time which is defined by the variable duration.
 */

package src.model;

import src.utils.Constants;

public class Armor extends Attribute 
{
	/**
	 * Constructor.
	 */
    public Armor() 
    {
		health = Constants.ARMOR_HEALTH;
		energy = Constants.ARMOR_ENERGY;
		strength = Constants.ARMOR_STRENGTH;
		intelligence = Constants.ARMOR_INTELLIGENCE;
		speed = Constants.ARMOR_SPEED;
		armor = Constants.ARMOR_ARMOR;
		duration = Constants.ARMOR_DURATION;
    }

    /**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_ARMOR;
	}
}
