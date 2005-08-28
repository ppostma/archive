/* $Id: Attribute.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Attribute.java
 * 
 * Defines the class Attribute, this is the superclass of the following subclasses:
 * Armor, Broomstick, Invisibility, Lipstick, NightVision, Shield, Sword, Vitamines.
 */

package src.model;

import src.model.timers.ResetAttribute;
import src.utils.Timer;

public abstract class Attribute 
{
	protected int health;
	protected int energy;
	protected int strength;
	protected int intelligence;
	protected int speed;
	protected int armor;
	protected int duration;

	protected boolean active;
	protected Timer timer;

	/**
	 * Constructor.
	 */
    public Attribute()
    {
    	this.health = 0;
    	this.energy = 0;
    	this.strength = 0;
    	this.intelligence = 0;
    	this.speed = 0;
    	this.duration = 0;
    	this.armor = 0;

    	this.timer = null;
    	this.active = false;
    }

    /**
     * Destructor.
     */
    public void destroy()
    {
        this.active = false;
        if (timer != null)
            timer.stop();
    }

    /**
     * Use the attribute.
     * 
     * @param ch
     */
	public void use(Character ch)
	{
	    int oldEnergy = ch.getEnergy();
	    int oldHealth = ch.getHealth();
	    int oldStrength = ch.getStrength();
	    int oldIntelligence = ch.getIntelligence();
	    int oldSpeed = ch.getSpeed();
	    int oldArmor = ch.getArmor();

	    /* Update the character with the attribute. */
		ch.setEnergy(oldEnergy + this.energy);
		ch.setHealth(oldHealth + this.health);
		ch.setStrength(oldStrength + this.strength);
		ch.setIntelligence(oldIntelligence + this.intelligence);
		ch.setSpeed(oldSpeed + this.speed);
		ch.setArmor(oldArmor + this.armor);

		/* Attribute is used and only works for n seconds. */
		if (duration > 0) {
			/*
			 * Adjust attribute values with the amount that was added.
			 * This is needed to reset the attribute properly.
			 */
			this.energy = ch.getEnergy() - oldEnergy;
			this.health = ch.getHealth() - oldHealth;
			this.strength = ch.getStrength() - oldStrength;
			this.intelligence = ch.getIntelligence() - oldIntelligence;
			this.speed = ch.getSpeed() - oldSpeed;
			this.armor = ch.getArmor() - oldArmor;

			this.setActive(true);

			/* Create new timer to use on this attribute. */
			timer = new Timer(this.duration * 1000, 1, new ResetAttribute(ch, this));
			timer.start();
		}
	}
	
	/**
	 * Timer finished (called from ResetAttribute timer).
	 * 
	 * @param ch
	 */
	public void reset(Character ch)
	{
	}

	/**
	 * Get the energy of this attribute.
	 * 
	 * @return int
	 */
	public int getEnergy()
	{
	    return this.energy;
	}

	/**
	 * Get the health of this attribute.
	 * 
	 * @return int
	 */
	public int getHealth()
	{
	    return this.health;
	}

	/**
	 * Get the strength of this attribute.
	 * 
	 * @return int
	 */
	public int getStrength()
	{
	    return this.strength;
	}

	/**
	 * Get the intelligence of this attribute.
	 * 
	 * @return int
	 */
	public int getIntelligence()
	{
	    return this.intelligence;
	}

	/**
	 * Get the speed of this attribute.
	 * 
	 * @return int
	 */
	public int getSpeed()
	{
	    return this.speed;
	}

	/**
	 * Get the armor of this attribute.
	 * 
	 * @return int
	 */
	public int getArmor()
	{
	    return this.armor;
	}

	/**
	 * Returns if the attribute is active or not.
	 * 
	 * @return boolean
	 */
	public boolean isActive()
	{
		return this.active;
	}
	
	/**
	 * Set the attribute to active or inactive.
	 * 
	 * @param a
	 */
	public void setActive(boolean a)
	{
		this.active = a;
	}

	/*
	 * To be implemented by subclasses.
	 */
	public abstract char getAttributeChar();
}
