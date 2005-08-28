/* $Id: Invisibility.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Invisibility.java
 * 
 * Defines the class Invisibility, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings. It has a limited working time which is defined by the variable duration.
 */

package src.model;

import src.utils.Constants;

public class Invisibility extends Attribute 
{
	/**
	 * Constructor.
	 */
    public Invisibility() 
    {
		health = Constants.INVISIBILITY_HEALTH;
		energy = Constants.INVISIBILITY_ENERGY;
		strength = Constants.INVISIBILITY_STRENGTH;
		intelligence = Constants.INVISIBILITY_INTELLIGENCE;
		speed = Constants.INVISIBILITY_SPEED;
		armor = Constants.INVISIBILITY_ARMOR;
		duration = Constants.INVISIBILITY_DURATION;
    }

    /**
     * Enables invisibility.
     * 
     * @param ch
     */
    public void use(Character ch)
    {
        ch.setInvisible(true);
        super.use(ch);
    }
    
    /**
     * Disables invisibility.
     * 
     * @param ch
     */
	public void reset(Character ch)
	{
	    ch.setInvisible(false);
	}

    /**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_INVISIBILITY;
	}
}
