/* $Id: Monster.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Monster.java
 * 
 * Monster Character type.
 */

package src.model;

public class Monster extends Character 
{
    private TeamCharacter teamCharacter;

	/**
	 * Constructor.
	 * 
	 * @param tch
	 */
    public Monster(TeamCharacter tch) 
    {
    	super(tch.getGender());
    	this.teamCharacter = tch;
    	setAttributes();

    	tch.killTimers();
    	CharacterSpot cs = tch.getLocation(); 
    	this.setLocation(cs);
    	cs.setCharacter(null);
    	cs.setCharacter(this);
    }

    /**
     * Set the default attributes.
     */
    public void setAttributes()
    {
    	this.health = 119;
    	this.armor  = 119;
    	this.intelligence = 119;
    	this.energy = 119;
    	this.speed = 119;
    	this.strength = 119;
    }

    /**
     * Get the player type.
     * 
     * @return Character 
     */
    public Character getPlayerType() 
    {
        return teamCharacter.getPlayerType();
    }

    /**
     * Get the player type as a char.
     * 
     * @return char
     */
    public char getPlayerTypeChar()
    {
    	return teamCharacter.getPlayerTypeChar();
    }

    /**
     * Get the player type as a String.
     * 
     * @return String
     */
    public String getPlayerTypeName()
    {
        return teamCharacter.getPlayerTypeName();
    }

    /**
     * Use an attribute. Returns true if use was successful
     * or false if the attribute was invalid.
     * 
     * @param attributeNr
     * @return Attribute
     */
    public Attribute useAttribute(int attributeNr) 
    {
    	if (attribute[attributeNr] instanceof Lipstick)
    		return null;

		return super.useAttribute(attributeNr);
    }

    public void setHealth(int h)
	{
		health = h;

		if (health <= 0) {
		    health = 0;
		    this.setDead();
		} else if (health > 119)
		    health = 119;
	}

    public void setEnergy(int e)
	{
		energy = e;

		if (energy < 0)
		    energy = 0;
		else if (energy > 119)
		    energy = 119;
	}

    public void setStrength(int s)
	{
		strength = s;

		if (strength < 0)
		    strength = 0;
		else if (strength > 119)
		    strength = 119;
	}

    public void setSpeed(int s)
	{
		speed = s;

		if (speed < 0)
		    speed = 0;
		else if (speed > 119)
		    speed = 119;
	}

    public void setIntelligence(int i)
	{
		intelligence = i;

		if (intelligence < 0)
		    intelligence = 0;
		else if (intelligence > 119)
		    intelligence = 119;
	}

    public void setArmor(int a)
	{
		armor = a;

		if (armor < 0)
		    armor = 0;
		else if (armor > 119)
		    armor = 119;
	}
}
