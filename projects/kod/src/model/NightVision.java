/* $Id: NightVision.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * NightVision.java
 * 
 * Defines the class NightVision, this is an attribute in the game and can be used to
 * increase your health/energy/strength/intelligence/speed/armor, depending on the
 * settings. It has a limited working time which is defined by the variable duration.
 */

package src.model;

import src.utils.Constants;

public class NightVision extends Attribute 
{
	/**
	 * Constructor.
	 */
    public NightVision() 
    {
		health = Constants.NIGHTVISION_HEALTH;
		energy = Constants.NIGHTVISION_ENERGY;
		strength = Constants.NIGHTVISION_STRENGTH;
		intelligence = Constants.NIGHTVISION_INTELLIGENCE;
		speed = Constants.NIGHTVISION_SPEED;
		armor = Constants.NIGHTVISION_ARMOR;
		duration = Constants.NIGHTVISION_DURATION;
    }

    /**
     * Enables nightvision
     * 
     * @param ch
     */
    public void use(Character ch)
    {
        ch.setNightvision(true);
        super.use(ch);
    }
    
    /**
     * Disables nightvision
     * 
     * @param ch
     */
	public void reset(Character ch)
	{
	    ch.setNightvision(false);
	}

	/**
     * Get the attribute as char.
     * 
     * @return AttributeChar
     */
	public char getAttributeChar()
	{
		return Constants.ATTR_NIGHTVISION;
	}
}
