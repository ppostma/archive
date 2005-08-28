/* $Id: Shield.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Shield.java
 * 
 * Defines the class Shield, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings. It has a limited working time which is defined by the variable duration.
 */

package src.model;

import src.utils.Constants;

public class Shield extends Attribute 
{
	/** 
	 * Constructor.
	 */
    public Shield() 
    {
		health = Constants.SHIELD_HEALTH;
		energy = Constants.SHIELD_ENERGY;
		strength = Constants.SHIELD_STRENGTH;
		intelligence = Constants.SHIELD_INTELLIGENCE;
		speed = Constants.SHIELD_SPEED;
		armor = Constants.SHIELD_ARMOR;
		duration = Constants.SHIELD_DURATION;
	}

    /**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_SHIELD;
	}
}
